#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-permission-sets.XXXXXX)
sha256sum include/meta/PermissionSetDef.h include/runtime/PermissionSets.h \
  src/rt/PermissionSets.cpp test/gate/PermissionSetsGate.cpp \
  test/runtime/permission-sets.sh > "$proof/inputs.sha256"
controls=(precedence exclude filter identity cycle entries edges override)
cleanup() {
  for control in "${controls[@]}"; do
    for suffix in cpp bin; do
      if [[ -f "$proof/$control.$suffix" ]]; then unlink "$proof/$control.$suffix"; fi
    done
  done
  if [[ -f "$proof/sanitizers.bin" ]]; then unlink "$proof/sanitizers.bin"; fi
}
trap cleanup EXIT
"$B/gate_PermissionSetsGate" > "$proof/execution.log" 2>&1
cat "$proof/execution.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in "${controls[@]}"; do
  awk -v control="$control" '
    control == "precedence" && /return PermissionLevel::Direct;/ {
      sub(/PermissionLevel::Direct/, "PermissionLevel::Indirect"); changed++
    }
    control == "exclude" && /result.level = Subtract\(result.level, excluded.level\);/ {
      sub(/Subtract\(result.level, excluded.level\)/,
          "Combine(result.level, Subtract(excluded.level, PermissionLevel::None))"); changed++
    }
    control == "filter" && /if \(result.filtered\)/ {
      sub(/result.filtered/, "(static_cast<void>(result.filtered), false)"); changed++
    }
    control == "identity" && /definition == nullptr \|\| definition->identity != identity/ {
      sub(/ \|\| definition->identity != identity/, ""); changed++
    }
    control == "cycle" && /std::ranges::find\(active_, identity\) != active_.end\(\)/ {
      sub(/std::ranges::find\(active_, identity\) != active_.end\(\)/,
          "(static_cast<void>(std::ranges::find(active_, identity)), false)"); changed++
    }
    control == "entries" && /entries_ == limits_.entries/ {
      sub(/entries_ == limits_.entries/,
          "(static_cast<void>(entries_ == limits_.entries), false)"); changed++
    }
    control == "edges" && /edges_ == limits_.edges/ {
      sub(/edges_ == limits_.edges/,
          "(static_cast<void>(edges_ == limits_.edges), false)"); changed++
    }
    control == "override" && /result.level = PermissionLevel::Indirect;/ {
      sub(/PermissionLevel::Indirect/, "PermissionLevel::None"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/PermissionSets.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" test/gate/PermissionSetsGate.cpp \
    "${links[@]}" -o "$proof/$control.bin" > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    precedence) claim='every documented include/exclude operation matches' ;;
    exclude) claim='child exclusion removes its read grant' ;;
    filter) claim='security filters never become unrestricted table grants' ;;
    identity) claim='catalogue cannot substitute a different app/role/scope' ;;
    cycle) claim='include cycles refuse explicitly' ;;
    entries) claim='entry bound refuses excess permission arrays' ;;
    edges) claim='edge bound counts repeated cached references' ;;
    override) claim='tenant override reduces a direct modify permission to indirect' ;;
  esac
  rg -q "^FAIL .*${claim}" "$proof/$control.log"
done
"$CXX" "${flags[@]}" -fsanitize=address,undefined -fno-omit-frame-pointer \
  src/rt/PermissionSets.cpp test/gate/PermissionSetsGate.cpp "${links[@]}" \
  -o "$proof/sanitizers.bin" > "$proof/sanitizers.compile.log" 2>&1
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  "$proof/sanitizers.bin" > "$proof/sanitizers.log" 2>&1
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'permission-sets: BC composition, ASan/UBSan and eight compiled defects; native SQL assignment/execution-context providers remain unqualified; %s\n' "$proof"
