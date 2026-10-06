#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-table-permissions.XXXXXX)
sha256sum include/runtime/{TablePermissions,Table,RecordRef,Session}.h \
  src/rt/{TablePermissions,Table,RecordRef,Navigate,Query,Session}.cpp \
  test/gate/TablePermissionsGate.cpp test/runtime/table-permissions.sh > "$proof/inputs.sha256"
cleanup() {
  for control in bypass partial-write; do
    for suffix in cpp bin; do
      if [[ -f "$proof/$control.$suffix" ]]; then unlink "$proof/$control.$suffix"; fi
    done
  done
}
trap cleanup EXIT
"$B/gate_TablePermissionsGate" > "$proof/execution.log" 2>&1
cat "$proof/execution.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate -Isrc/rt
  "-DAGIRU_TEST_DSN=\"$dsn\"")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
for control in bypass partial-write; do
  awk -v control="$control" '
    control == "bypass" && /if \(HasTablePermission\(table, operation\)\)/ {
      sub(/HasTablePermission\(table, operation\)/, "true"); changed++
    }
    control == "partial-write" && /return HasTablePermission\(table, TableOperation::Insert\) &&/ {
      sub(/HasTablePermission\(table, TableOperation::Insert\)/,
          "(static_cast<void>(HasTablePermission(table, TableOperation::Insert)), true)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/TablePermissions.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" test/gate/TablePermissionsGate.cpp \
    "${links[@]}" -o "$proof/$control.bin" > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    bypass) rg -q '^FAIL .*denied insert refuses before SQL' "$proof/$control.log" ;;
    partial-write) rg -q '^FAIL .*WritePermission requires every individual write kind' "$proof/$control.log" ;;
  esac
done
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'table-permissions: typed/reflected SQL-backed grants and two compiled defects; not native BC permission-set/filter qualification; %s\n' "$proof"
