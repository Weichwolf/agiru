#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
container=${AGIRU_DEV_CONTAINER:-agiru-dev}
seed=${AGIRU_ERP_SEED:-agiru_client_seed_20261007b}
source_container=${AGIRU_ERP_SOURCE_CONTAINER:-agiru-pg}
source_database=${AGIRU_ERP_SOURCE_DATABASE:-cronus}
native_build=${AGIRU_ERP_BUILD:-/workspace/build/podman}
[[ "$seed" =~ ^agiru_client_seed_[A-Za-z0-9_]+$ ]]
[[ "$container" =~ ^[A-Za-z0-9][A-Za-z0-9_.-]*$ ]]
[[ "$source_container" =~ ^[A-Za-z0-9][A-Za-z0-9_.-]*$ ]]
[[ "$source_database" =~ ^[A-Za-z0-9_]+$ ]]
[[ "$native_build" =~ ^/workspace/build(/[A-Za-z0-9_-]+)+$ ]]
native_exec=(podman exec --user agiru --workdir /workspace "$container")
native_sql() { "${native_exec[@]}" psql -XAt -v ON_ERROR_STOP=1 -d "$1" -c "$2"; }
source_sql() { podman exec "$source_container" psql -U agiru -XAt -v ON_ERROR_STOP=1 -d "$source_database" -c "$1"; }
original_company() {
  source_sql $'COPY (SELECT
(\'x\'||encode("timestamp",\'hex\'))::bit(64)::bigint,
"Name",("Evaluation Company"=1),"Display Name","Id","Business Profile Id",
"$systemId","$systemCreatedAt","$systemCreatedBy","$systemModifiedAt","$systemModifiedBy"
FROM system."Company") TO STDOUT'
}
"${native_exec[@]}" findmnt -T /tmp
"${native_exec[@]}" df -h /tmp
proof=$(mktemp -d /tmp/agiru-erp-fixture.XXXXXX)
native=$("${native_exec[@]}" mktemp -d /tmp/agiru-erp-fixture.XXXXXX)
[[ "$native" =~ ^/tmp/agiru-erp-fixture\.[A-Za-z0-9]+$ ]]
database="agiru_erp_gate_${BASHPID}_${native##*.}"
nonce=${native##*.}
created=0
cleanup() {
  local status=$?
  trap - EXIT
  if [[ "$created" = 1 ]]; then
    local owner
    owner=$(native_sql postgres "SELECT shobj_description(oid,'pg_database') FROM pg_database WHERE datname='$database'") || status=1
    if [[ "$owner" = "agiru ERP fixture $nonce" ]]; then
      "${native_exec[@]}" dropdb --force "$database" || status=1
    else
      printf 'erp-fixture: refusing cleanup without matching database ownership\n' >&2
      status=1
    fi
  fi
  "${native_exec[@]}" rm -f -- "$native/prepare" "$native/prepare.o" "$native/auth.json" "$native/auth.json.denied" || status=1
  "${native_exec[@]}" rmdir -- "$native" || status=1
  printf 'erp-fixture: exit %s; receipt %s\n' "$status" "$proof"
  exit "$status"
}
trap cleanup EXIT
git rev-parse HEAD > "$proof/head.txt"
sha256sum test/ui/erp-fixture.sh test/ui/erp/Prepare.cpp > "$proof/inputs.sha256"
native_sql "$seed" 'SELECT status,details FROM agiru_seed_provenance' > "$proof/seed-before.txt"
identity=$(native_sql "$seed" "SELECT status='complete' AND details->>'typed_readback'='true'
AND details->>'source_container'='$source_container'
AND details->>'source_database'='$source_database' FROM agiru_seed_provenance")
[[ "$identity" = t ]] || { printf 'erp-fixture: seed lacks matching complete typed-readback provenance\n' >&2; exit 1; }
[[ $(source_sql 'SELECT count(*) FROM system."Company"') = 1 ]]
[[ $(source_sql 'SELECT octet_length("timestamp")=8 AND get_byte("timestamp",0)<128 AND "Evaluation Company" IN (0,1) FROM system."Company"') = t ]]
company=$(source_sql 'SELECT "Name" FROM system."Company"')
[[ -n "$company" && "$company" != *$'\n'* && "$company" != *$'\r'* ]]
[[ $(native_sql postgres "SELECT count(*) FROM pg_database WHERE datname='$database'") = 0 ]]
"${native_exec[@]}" createdb --template "$seed" "$database"
created=1
native_sql postgres "COMMENT ON DATABASE \"$database\" IS 'agiru ERP fixture $nonce'"
dsn="postgresql://agiru:agiru@127.0.0.1:5432/$database"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate -Iapps/platform -Iapps/shared)
"${native_exec[@]}" sha256sum "$native_build"/libagiru_{app_platform,rt,net,db}.so > "$proof/libraries.sha256"
"${native_exec[@]}" clang++-19 "${flags[@]}" -c test/ui/erp/Prepare.cpp -o "$native/prepare.o"
"${native_exec[@]}" clang++-19 -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "$native/prepare.o" "-L$native_build" "-Wl,-rpath,$native_build" \
  -lagiru_app_platform -lagiru_rt -lagiru_net -lagiru_db -o "$native/prepare"
jq -n --arg directory /workspace --arg file /workspace/test/ui/erp/Prepare.cpp \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  clang++-19 "${flags[@]}" -c /workspace/test/ui/erp/Prepare.cpp -o "$native/prepare.o" > "$proof/commands.json"
"${native_exec[@]}" mkdir -p "$native_build/fixture-commands"
podman cp "$proof/commands.json" "$container:$native_build/fixture-commands/erp-fixture.json"
if "${native_exec[@]}" "$native/prepare" "postgresql://agiru:agiru@127.0.0.1:5432/$seed" --schema > "$proof/non-owned.log" 2>&1; then
  printf 'erp-fixture: preparation accepted a non-fixture database\n' >&2
  exit 1
fi
rg -q 'ERP preparation refuses a non-fixture database' "$proof/non-owned.log"
"${native_exec[@]}" "$native/prepare" "$dsn" --schema | tee "$proof/schema.log"
[[ $(native_sql "$database" 'SELECT count(*) FROM "Company"') = 0 ]]
original_company > "$proof/company-source.tsv"
podman exec --interactive --user agiru "$container" psql -X -v ON_ERROR_STOP=1 -d "$database" \
  -c 'COPY "Company" ("timestamp","Name","Evaluation Company","Display Name","Id","Business Profile Id",
"SystemId","SystemCreatedAt","SystemCreatedBy","SystemModifiedAt","SystemModifiedBy") FROM STDIN' \
  < "$proof/company-source.tsv" > "$proof/company-copy.log"
