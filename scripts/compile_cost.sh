#!/bin/sh
# WHAT ONE GENERATED TRANSLATION UNIT COSTS, and what a precompiled door would save.
#
# A generated file includes the door headers it names, and the door plus the standard library is some
# 36 500 of the ~36 760 preprocessed lines in one -- 99 % of the file is the same 99 % as every
# other file's. That is the same text parsed once per translation unit, and there are 5 681 of them.
# This measures the three costs a build actually pays, so the projection is arithmetic rather than a
# guess.
set -eu
cd "$(dirname "$0")/.."

FILE=${1:-}
if [ -z "$FILE" ]; then
  FILE=$(head -1 build/tree-syntax/passed 2>/dev/null | sed 's/\.h$/.cpp/')
fi
[ -f "$FILE" ] || { printf 'compile: no generated source to measure (%s)\n' "$FILE" >&2; exit 2; }

APP=$(printf '%s' "$FILE" | cut -d/ -f1-2)
FLAGS="-std=c++23 -Iinclude -I$APP -Wall -Wextra -Wpedantic"
ROUNDS=${ROUNDS:-5}
OUT=build/compile-cost
mkdir -p "$OUT"

printf 'compile: %s, %s rounds each\n\n' "$FILE" "$ROUNDS"

time_it() {
  label=$1
  shift
  start=$(date +%s%N)
  i=0
  while [ "$i" -lt "$ROUNDS" ]; do
    "$@" >/dev/null 2>&1 || { printf '  %-28s FAILED\n' "$label"; return 1; }
    i=$((i + 1))
  done
  end=$(date +%s%N)
  printf '  %-28s %s ms\n' "$label" "$(( (end - start) / 1000000 / ROUNDS ))"
}

# shellcheck disable=SC2086
time_it "syntax only" clang++ $FLAGS -fsyntax-only "$FILE"
# shellcheck disable=SC2086
time_it "compile, -O2" clang++ $FLAGS -O2 -c -o "$OUT/one.o" "$FILE"
# shellcheck disable=SC2086
time_it "compile, -O0" clang++ $FLAGS -O0 -c -o "$OUT/one.o" "$FILE"

printf '\ncompile: the door, precompiled\n'
# shellcheck disable=SC2086
clang++ $FLAGS -x c++-header -O2 -o "$OUT/agiru.pch" cmake/Precompiled.h 2>/dev/null || {
  printf '  the door does not precompile on its own\n'
  exit 0
}
printf '  %-28s %s\n' "size" "$(du -h "$OUT/agiru.pch" | cut -f1)"
# shellcheck disable=SC2086
time_it "compile, -O2, with the pch" \
  clang++ $FLAGS -O2 -include-pch "$OUT/agiru.pch" -c -o "$OUT/one.o" "$FILE"
