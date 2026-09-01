#!/bin/sh
# How much of the GENERATED tree the compiler accepts. Not the build -- the build is still behind
# AGIRU_BUILD_APPS -- but the same compiler with the same flags, which is what names the next gap.
#
# THIS SCRIPT HAS LIED TWICE AND BOTH GUARDS BELOW ARE SCARS.
#
# First it was a shell line, and it gave three numbers for two states of the tree: 1453 headers
# compiling, then 1310 after a commit that only ADDED types. Two runs had overlapped on the same
# scratch files and each was reading the other's output. Hence the lock.
#
# Then its output directory was deleted underneath it by an unrelated `rm -rf build`, every child
# failed to record anything, and it reported "2121 of 2121 compile" -- a FALSE GREEN, which is the
# worse direction. It had derived the pass count by subtracting a failure count it could not read.
# Hence: every result is COUNTED, never derived; a missing tally is an abort; and the two counts
# must add up to the population or the run is thrown away.
set -eu
cd "$(dirname "$0")/.."

APPS=${1:-apps}
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

# Each app is compiled with its own directory on the include path AND the ones it depends on, which
# is how a table reaches an enum declared in another app. Reading that from apps.json would mean a
# second JSON reader in shell; every app directory is added instead, which is a SUPERSET of what
# each app may see. The linker enforces the real direction, and the transpiler already enforces it
# by the order it resolves names in.
includes="-Iinclude"
for d in "$APPS"/*/; do
  [ -d "$d" ] && includes="$includes -I${d%/}"
done

find "$APPS" -name '*.h' | sort > "$OUT/files"
total=$(wc -l < "$OUT/files" | tr -d ' ')
# A COUNT OF 0 OVER N UNITS IS AN ABORT, NOT A PASS.
[ "$total" -gt 0 ] || {
  printf 'tree: no generated headers under %s -- ABORT\n' "$APPS" >&2
  exit 1
}

: > "$OUT/passed"
: > "$OUT/failed"
: > "$OUT/errors"
xargs -a "$OUT/files" -P "$(nproc)" -I{} sh -c '
  err=$(mktemp)
  if clang++ -std=c++23 -fsyntax-only -Wall -Wextra -Wpedantic $2 "$1" 2>"$err"; then
    printf "%s\n" "$1" >> "$3/passed" || exit 1
  else
    printf "%s\n" "$1" >> "$3/failed" || exit 1
    cat "$err" >> "$3/errors"
  fi
  rm -f "$err"
' _ {} "$includes" "$OUT"

# BOTH TALLIES ARE READ, NEITHER IS DERIVED, AND THEY MUST ADD UP.
for f in passed failed; do
  [ -f "$OUT/$f" ] || {
    printf 'tree: %s/%s is gone -- the run recorded nothing. ABORT, not a pass.\n' "$OUT" "$f" >&2
    exit 1
  }
done
passed=$(wc -l < "$OUT/passed" | tr -d ' ')
failed=$(wc -l < "$OUT/failed" | tr -d ' ')
if [ "$((passed + failed))" -ne "$total" ]; then
  printf 'tree: %s compiled + %s failed is not %s attempted -- the run lost files. ABORT.\n' \
    "$passed" "$failed" "$total" >&2
  exit 1
fi

printf 'tree: %s of %s generated headers compile\n' "$passed" "$total"
[ "$failed" -eq 0 ] && exit 0

printf '\ntree: what is missing, by how many headers name it\n'
grep -oE "unknown type name '[^']+'" "$OUT/errors" | sed "s/unknown type name //" | sort | uniq -c |
  sort -rn | head -20
printf '\ntree: everything else, by kind\n'
grep -E 'error:' "$OUT/errors" | grep -v 'unknown type name' |
  sed -E "s/^[^ ]+: //; s/'[^']*'/'X'/g" | sort | uniq -c | sort -rn | head -10
exit 0
