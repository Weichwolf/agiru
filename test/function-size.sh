#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
tidy=${AGIRU_CLANG_TIDY:-clang-tidy-19}
output=${B:-"$root/build"}
mkdir -p -- "$output"
fixtures=$(mktemp -d "$output/function-size.XXXXXX")

fixture() {
  printf 'void Boundary() {\n'
  for ((line = 0; line < $1; ++line)); do
    printf '  (void)0;\n'
  done
  printf '}\n'
}

fixture 119 > "$fixtures/accepted.cpp"
fixture 120 > "$fixtures/refused.cpp"
options=(--quiet --config-file="$root/.clang-tidy"
  '--checks=-*,readability-function-size')
"$tidy" "${options[@]}" "$fixtures/accepted.cpp" -- -std=c++23 \
  > "$fixtures/accepted.log" 2>&1
if "$tidy" "${options[@]}" "$fixtures/refused.cpp" -- -std=c++23 \
    > "$fixtures/refused.log" 2>&1; then
  printf 'function-size: 121-line function was accepted\n' >&2
  exit 1
fi
if ! rg -q '121 lines including whitespace and comments \(threshold 120\)' "$fixtures/refused.log"; then
  printf 'function-size: refusal was not the required line limit\n' >&2
  exit 1
fi
printf 'function-size: 120 lines accepted; 121 lines refused\n'
