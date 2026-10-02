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
  elif ! "$case"; then
    red=$((red + 1))
  fi
done
for script in test/door-reproduces.sh test/one-definition.sh; do
  n=$((n + 1))
  if ! sh "$script"; then red=$((red + 1)); fi
done
for script in test/function-size.sh test/platform-source.sh test/required-isolation.sh test/header-dependencies.sh test/slice-check.sh; do
  n=$((n + 1))
  if ! B="$B" bash "$script"; then red=$((red + 1)); fi
done
n=$((n + 1))
if ! B="$B" python3 test/toolchain.py; then red=$((red + 1)); fi
printf '\ntest: %s case(s), %s red\n' "$n" "$red"
[ "$red" -eq 0 ]
