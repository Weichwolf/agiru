#!/bin/sh
# How much of the GENERATED tree the compiler accepts. Not the build -- the build is still behind
# AGIRU_BUILD_APPS -- but the same compiler with the same flags, which is what names the next gap.
#
# WHY THIS IS A SCRIPT AND NOT A SHELL LINE. It was a shell line, and it produced three numbers for
# two states of the tree: 1453, then 1310 after a commit that only ADDED types. Two runs had
# overlapped on the same scratch files and each was reading the other's output. A measurement that
# can silently interleave with itself is the blind gate CLAUDE.md lists, pointed at the measuring
# apparatus rather than at the code. So: one lock, one output directory, and a refusal rather than a
# wrong number.
set -eu
cd "$(dirname "$0")/.."

APPS=${1:-apps/base}
OUT=build/tree-syntax
LOCK=$OUT/lock

mkdir -p "$OUT"
if ! mkdir "$LOCK" 2>/dev/null; then
  printf 'tree: a run is already in progress (%s). Refusing rather than mixing with it.\n' "$LOCK" >&2
  exit 2
fi
trap 'rmdir "$LOCK" 2>/dev/null || true' EXIT INT TERM

[ -d "$APPS" ] || {
  printf 'tree: %s does not exist -- run `make transpile` first\n' "$APPS" >&2
  exit 2
}

find "$APPS" -name '*.h' | sort > "$OUT/files"
total=$(wc -l < "$OUT/files" | tr -d ' ')
# A COUNT OF 0 OVER N UNITS IS AN ABORT, NOT A PASS.
[ "$total" -gt 0 ] || {
  printf 'tree: no generated headers under %s -- ABORT\n' "$APPS" >&2
  exit 1
}

: > "$OUT/failed"
: > "$OUT/errors"
xargs -a "$OUT/files" -P "$(nproc)" -I{} sh -c '
  err=$(mktemp)
  if clang++ -std=c++23 -fsyntax-only -Wall -Wextra -Wpedantic -Iinclude -I"$2" "$1" 2>"$err"; then
    :
  else
    printf "%s\n" "$1" >> "$3/failed"
    cat "$err" >> "$3/errors"
  fi
  rm -f "$err"
' _ {} "$APPS" "$OUT"

failed=$(wc -l < "$OUT/failed" | tr -d ' ')
printf 'tree: %s of %s generated headers compile\n' "$((total - failed))" "$total"
[ "$failed" -eq 0 ] && exit 0

printf '\ntree: what is missing, by how many headers name it\n'
grep -oE "unknown type name '[^']+'" "$OUT/errors" | sed "s/unknown type name //" | sort | uniq -c |
  sort -rn | head -20
printf '\ntree: everything else, by kind\n'
grep -E 'error:' "$OUT/errors" | grep -v 'unknown type name' |
  sed -E "s/^[^ ]+: //; s/'[^']*'/'X'/g" | sort | uniq -c | sort -rn | head -10
exit 0
