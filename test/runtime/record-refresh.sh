#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-record-refresh.XXXXXX)
gate="$B/gate_RecordRefreshGate"
generator="$B/gate_GenReceiverGate"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -shared
  -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.so" \) -delete' EXIT
git rev-parse HEAD > "$proof/head"
sha256sum include/runtime/RecordRefresh.h src/rt/{RecordRefresh,RecordChanges,Navigate}.cpp \
  src/rt/RecordChanges.h src/gen/RuntimeSurface.cpp \
  test/gate/{RecordRefresh,GenReceiver}Gate.cpp test/runtime/record-refresh.sh \
  "$gate" "$generator" "$B/libagiru_rt.so" "$B/libagiru_gen.so" > "$proof/inputs.sha256"
"$CXX" --version > "$proof/compiler.txt"
"$gate" > "$proof/current.log" 2>&1
"$generator" > "$proof/generator.log" 2>&1
for control in ignored-refresh all-tables lost-cache-lock implicit-commit frozen-snapshot; do
  source=src/rt/RecordChanges.cpp
  case "$control" in
    ignored-refresh) claim='an all-table refresh invalidates non-locked readers' ;;
    all-tables) claim='a selected refresh leaves another table current' ;;
    lost-cache-lock) claim='cache lock metadata survives an all-table refresh' ;;
    implicit-commit)
      source=src/rt/RecordRefresh.cpp
      claim='refresh cannot implicitly commit a pending write'
      ;;
    frozen-snapshot)
      source=src/rt/RecordRefresh.cpp
      claim='a frozen SQL snapshot refuses instead of claiming freshness'
      ;;
  esac
  awk -v control="$control" '
    /if \(!table.has_value\(\) \|\| id == \*table\) \{ \+\+revision.refreshed; \}/ {
      if (control == "ignored-refresh") {
        print "    static_cast<void>(id); static_cast<void>(revision); static_cast<void>(table);";
        changed++; next
      }
      if (control == "all-tables") {
        print "    static_cast<void>(id); static_cast<void>(table); ++revision.refreshed;";
        changed++; next
      }
    }
    control == "lost-cache-lock" && /\(locked_ \|\| refreshed_ == revision_->second.refreshed\)/ {
      sub(/locked_ \|\| /, ""); changed++
    }
    control == "implicit-commit" && /const auto &connection = Session::Current\(\).Database\(\);/ {
      print; print "  Session::Current().Transaction().Commit(connection);"; changed++; next
    }
    control == "frozen-snapshot" && /if \(!value.has_value\(\)/ {
      print "  static_cast<void>(value); if (false) {"; changed++; next
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
awk '
  /\{"SelectLatestVersion", "runtime\/RecordRefresh.h"\}/ {
    sub(/runtime\/RecordRefresh.h/, "runtime/Database.h"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/gen/RuntimeSurface.cpp > "$proof/wrong-provider.cpp"
"$CXX" "$proof/wrong-provider.cpp" "${flags[@]}" -Isrc/gen -Isrc/al \
  "-DAGIRU_SOURCE_DIR=\"$(pwd)\"" -lagiru_gen -lagiru_al -o "$proof/wrong-provider.so" \
  > "$proof/wrong-provider.compile.log" 2>&1
status=0
LD_PRELOAD="$proof/wrong-provider.so" "$generator" > "$proof/wrong-provider.log" 2>&1 || status=$?
[[ "$status" = 1 ]]
rg -q 'FAIL .*both refresh overloads name the narrow runtime provider' "$proof/wrong-provider.log"
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
cat "$proof/current.log"
cat "$proof/generator.log"
printf 'record-refresh: six compiled freshness/table/cache-lock/transaction/snapshot/provider defects reject; receipts %s\n' "$proof"
