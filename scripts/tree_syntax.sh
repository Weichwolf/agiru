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
# THE SAME FLAGS THE BUILD WILL USE, so that what this measures is what the build will do. -O2
# costs 2 % over -O0 here (measured 2026-09-02: 502 ms against 493 ms without a precompiled door),
# because the front end dominates entirely -- and the test suite has to RUN fast, which is what the
# optimiser is for. Measuring something cheaper than the build would answer a different question.
OPT=-O2
includes="-Iinclude"
for d in "$APPS"/*/; do
  [ -d "$d" ] && includes="$includes -I${d%/}"
done

# THE DOOR IS PARSED ONCE AND NOT 6 398 TIMES. Measured 2026-09-02: a generated table costs 502 ms
# with the door re-parsed and 77 ms with it precompiled, and an EMPTY translation unit costs 53 ms
# of that -- so the door was five sixths of every file. The PCH is rebuilt on every run because it
# takes about two seconds and a stale one is worse than no measurement at all.
#
# THE FLAGS MUST MATCH. A PCH built with -O2 and used without it is rejected, and clang reports that
# in 13 ms -- which looks exactly like a very fast compile. It was measured as one before the
# negative control caught it.
PCH=$OUT/agiru.pch
# shellcheck disable=SC2086
clang++ -std=c++23 $OPT $includes -x c++-header -o "$PCH" include/agiru.h 2>"$OUT/pch.log" || {
  printf 'tree: the door does not precompile -- see %s
' "$OUT/pch.log" >&2
  exit 1
}
includes="$includes -include-pch $PCH"

# AND THE TREE MUST NOT MOVE WHILE IT IS BEING MEASURED. Editing a door header mid-run invalidates
# the precompiled header, and clang then reports "file has been modified since the precompiled
# header was built" -- which lands in the failure count and looks like 1 701 broken objects. That
# happened. The lock above stops two RUNS from colliding; this stops a run from colliding with an
# edit, which is the same mistake wearing different clothes.
DOORPRINT=$(cat include/agiru.h include/*/*.h | cksum)

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
: > "$OUT/roots"
xargs -a "$OUT/files" -P "$(nproc)" -I{} sh -c '
  err=$(mktemp)
  if clang++ -std=c++23 $4 -fsyntax-only -Wall -Wextra -Wpedantic $2 "$1" 2>"$err"; then
    printf "%s\n" "$1" >> "$3/passed" || exit 1
  else
    printf "%s\n" "$1" >> "$3/failed" || exit 1
    cat "$err" >> "$3/errors"
    # THE FIRST DIAGNOSTIC IS THE ROOT AND THE REST IS THE CASCADE. clang reports the deepest
    # include first, so a header that fails because something it includes fails names the OTHER
    # file here. Counting how often a type is MENTIONED ranked symptoms as causes twice in one
    # session: `LibraryVariableStorage` looked like the third-largest gap and was a missing source
    # root, and `LibraryLowerPermissions` looked like a gap and is `DotNet` two includes down.
    first=$(grep -m1 -E "error: " "$err")
    printf "%s\t%s\t%s\n" "$1" \
      "$(printf "%s" "$first" | sed -E "s/:[0-9]+:[0-9]+: (fatal )?error: .*//")" \
      "$(printf "%s" "$first" | sed -E "s/^.*: (fatal )?error: //")" >> "$3/roots"
  fi
  rm -f "$err"
' _ {} "$includes" "$OUT" "$OPT"

# BOTH TALLIES ARE READ, NEITHER IS DERIVED, AND THEY MUST ADD UP.
for f in passed failed; do
  [ -f "$OUT/$f" ] || {
    printf 'tree: %s/%s is gone -- the run recorded nothing. ABORT, not a pass.\n' "$OUT" "$f" >&2
    exit 1
  }
done
if [ "$(cat include/agiru.h include/*/*.h | cksum)" != "$DOORPRINT" ]; then
  printf 'tree: the door changed while the run was measuring it. ABORT, not a number.\n' >&2
  exit 1
fi
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
printf '\ntree: the ROOT of each failure -- its FIRST diagnostic, by how many headers it stops\n'
awk -F'\t' '{print $3}' "$OUT/roots" | sed -E "s/'[^']*'/'X'/g" | sort | uniq -c | sort -rn | head -10
printf '\ntree: what those roots NAME, by how many headers stop on it\n'
awk -F'\t' '{print $3}' "$OUT/roots" | grep -oE "'[^']+'" | sort | uniq -c | sort -rn | head -14
printf '\ntree: the single FILE each failure starts in, by how many others it stops\n'
awk -F'\t' '$1 != $2 {print $2}' "$OUT/roots" | sort | uniq -c | sort -rn | head -10
printf '\ntree: everything else, by kind\n'
grep -E 'error:' "$OUT/errors" | grep -v 'unknown type name' |
  sed -E "s/^[^ ]+: //; s/'[^']*'/'X'/g" | sort | uniq -c | sort -rn | head -10
exit 0
