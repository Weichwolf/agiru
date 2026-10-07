#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
image=${AGIRU_DEV_IMAGE:-localhost/agiru-dev:latest}
prefix=agiru-dev-check-$$
volume=$prefix-data
bad_volume=$prefix-bad-data
proof=$(mktemp -d /tmp/agiru-dev-check.XXXXXX)
sha256sum deploy/dev/Containerfile deploy/dev/entrypoint.sh deploy/dev/Caddyfile scripts/dev_container.sh \
  test/tooling/podman-development.sh > "$proof/inputs.sha256"
podman image inspect "$image" --format '{{.Id}}' > "$proof/image.txt"
cleanup() {
  for suffix in main failure proxy-failure bad unowned tls lock-budget; do
    target=$prefix-$suffix
    if podman container exists "$target"; then
      [[ "$(podman inspect --format '{{index .Config.Labels "io.agiru.development.test"}}' "$target")" = "$prefix" ]] || continue
      podman rm --force "$target" >/dev/null
    fi
  done
  for target in "$volume" "$bad_volume"; do
    if podman volume exists "$target"; then
      [[ "$(podman volume inspect --format '{{index .Labels "io.agiru.development.test"}}' "$target")" = "$prefix" ]] || continue
      podman volume rm "$target" >/dev/null
    fi
  done
  if [[ -f "$proof/lock-budget-entrypoint.sh" ]]; then unlink "$proof/lock-budget-entrypoint.sh"; fi
}
trap cleanup EXIT
ready() {
  local container=${1:-$prefix-main}
  for ((attempt=0; attempt<60; attempt++)); do
    if podman exec --user postgres "$container" psql -XAt -v ON_ERROR_STOP=1 \
      -d agiru_gate -c 'SELECT 1' > "$proof/ready.log" 2>&1 \
      && [[ "$(podman exec "$container" curl --silent --output /dev/null --write-out '%{http_code}' http://127.0.0.1:8080/ 2>/dev/null)" = 502 ]]; then return; fi
    [[ "$(podman inspect --format '{{.State.Running}}' "$container")" = true ]] || {
      podman logs "$container" >&2; return 1;
    }
    sleep 0.5
  done
  printf 'Development SQL readiness timed out\n' >&2
  return 1
}
schema_locks() {
  podman exec --interactive --user postgres "$1" psql -XAt -v ON_ERROR_STOP=1 \
    -d agiru_gate <<'SQL'
BEGIN;
DO $locks$
BEGIN
  PERFORM pg_advisory_xact_lock(x'41474456'::integer, slot)
  FROM generate_series(1, 16384) AS slot;
END
$locks$;
SELECT count(*) FROM pg_locks WHERE locktype = 'advisory'
AND pid = pg_backend_pid() AND classid = x'41474456'::integer;
ROLLBACK;
SQL
}
podman volume create --label "io.agiru.development.test=$prefix" "$volume" >/dev/null
podman volume create --label "io.agiru.development.test=$prefix" "$bad_volume" >/dev/null
podman run --detach --name "$prefix-main" --label io.agiru.development=true \
  --label "io.agiru.repository=$PWD" --label "io.agiru.development.test=$prefix" --volume "$volume:/var/lib/agiru" \
  "$image" /bin/sleep 300 > "$proof/main-id.txt"
ready
podman exec "$prefix-main" dpkg-query -W -f='${Version}\n' caddy > "$proof/caddy-package.txt"
[[ "$(podman exec "$prefix-main" sh -c 'command -v caddy')" = /usr/bin/caddy ]]
podman exec "$prefix-main" dpkg-query -S /usr/bin/caddy > "$proof/caddy-owner.txt"
podman exec "$prefix-main" test -s /usr/share/doc/caddy/copyright
if podman exec "$prefix-main" sh -c 'command -v nginx' > "$proof/obsolete-proxy.log" 2>&1; then exit 1; fi
[[ "$(podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 -d agiru_gate -c 'SHOW server_encoding')" = UTF8 ]]
[[ "$(podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 -d agiru_gate -c 'SHOW max_locks_per_transaction')" = 1024 ]] || {
  printf 'Development PostgreSQL requires the full-schema lock budget of 1024\n' >&2; exit 1;
}
schema_locks "$prefix-main" > "$proof/schema-locks.log"
rg -q '^16384$' "$proof/schema-locks.log"
awk '
  /max_locks_per_transaction=1024/ { sub(/=1024/, "=64"); changed++ }
  { print }
  END { if (changed != 1) exit 2 }