native_sql "$database" 'COPY (SELECT "timestamp","Name","Evaluation Company","Display Name","Id","Business Profile Id",
"SystemId","SystemCreatedAt","SystemCreatedBy","SystemModifiedAt","SystemModifiedBy" FROM "Company") TO STDOUT' \
  > "$proof/company-target.tsv"
cmp "$proof/company-source.tsv" "$proof/company-target.tsv"
"${native_exec[@]}" "$native/prepare" "$dsn" "$company" "$native/auth.json" | tee "$proof/prepare.log"
[[ $("${native_exec[@]}" stat --format=%a "$native/auth.json") = 600 ]]
[[ $("${native_exec[@]}" stat --format=%a "$native/auth.json.denied") = 600 ]]
native_sql "$database" $'SELECT
(SELECT count(*) FROM "User"), (SELECT count(*) FROM "Access Control"),
(SELECT count(*) FROM "Tenant Permission Set" WHERE "Assignable"),
(SELECT count(*) FROM "Tenant Permission" WHERE "Object ID"=0 AND "Security Filter"=\'\'),
(SELECT count(*) FROM agiru_client.credentials),
(SELECT count(*) FROM "Customer"), (SELECT count(*) FROM "G/L Entry"),
(SELECT count(*) FROM "Item"), (SELECT count(*) FROM "Sales Header")' > "$proof/sql.txt"
[[ $(<"$proof/sql.txt") = '2|1|1|9|2|68|2820|149|44' ]]
native_sql "$seed" 'SELECT status,details FROM agiru_seed_provenance' > "$proof/seed-after.txt"
cmp "$proof/seed-before.txt" "$proof/seed-after.txt"
original_company > "$proof/company-source-after.tsv"
cmp "$proof/company-source.tsv" "$proof/company-source-after.tsv"
"${native_exec[@]}" sha256sum "$native_build"/libagiru_{app_platform,rt,net,db}.so > "$proof/libraries-after.sha256"
cmp "$proof/libraries.sha256" "$proof/libraries-after.sha256"
sha256sum --check --status "$proof/inputs.sha256"
printf 'erp-fixture: original Company copied exactly; explicit tenant authority and denial; no ERP workflow acceptance\n'
