#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-ui-host.XXXXXX)
gate="$B/gate_UiHostGate"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -shared
  -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.so" \) -delete' EXIT
git rev-parse HEAD > "$proof/head"
sha256sum include/runtime/UiHost.h include/type/Dialog.h include/BuiltinsWritten.h \
  src/rt/{UiHost,TypeMethods,SingleInstance}.cpp src/rt/SessionState.h \
  src/rt/written/BuiltinsWritten.cpp test/gate/UiHostGate.cpp test/gate/OwnedDatabase.h \
  test/runtime/ui-host.sh "$gate" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
"$CXX" --version > "$proof/compiler.txt"
"$gate" > "$proof/current.log" 2>&1

for control in always-true always-false default-consent test-fallback implicit-commit lost-bindings; do
  source=src/rt/written/BuiltinsWritten.cpp
  claim=''
  case "$control" in
    always-true) claim='a background session has no UI capability' ;;
    always-false) claim='a real installed endpoint enables GUI interaction' ;;
    default-consent) claim='an explicit negative answer overrides a positive presentation default' ;;
    test-fallback) claim='an undeclared test handler never falls back to a native endpoint' ;;
    implicit-commit) claim='a question cannot implicitly commit a pending write' ;;
    lost-bindings)
      source=src/rt/UiHost.cpp
      claim='typed progress bindings retain every variable'
      ;;
  esac
  awk -v control="$control" '
    /return HandlerTable::Installed\(\) \|\| CurrentUiHost\(\) != nullptr;/ {
      if (control == "always-true" || control == "always-false") {
        print "  return " (control == "always-true" ? "true" : "false") ";";
        changed++; next
      }
    }
    control == "default-consent" && /answer = host->Confirm\(text, answer\);/ {
      print "      static_cast<void>(host->Confirm(text, answer));"; changed++; next
    }
    control == "test-fallback" && /if \(HandlerTable::Installed\(\)\) \{ return false; \}/ {
      changed++; next
    }
    control == "implicit-commit" && /answer = host->Confirm\(text, answer\);/ {
      print "      ::agiru::Commit();"; changed++
    }
    control == "lost-bindings" && /host->OpenProgress\(owner, text, values\);/ {
      print "    static_cast<void>(values);";
      print "    host->OpenProgress(owner, text, {});"; changed++; next
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "$proof/$control.cpp" "${flags[@]}" -o "$proof/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  rg -q "FAIL .*${claim}" "$proof/$control.log"
done
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
cat "$proof/current.log"
printf 'ui-host: six compiled capability/answer/test-boundary/transaction/binding defects reject; receipts %s\n' "$proof"