' deploy/dev/entrypoint.sh > "$proof/lock-budget-entrypoint.sh"
chmod 700 "$proof/lock-budget-entrypoint.sh"
podman run --detach --name "$prefix-lock-budget" --label "io.agiru.development.test=$prefix" \
  --tmpfs /var/lib/agiru:rw \
  --volume "$proof/lock-budget-entrypoint.sh:/usr/local/bin/agiru-dev-entrypoint:ro" \
  "$image" /bin/sleep 300 > "$proof/lock-budget-id.txt"
ready "$prefix-lock-budget"
status=0
schema_locks "$prefix-lock-budget" > "$proof/lock-budget-control.log" 2>&1 || status=$?
[[ "$status" = 3 ]]
rg -q 'out of shared memory' "$proof/lock-budget-control.log"
podman stop --time 30 "$prefix-lock-budget" > "$proof/lock-budget-stop.log"
[[ "$(podman exec "$prefix-main" curl --silent --output /dev/null --write-out '%{http_code}' http://127.0.0.1:8080/)" = 502 ]]
podman exec "$prefix-main" ss -H -l -t -n > "$proof/listeners.log"
rg -q '127\.0\.0\.1:5432' "$proof/listeners.log"
rg -q '(0\.0\.0\.0|\*):8080' "$proof/listeners.log"
podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 -d agiru_gate \
  -c "CREATE TABLE development_receipt (amount numeric); INSERT INTO development_receipt VALUES (1234.56)" \
  > "$proof/sql-write.log"
AGIRU_DEV_CONTAINER=$prefix-main bash scripts/dev_container.sh stop > "$proof/stop.log"
AGIRU_DEV_CONTAINER=$prefix-main bash scripts/dev_container.sh start > "$proof/restart.log"
ready
[[ "$(podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 \
  -d agiru_gate -c 'SELECT amount::text FROM development_receipt')" = 1234.56 ]]
podman exec "$prefix-main" bash -c 'kill -TERM "$(head -n 1 "$PGDATA/postmaster.pid")"'
[[ "$(timeout 30 podman wait "$prefix-main")" = 1 ]]
podman logs "$prefix-main" > "$proof/database-exit.log" 2>&1
podman run --detach --name "$prefix-failure" --label "io.agiru.development.test=$prefix" \
  --volume "$volume:/var/lib/agiru" "$image" /bin/sh -c 'exit 42' > "$proof/failure-id.txt"
[[ "$(timeout 45 podman wait "$prefix-failure")" = 42 ]]
podman logs "$prefix-failure" > "$proof/application-exit.log" 2>&1
podman run --detach --name "$prefix-proxy-failure" --label "io.agiru.development.test=$prefix" \
  --volume "$volume:/var/lib/agiru" "$image" /bin/sh -c \
  'sleep 1; kill -TERM "$(head -n 1 /run/agiru/caddy.pid)"; sleep 300' > "$proof/proxy-failure-id.txt"
[[ "$(timeout 45 podman wait "$prefix-proxy-failure")" = 1 ]]
podman logs "$prefix-proxy-failure" > "$proof/proxy-exit.log" 2>&1
podman create --name "$prefix-unowned" --label "io.agiru.development.test=$prefix" \
  --entrypoint /bin/true "$image" > "$proof/unowned-id.txt"
status=0
AGIRU_DEV_CONTAINER=$prefix-unowned bash scripts/dev_container.sh start \
  > "$proof/unowned.log" 2>&1 || status=$?
[[ "$status" = 1 ]]
[[ "$(podman inspect --format '{{.State.Status}}' "$prefix-unowned")" = created ]]
podman run --detach --name "$prefix-tls" --label "io.agiru.development.test=$prefix" \
  --env AGIRU_HTTP_ADDRESS=localhost --volume "$volume:/var/lib/agiru" "$image" \
  > "$proof/tls-id.txt"
