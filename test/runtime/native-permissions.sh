#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-native-permissions.XXXXXX)
sha256sum include/runtime/NativePermissions.h src/rt/NativePermissionSnapshot.h \
  src/rt/{NativePermissionSnapshot,NativePermissions}.cpp test/gate/NativePermissionsGate.cpp \
  test/runtime/native-permissions.sh > "$proof/inputs.sha256"
controls=(user company scope filter missing bytes indirect override)
cleanup() {
  for control in "${controls[@]}" sanitizers; do
    for suffix in cpp bin; do
      if [[ -f "$proof/$control.$suffix" ]]; then unlink "$proof/$control.$suffix"; fi
    done
  done
}
trap cleanup EXIT
"$B/gate_NativePermissionsGate" > "$proof/execution.log" 2>&1
cat "$proof/execution.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate -Isrc/rt
  "-DAGIRU_TEST_DSN=\"$dsn\"")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
for control in "${controls[@]}"; do
  source=src/rt/NativePermissionSnapshot.cpp
  companion=src/rt/NativePermissions.cpp
  if [[ "$control" = indirect ]]; then
    source=src/rt/NativePermissions.cpp
    companion=src/rt/NativePermissionSnapshot.cpp
  fi
  awk -v control="$control" '
    control == "user" && /"User Security ID" = \$1::uuid AND/ {
      sub(/"User Security ID" = \$1::uuid AND/, "$1::uuid IS NOT NULL AND"); changed++
    }
    control == "company" && /AND \("Company Name"/ {
      sub(/AND .*/, "AND $2::text IS NOT NULL"); changed++
    }
    control == "scope" && /"Scope" AS scope/ {
      sub(/"Scope" AS scope/, "1 AS scope"); changed++
    }
    control == "filter" && /\.securityFiltered = Boolean\(Cell\(row, kFilter\)\)/ {
      sub(/Boolean\(Cell\(row, kFilter\)\)/,
          "(static_cast<void>(Boolean(Cell(row, kFilter))), false)"); changed++
    }
    control == "missing" && /if \(!Boolean\(Cell\(row, 4\)\)\)/ {
      sub(/!Boolean\(Cell\(row, 4\)\)/,
          "(static_cast<void>(Boolean(Cell(row, 4))), false)"); changed++
    }
    control == "bytes" && /if \(value->size\(\) > available\)/ {
      sub(/value->size\(\) > available/,
          "(static_cast<void>(value->size() > available), false)"); changed++
    }
    control == "indirect" && /if \(level == PermissionLevel::Indirect\)/ {
      sub(/level == PermissionLevel::Indirect/,
          "(static_cast<void>(level == PermissionLevel::Indirect), false)"); changed++
    }
    control == "override" && /if \(text == "1"\).*return true;/ {
      sub(/return true;/, "return false;"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" "$companion" test/gate/NativePermissionsGate.cpp \
    "${links[@]}" -o "$proof/$control.bin" > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    user) claim="another user does not inherit the first user.s assignments" ;;
    company) claim='company-specific assignments never cross companies' ;;
    scope) claim='same role in another scope cannot substitute tenant authority' ;;
    filter) claim='native security filters never become unrestricted grants' ;;
    missing) claim='missing SQL set declarations refuse instead of losing assignments' ;;
    bytes) claim='native row/text bounds refuse excess policy' ;;
    indirect) claim='indirect rights never authorize unsupported execution contexts' ;;
    override) claim='native inline exclusion reduces direct modify to indirect' ;;
  esac
  rg -q "^FAIL .*${claim}" "$proof/$control.log"
done
"$CXX" "${flags[@]}" -fsanitize=address,undefined -fno-omit-frame-pointer \
  src/rt/NativePermissionSnapshot.cpp src/rt/NativePermissions.cpp test/gate/NativePermissionsGate.cpp \
  "${links[@]}" -o "$proof/sanitizers.bin" > "$proof/sanitizers.compile.log" 2>&1
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  "$proof/sanitizers.bin" > "$proof/sanitizers.log" 2>&1
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'native-permissions: original SQL assignments, table/page effects, ASan/UBSan and eight compiled defects; installed system metadata, filters and indirect execution contexts remain unqualified; %s\n' "$proof"
