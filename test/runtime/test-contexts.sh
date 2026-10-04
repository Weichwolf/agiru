#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-test-contexts.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/test-contexts/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/runtime/test-contexts/TryScopes.{Table,Page}.al "$proof/source/"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" -Isrc/rt -c test/runtime/test-contexts/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" "$dsn"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/test-contexts/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -Isrc/rt -c test/runtime/test-contexts/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/test-contexts.json"
for control in consumed discarded nested case sort-value sort-discard; do
  source="$proof/generated/fixture/fixture/table/TryRow.cpp"
  if [ "$control" = nested ]; then
    source="$proof/generated/fixture/fixture/codeunit/TestContextConsumer.cpp"
  fi
  cp "$source" "$proof/$control-original.cpp"
  awk -v control="$control" '
    control == "consumed" && $0 == "  return ::agiru::Tried([&] { return TryFail(); });" {
      $0 = "  return TryFail();"; changed++
    }
    control == "discarded" && $0 == "  TryFail();" {
      $0 = "  static_cast<void>(::agiru::Tried([&] { return TryFail(); }));"; changed++
    }
    control == "nested" && $0 == "  Store(::agiru::Tried([&] { return (*this).TryFail(); }), Result);" {
      $0 = "  Store((*this).TryFail(), Result);"; changed++
    }
    control == "sort-value" && $0 == "  return Ok_SetCurrentKey(Payload);" {
      $0 = "  return SetCurrentKey(Payload);"; changed++
    }
    control == "sort-discard" && $0 == "  SetCurrentKey(Payload);" {
      $0 = "  static_cast<void>(Ok_SetCurrentKey(Payload));"; changed++
    }
    control == "case" && /\[\[maybe_unused\]\] const auto CaseValue_Block_1 =/ {
      sub(/ = /, " = [\\&] { return "); sub(/;$/, "; };"); changed++
    }
    control == "case" && /CaseValue_Block_1 ==/ {
      sub(/CaseValue_Block_1 ==/, "CaseValue_Block_1() =="); changed++
    }
    { print }
    END { if (changed != (control == "case" ? 3 : 1)) exit 2 }
  ' "$proof/$control-original.cpp" > "$source"
  "$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/$control"
  if "$proof/$control" "$dsn" > "$proof/$control-execution.log" 2>&1; then
    printf 'test-contexts: %s TryFunction defect escaped the execution control\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    consumed) rg -q 'an exception left Generated Test Context' "$proof/$control-execution.log"
              rg -q 'table failure' "$proof/$control-execution.log" ;;
    discarded) rg -q 'discarded table-local TryFunctions still raise' "$proof/$control-execution.log" ;;
    nested) rg -q 'an exception left Generated Test Context' "$proof/$control-execution.log"
            rg -q 'codeunit failure' "$proof/$control-execution.log" ;;
    case) rg -q 'case catches retain their preceding mutation' "$proof/$control-execution.log" ;;
    sort-value) rg -q 'an exception left Generated Test Context' "$proof/$control-execution.log"
                rg -q 'not sortable' "$proof/$control-execution.log" ;;
    sort-discard) rg -q 'discarded implicit Blob sorting raises' "$proof/$control-execution.log" ;;
  esac
  cp "$proof/$control-original.cpp" "$source"
  rm -- "$proof/$control" "$proof/$control-original.cpp"
done
for control in key-prefix key-disabled key-unindexed; do
  awk -v control="$control" '
    control == "key-prefix" && /fields = declared.fields;/ {
      sub(/fields = declared.fields;/, "fields = declared.fields.first(fields.size());"); changed++
    }
    control == "key-disabled" && /if \(!declared.enabled \|\| declared.fields.size\(\) < fields.size\(\)\)/ {
      sub(/!declared.enabled \|\| /, ""); changed++
    }
    control == "key-unindexed" && /std::vector<SortField> selected;/ {
      print "  if (std::ranges::none_of(table.keys, [fields](const auto &key) { return key.fields.data() == fields.data(); })) { return false; }";
      changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/RecordState.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "$proof/$control.cpp" "${links[@]}" -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$B/gate_CurrentKeyGate" > "$proof/$control-execution.log" 2>&1; then
    printf 'test-contexts: %s escaped the common key-selection gate\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    key-prefix|key-disabled) rg -q 'the first active prefix contributes its entire key' "$proof/$control-execution.log" ;;
    key-unindexed) rg -q 'sortable unindexed fields succeed' "$proof/$control-execution.log" ;;
  esac
  rm -- "$proof/$control.so" "$proof/$control.cpp"
done
cp "$proof/source/Fixture.Codeunit.al" "$proof/original.al"
sed 's/exit(Context.CodeunitId);/exit(0);/' "$proof/original.al" > "$proof/source/Fixture.Codeunit.al"
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/mutant.log" 2>&1
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/mutant"
if "$proof/mutant" "$dsn" > "$proof/mutant-execution.log" 2>&1; then
  printf 'test-contexts: altered provider identity escaped the execution control\n' >&2
  exit 1
fi
rg -q 'AL property syntax returns the provider ID' "$proof/mutant-execution.log"
printf 'test-contexts: generated getters/copy/skip, TryFunction scopes/arguments, case selectors and SetCurrentKey contexts execute; six call-context, three key-selection and source-expression controls fail; %s\n' "$proof"
