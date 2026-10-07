#!/bin/sh
# The source manifest is authoritative: a missing executable must not shrink the gate.
set -eu
cd "$(dirname "$0")/.."
B=${B:-build}
red=0
n=0
for source in test/gate/*.cpp; do
  [ -f "$source" ] || { echo 'test: no gate sources found' >&2; exit 2; }
  name=$(basename "$source" .cpp)
  case="$B/gate_$name"
  n=$((n + 1))
  if [ ! -x "$case" ]; then
    printf 'test: missing executable %s; run make\n' "$case" >&2
    red=$((red + 1))
  elif [ "$name" = PlatformSourceGate ] && [ "${AGIRU_SYSTEM_SYMBOLS+x}" = x ]; then
    if ! python3 scripts/fetch_symbols.py --verify "$AGIRU_SYSTEM_SYMBOLS"; then
      red=$((red + 1))
    elif ! "$case" "$AGIRU_SYSTEM_SYMBOLS"; then
      red=$((red + 1))
    fi
  elif ! "$case"; then
    red=$((red + 1))
  fi
done
for script in test/transpiler/builtins-reproduce.sh test/tooling/one-definition.sh; do
  n=$((n + 1))
  if sh "$script"; then :; else
    status=$?
    printf 'test: %s failed (exit %s)\n' "$script" "$status" >&2
    red=$((red + 1))
  fi
done
for script in test/tooling/function-size.sh test/runtime/required-isolation.sh test/tooling/header-dependencies.sh test/tooling/slice-check.sh test/transpiler/interface-defaults.sh test/reporting/report-layouts.sh test/reporting/layout-assets.sh test/runtime/number-sequences.sh test/transpiler/table-keys.sh test/runtime/reflection-metadata.sh test/runtime/catalogue.sh test/runtime/variant-text.sh test/transpiler/native-enums.sh test/runtime/test-contexts.sh test/runtime/text-positions.sh test/runtime/xml-reader.sh test/runtime/codeunit-record.sh test/runtime/page-navigation.sh test/runtime/boolean-expressions.sh test/runtime/for-loops.sh test/transpiler/control-extensions.sh test/transpiler/native-table-ids.sh test/transpiler/native-codeunits.sh test/runtime/base64.sh test/runtime/encoding.sh test/runtime/hashing.sh test/runtime/conversion.sh test/runtime/record-order.sh test/runtime/streams.sh test/transpiler/system-profile.sh test/runtime/xmlport-import.sh; do
  n=$((n + 1))
  if B="$B" bash "$script"; then :; else
    status=$?
    printf 'test: %s failed (exit %s)\n' "$script" "$status" >&2
    red=$((red + 1))
  fi
done
n=$((n + 1))
if ! B="$B" bash test/ui/page-profile.sh; then red=$((red + 1)); fi
n=$((n + 1))
if ! B="$B" bash test/runtime/session-identity.sh; then red=$((red + 1)); fi
n=$((n + 1))
if ! B="$B" bash test/runtime/ui-host.sh; then red=$((red + 1)); fi
n=$((n + 1))
if ! B="$B" bash test/runtime/table-permissions.sh; then red=$((red + 1)); fi
n=$((n + 1))
if ! B="$B" bash test/runtime/permission-sets.sh; then red=$((red + 1)); fi
n=$((n + 1))
if ! B="$B" bash test/runtime/native-permissions.sh; then red=$((red + 1)); fi
n=$((n + 1))
if ! B="$B" python3 test/tooling/toolchain.py; then red=$((red + 1)); fi
printf '\ntest: %s case(s), %s red\n' "$n" "$red"
[ "$red" -eq 0 ]
