#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-codeunit-record.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/codeunit-record/*.al "$proof/source/"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-DAGIRU_TEST_DSN=\"$dsn\""
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" -c test/runtime/codeunit-record/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" "$dsn" > "$proof/execution.log" 2>&1
cat "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/codeunit-record/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/runtime/codeunit-record/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/codeunit-record.json"
mkdir -p "$proof/mutant"
cp -a include "$proof/mutant/"
for control in no-borrow no-restore assignment-alias; do
  awk -v control="$control" '
    control == "no-borrow" && /borrower.borrowed_ = owner;/ {
      sub(/borrower.borrowed_ = owner;/, "static_cast<void>(owner);"); changed++
    }
    control == "no-restore" && /~BorrowScope\(\)/ {
      sub(/borrower_.borrowed_ = previous_;/, "static_cast<void>(previous_);"); changed++
    }
    control == "assignment-alias" && /Globals &operator=\(const Globals &other\)/ {
      print
      print "    borrowed_ = const_cast<Globals *>(&other);"
      changed++
      next
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/runtime/Codeunit.h > "$proof/mutant/include/runtime/Codeunit.h"
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/runtime/codeunit-record/Runner.cpp \
    "${sources[@]}" "${links[@]}" -o "$proof/$control"
  arguments=("$dsn")
  if [ "$control" = assignment-alias ]; then arguments+=(--copy-only); fi
  if "$proof/$control" "${arguments[@]}" > "$proof/$control-execution.log" 2>&1; then
    printf 'codeunit-record: %s escaped execution controls\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    no-borrow) rg -q 'table globals saved before rollback survive in the caller' "$proof/$control-execution.log" ;;
    no-restore) rg -q 'a failed call restores the callee.s original globals' "$proof/$control-execution.log" ;;
    assignment-alias) rg -q 'ordinary assignment retains the destination.s table globals' "$proof/$control-execution.log" ;;
  esac
  rm -- "$proof/$control" "$proof/mutant/include/runtime/Codeunit.h"
done
rm -- "$proof/runner" "$proof/runner.o"
rm -r -- "$proof/mutant"
"$B/gate_TransactionContractGate" > "$proof/transaction-baseline.log" 2>&1
for control in no-implicit-commit no-rollback try-write-bypass worker-scope-leak; do
  source=src/rt/Transaction.cpp
  if [ "$control" = worker-scope-leak ]; then source=src/rt/Scopes.cpp; fi
  awk -v control="$control" '
    control == "no-implicit-commit" && /session.Transaction\(\).Commit\(session.Database\(\)\);/ {
      $0 = "  static_cast<void>(session);"; changed++
    }
    control == "no-rollback" && /connection.Run\("ROLLBACK TO SAVEPOINT " \+/ {
      changed++; next
    }
    control == "try-write-bypass" && /if \(session.Options\(\).disableWriteInsideTryFunctions/ {
      $0 = "  if (false) {"; changed++
    }
    control == "worker-scope-leak" && /return detail::SessionState::Current\(\).(commits|errors|collectedErrors);/ {
      if (/\.commits;/) print "  static thread_local std::vector<::agiru::CommitBehavior> leaked;"
      if (/\.errors;/) print "  static thread_local std::vector<::agiru::ErrorBehavior> leaked;"
      if (/\.collectedErrors;/) print "  static thread_local std::vector<std::string> leaked;"
      $0 = "  return leaked;"; changed++
    }
    { print }
    END { if (changed != (control == "worker-scope-leak" ? 3 : 1)) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -Isrc/rt -fPIC -shared "$proof/$control.cpp" \
    "${links[@]}" -o "$proof/$control.so"
  status=0
  LD_PRELOAD="$proof/$control.so" "$B/gate_TransactionContractGate" \
    > "$proof/$control-execution.log" 2>&1 || status=$?
  if [ "$status" -ne 1 ]; then
    printf 'codeunit-record: %s expected failure 1, received %s\n' "$control" "$status" >&2
    exit 1
  fi
  case "$control" in
    no-implicit-commit) claim='evaluated success is independently durable' ;;
    no-rollback) claim='failed evaluated writes are removed from the writer' ;;
    try-write-bypass) claim='configured try policy controls actual database writes' ;;
    worker-scope-leak) claim='another session on the same worker inherits no CommitBehavior' ;;
  esac
  rg -q "$claim" "$proof/$control-execution.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/transaction-controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
printf 'codeunit-record: generated Run/try policies, transaction contracts, three globals and four transaction controls proved; %s\n' "$proof"
