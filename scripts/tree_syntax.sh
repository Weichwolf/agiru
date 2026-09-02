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
# THE LOCK LIVES OUTSIDE WHAT THE RUN WIPES. It sat in `$OUT`, which a caller clears before
# starting -- so clearing it DELETED the lock and a second run began while the first was still
# appending. That happened: an `xargs` from a killed run stayed alive for 24 minutes, reading the
# old file list and writing into the new run's tallies, and `passed + failed` came to 6 797 of
# 6 631. The tally guard caught the count; nothing caught the cause.
LOCK=$(dirname "$OUT")/tree.lock

mkdir -p "$OUT"
if ! mkdir "$LOCK" 2>/dev/null; then
  printf 'tree: a run is already in progress (%s). Refusing rather than mixing with it.\n' "$LOCK" >&2
  exit 2
fi
# AND THE WORKERS DIE WITH THE RUN. Killing the script left `xargs` and its children alive for 24
# minutes, writing into the next run's tallies. The trap takes the CHILDREN and not the process
# group: `kill 0` reaches the caller's group too, which killed the shell that started the run.
trap 'rmdir "$LOCK" 2>/dev/null || true; pkill -P $$ 2>/dev/null || true' EXIT INT TERM

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
DOORPRINT=$(cat include/agiru.h include/*/*.h | sha1sum | cut -d' ' -f1)

find "$APPS" -name '*.h' | sort > "$OUT/files"

# THE SAME HEADER OVER THE SAME CLOSURE GIVES THE SAME VERDICT, so it is asked once. A run used to
# compile all 6 630 headers every time, and between two runs a few hundred of them differ -- 25
# minutes to learn what 24 of them were already going to say.
#
# THE KEY IS THE CLOSURE AND NOT THE FILE. A header's result depends on everything it includes, so
# keying on its own bytes would reuse a verdict for a file that did not change while a header it
# includes did -- the measurement lying in the direction that is hardest to notice, because it lies
# in favour of the last good answer. The door's print is part of every key, so a door edit empties
# the cache by construction. Computing all 6 631 keys costs 3 s (measured).
CACHE=${AGIRU_TREE_CACHE:-build/tree-cache}
mkdir -p "$CACHE"
python3 scripts/tree_keys.py "$DOORPRINT" "$APPS" < "$OUT/files" > "$OUT/keys"
[ "$(wc -l < "$OUT/keys")" = "$(wc -l < "$OUT/files")" ] || {
  printf 'tree: one key per header is required and %s of %s came back. ABORT.\n' \
    "$(wc -l < "$OUT/keys")" "$(wc -l < "$OUT/files")" >&2
  exit 1
}
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
# `-I{}` BRINGS ITS OWN LINE SPLITTING and `-d` overrides it: with both, some lines ran twice and
# `passed + failed` came to 6 932 of 6 631. The tally guard caught it, which is what it is for.
xargs -a "$OUT/keys" -P "$(nproc)" -I{} sh -c '
  file=${1%%	*}
  hit="$5/${1##*	}"
  if [ -f "$hit" ]; then
    if [ "$(head -1 "$hit")" = pass ]; then
      printf "%s\n" "$file" >> "$3/passed" || exit 1
    else
      printf "%s\n" "$file" >> "$3/failed" || exit 1
      tail -n +2 "$hit" >> "$3/roots"
    fi
    exit 0
  fi
  err=$(mktemp)
  unit=$(mktemp --suffix=.cpp)
  # A HEADER IS INCLUDED, NOT COMPILED: `#pragma once` has no effect in the main file, so a header
  # reached again through one of its own includes is read twice and every class in it is a
  # redefinition that a real build never sees.
  # AN ABSOLUTE PATH, because the unit is written to the system temp directory and a repo-relative
  # include resolves against the directory of the unit first and the include path second -- neither
  # of which is the repo root. Every one of 6 631 headers came back "file not found".
  printf "#include \"%s\"\n" "$(cd "$(dirname "$file")" && pwd)/$(basename "$file")" > "$unit"
  if clang++ -std=c++23 $4 -fsyntax-only -ferror-limit=1 -Wall -Wextra -Wpedantic $2 "$unit" \
      2>"$err"; then
    printf "%s\n" "$file" >> "$3/passed" || exit 1
    printf "pass\n" > "$hit"
  else
    printf "%s\n" "$file" >> "$3/failed" || exit 1
    cat "$err" >> "$3/errors"
    # THE FIRST DIAGNOSTIC IS THE ROOT AND THE REST IS THE CASCADE, WHICH IS WHY THE RUN STOPS AT
    # IT. clang reports the deepest include first, so a header that fails because something it
    # includes fails names the OTHER file here. Counting how often a type is MENTIONED ranked
    # symptoms as causes twice in one session.
    first=$(grep -m1 -E "error: " "$err")
    root=$(printf "%s	%s	%s" "$file" \
      "$(printf "%s" "$first" | sed -E "s/:[0-9]+:[0-9]+: (fatal )?error: .*//")" \
      "$(printf "%s" "$first" | sed -E "s/^.*: (fatal )?error: //")")
    printf "%s\n" "$root" >> "$3/roots"
    { printf "fail\n"; printf "%s\n" "$root"; } > "$hit"
  fi
  rm -f "$err" "$unit"
' _ {} "$includes" "$OUT" "$OPT" "$CACHE"

# BOTH TALLIES ARE READ, NEITHER IS DERIVED, AND THEY MUST ADD UP.
for f in passed failed; do
  [ -f "$OUT/$f" ] || {
    printf 'tree: %s/%s is gone -- the run recorded nothing. ABORT, not a pass.\n' "$OUT" "$f" >&2
    exit 1
  }
done
# THE SAME PRINT AT BOTH ENDS. This compared a `cksum` against a `sha1sum` after the print changed
# at the top and not here, so it fired on every run -- a guard that always fires says as little as
# one that never does, and this one turned a four-minute measurement into an abort.
if [ "$(cat include/agiru.h include/*/*.h | sha1sum | cut -d' ' -f1)" != "$DOORPRINT" ]; then
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

printf '\ntree: the ROOT of each failure -- its FIRST diagnostic, by how many headers it stops\n'
awk -F'\t' '{print $3}' "$OUT/roots" | sed -E "s/'[^']*'/'X'/g" | sort | uniq -c | sort -rn | head -10
printf '\ntree: what those roots NAME, by how many headers stop on it\n'
awk -F'\t' '{print $3}' "$OUT/roots" | grep -oE "'[^']+'" | sort | uniq -c | sort -rn | head -14
printf '\ntree: the single FILE each failure starts in, by how many others it stops\n'
awk -F'\t' '$1 != $2 {print $2}' "$OUT/roots" | sort | uniq -c | sort -rn | head -10
exit 0
