#!/bin/sh
# `make gap` -- the generated header that blocks the MOST others, and the diagnostic that stops it.
#
# EVERY ERROR IN `apps/` IS A GENERIC GAP IN `src/`. That is the invariant, not an aspiration: the
# transpiler and the runtime know no AL object, so a generated file cannot be wrong about itself --
# it is wrong about what the generator emitted or what the runtime offers.
#
# IT ASKS THE CENSUS FIRST, AND THAT IS THE WHOLE SPEED ARGUMENT. `make tree` already compiled every
# header and recorded, per failure, the file the FIRST diagnostic came from -- the root. Sweeping
# the tree again to rediscover one of those costs one compile per header that already passed, and
# that cost GROWS with every repair: the further the tree gets, the longer the loop that carries it
# gets. Reading the census instead costs one compile, and it names the root that blocks 80 headers
# rather than the one that happens to sort first.
#
# A ROOT LEAVES THE CENSUS BY COMPILING, never by being crossed off. So a repair is confirmed the
# same way it was found, and a census that has gone stale ends the run by saying so rather than by
# reporting success.
set -eu
cd "$(dirname "$0")/.."

APPS=${1:-apps}
OUT=build/first-gap
PCH=$OUT/agiru.pch
CENSUS=build/tree-syntax/roots

includes="-Iinclude"
for d in "$APPS"/*/; do
  [ -d "$d" ] && includes="$includes -I${d%/}"
done

mkdir -p "$OUT"
clang++ -std=c++23 -O2 $includes -x c++-header -o "$PCH" include/agiru.h 2>"$OUT/pch.log" || {
  printf 'gap: the door does not precompile -- see %s\n' "$OUT/pch.log" >&2
  exit 1
}
includes="$includes -include-pch $PCH"

unit=$(mktemp --suffix=.cpp)
err=$(mktemp)
trap 'rm -f "$unit" "$err"' EXIT

# A HEADER IS INCLUDED, NOT COMPILED. `#pragma once` has NO EFFECT IN THE MAIN FILE, so a header
# compiled directly that is reached again through one of its own includes is read twice and every
# class in it is a redefinition -- the measurement lying about a tree a real build is fine with.
compiles() {
  # A SOURCE IS COMPILED AND A HEADER IS INCLUDED. `#pragma once` has no effect in the main file, so
  # a header compiled directly that is reached through one of its own includes is read twice.
  if [ -n "${SOURCE:-}" ]; then
    clang++ -std=c++23 -O2 -fsyntax-only -ferror-limit=1 -Wall -Wextra -Wpedantic \
      $includes "$1" 2>"$err"
    return
  fi
  case $1 in
    /*) printf '#include "%s"\n' "$1" > "$unit" ;;
    *) printf '#include "%s/%s"\n' "$PWD" "$1" > "$unit" ;;
  esac
  clang++ -std=c++23 -O2 -fsyntax-only -ferror-limit=1 -Wall -Wextra -Wpedantic \
    $includes "$unit" 2>"$err"
}

# `SOURCE=1` WALKS THE BODIES INSTEAD OF THE DECLARATIONS. A header is what a signature says and a
# source is what the AL statements became, so the two fail for different reasons and the second set
# has never been measured (board:0038). `make apps` finds the same defects and pays a full ninja
# rebuild for every header the repair touches; this pays one translation unit.
KIND=${SOURCE:+*.cpp}
KIND=${KIND:-*.h}

sweep() {
  : > "$OUT/files"
  for app in $(python3 -c "
import json
print(' '.join(a['name'] for a in json.load(open('apps.json'))['apps']))"); do
    [ -d "$APPS/$app" ] && find "$APPS/$app" -name "$KIND" | sort >> "$OUT/files"
  done
  total=$(wc -l < "$OUT/files")
  seen=0
  while IFS= read -r file; do
    seen=$((seen + 1))
    compiles "$file" && continue
    printf 'gap: %s of %s headers compile, then\n\n' "$((seen - 1))" "$total"
    cat "$err" >&2
    printf '\ngap: repair it in src/gen or src/rt. A fix inside apps/ does not survive the next run.\n'
    exit 1
  done < "$OUT/files"
  printf 'gap: all %s generated headers compile.\n' "$total"
}

[ "${SWEEP:-0}" = 1 ] || [ -n "${SOURCE:-}" ] && { sweep; exit 0; }
[ -s "$CENSUS" ] || {
  printf 'gap: no census under %s -- sweeping. `make tree` writes one.\n\n' "$CENSUS"
  sweep
  exit 0
}

# THE RANKING IS BY DEPENDENTS AND THEN BY NAME, so the same census always names the same root.
ranked=$(cut -f2 "$CENSUS" | sort | uniq -c | sort -k1,1nr -k2,2)
roots=$(printf '%s\n' "$ranked" | wc -l)
tried=0
printf '%s\n' "$ranked" | while read -r blocked root; do
  tried=$((tried + 1))
  [ -f "$root" ] || continue
  compiles "$root" && continue
  printf 'gap: %s blocks %s of %s failing headers, %s root(s) in the census, then\n\n' \
    "$root" "$blocked" "$(wc -l < "$CENSUS")" "$roots"
  cat "$err" >&2
  printf '\ngap: repair it in src/gen or src/rt. A fix inside apps/ does not survive the next run.\n'
  exit 1
done
status=$?
[ "$status" -ne 0 ] && exit "$status"
printf 'gap: every one of the %s root(s) in the census compiles -- the census is spent. `make tree`.\n' "$roots"