root_certificate=/var/lib/agiru/caddy/data/caddy/pki/authorities/local/root.crt
tls_ready=0
for ((attempt=0; attempt<60; attempt++)); do
  if [[ "$(podman exec "$prefix-tls" curl --silent --cacert "$root_certificate" \
    --output /dev/null --write-out '%{http_code}' https://localhost:8443/ 2>/dev/null)" = 502 ]]; then
    tls_ready=1; break
  fi
  [[ "$(podman inspect --format '{{.State.Running}}' "$prefix-tls")" = true ]] || {
    podman logs "$prefix-tls" >&2; exit 1;
  }
  sleep 0.5
done
[[ "$tls_ready" = 1 ]]
podman exec "$prefix-tls" curl --silent --dump-header - --output /dev/null \
  http://localhost:8080/ > "$proof/tls-redirect.log"
rg -q '^HTTP/1\.1 308 ' "$proof/tls-redirect.log"
rg -qi '^location: https://localhost/' "$proof/tls-redirect.log"
status=0
podman exec "$prefix-tls" curl --silent --output /dev/null https://localhost:8443/ \
  > "$proof/tls-untrusted.log" 2>&1 || status=$?
[[ "$status" = 60 ]]
certificate_hash=$(podman exec "$prefix-tls" sha256sum "$root_certificate")
podman stop --time 30 "$prefix-tls" > "$proof/tls-stop.log"
podman start "$prefix-tls" > "$proof/tls-restart.log"
tls_ready=0
for ((attempt=0; attempt<60; attempt++)); do
  if [[ "$(podman exec "$prefix-tls" curl --silent --cacert "$root_certificate" \
    --output /dev/null --write-out '%{http_code}' https://localhost:8443/ 2>/dev/null)" = 502 ]]; then
    tls_ready=1; break
  fi
  sleep 0.5
done
[[ "$tls_ready" = 1 ]]
[[ "$(podman exec "$prefix-tls" sha256sum "$root_certificate")" = "$certificate_hash" ]]
podman exec "$prefix-tls" ss -H -l -t -n > "$proof/tls-listeners.log"
if rg -q ':2019\s' "$proof/tls-listeners.log"; then exit 1; fi
podman stop --time 30 "$prefix-tls" > "$proof/tls-final-stop.log"
status=0
AGIRU_DEV_CONTAINER=$prefix-unowned bash scripts/dev_container.sh stop \
  > "$proof/unowned-stop.log" 2>&1 || status=$?
[[ "$status" = 1 ]]
[[ "$(podman inspect --format '{{.State.Status}}' "$prefix-unowned")" = created ]]
podman run --rm --entrypoint /bin/bash --volume "$bad_volume:/var/lib/agiru" "$image" \
  -c 'mkdir -p /var/lib/agiru/postgres; printf "18\n" > /var/lib/agiru/postgres/PG_VERSION; printf "preserve\n" > /var/lib/agiru/postgres/sentinel'
podman run --detach --name "$prefix-bad" --label "io.agiru.development.test=$prefix" \
  --volume "$bad_volume:/var/lib/agiru" "$image" > "$proof/bad-id.txt"
[[ "$(timeout 30 podman wait "$prefix-bad")" = 1 ]]
podman logs "$prefix-bad" > "$proof/version-refusal.log" 2>&1
rg -q 'PostgreSQL storage version mismatch' "$proof/version-refusal.log"
[[ "$(podman run --rm --entrypoint /bin/bash --volume "$bad_volume:/var/lib/agiru" "$image" \
  -c 'read -r marker < /var/lib/agiru/postgres/sentinel; printf "%s" "$marker"')" = preserve ]]
sha256sum --check "$proof/inputs.sha256" > "$proof/input-integrity.log"
printf 'podman-development: full-schema lock budget and low-budget control, SQL/certificate persistence, verified local TLS/redirect, disabled admin API, Caddy/app/PostgreSQL supervisor exits, ownership and incompatible storage verified; public ACME not qualified; %s\n' "$proof"
