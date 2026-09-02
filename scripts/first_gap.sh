#!/bin/sh
# `make gap` -- the FIRST generated header that does not compile, and the diagnostic that stops it.
#
# EVERY ERROR IN `apps/` IS A GENERIC GAP IN `src/`. That is the invariant, not an aspiration: the
# transpiler and the runtime know no AL object, so a generated file cannot be wrong about itself --
# it is wrong about what the generator emitted or what the runtime offers. Enumerating three
# thousand downstream failures therefore buys nothing over finding one and repairing its root.
#
# THE ORDER IS SORTED AND THE RUN IS SEQUENTIAL, so the same tree always names the same file.
# Parallel would find A failure sooner and a different one each time, which is not a starting point
# anybody can return to.
set -eu
cd "$(dirname "$0")/.."

APPS=${1:-apps}
OUT=build/first-gap
PCH=$OUT/agiru.pch

includes="-Iinclude"
for d in "$APPS"/*/; do
  [ -d "$d" ] && includes="$includes -I${d%/}"
done

mkdir -p "$OUT"
clang++ -std=c++23 -O2 $includes -x c++-header -o "$PCH" include/agiru.h 2>"$OUT/pch.log" || {
  printf 'gap: the door does not precompile -- see %s\n' "$OUT/pch.log" >&2
  exit 1
}

# THE ORDER IS DEPENDENCY ORDER AND THEN PATH. Alphabetically `apps/base` comes before
# `apps/system`, which is backwards: base STANDS ON system, so a gap in system blocks everything
# above it and is worth more per repair. `apps.json` already lists the apps in dependency order --
# it is the order the transpiler resolves names in -- so the sort follows it and the path decides
# within an app.
: > "$OUT/files"
for app in $(python3 -c "
import json, sys
print(' '.join(a['name'] for a in json.load(open('apps.json'))['apps']))"); do
  [ -d "$APPS/$app" ] && find "$APPS/$app" -name '*.h' | sort >> "$OUT/files"
done
total=$(wc -l < "$OUT/files")
seen=0

unit=$(mktemp --suffix=.cpp)
trap 'rm -f "$unit"' EXIT

while IFS= read -r file; do
  seen=$((seen + 1))
  err=$(mktemp)
  # A HEADER IS INCLUDED, NOT COMPILED. `#pragma once` has NO EFFECT IN THE MAIN FILE -- clang says
  # so with `-Wpragma-once-outside-header` -- so a header compiled directly that is reached again
  # through one of its own includes is read twice and every class in it is a redefinition. That is
  # the measurement lying about the tree: `AgentMessageImpl.h` includes `AgentTaskImpl.h`, which
  # includes it back, and a real build has never had a problem with it.
  printf '#include "%s"\n' "$file" > "$unit"
  if clang++ -std=c++23 -O2 -fsyntax-only -ferror-limit=1 -Wall -Wextra -Wpedantic \
      $includes -include-pch "$PCH" "$unit" 2>"$err"; then
    rm -f "$err"
    continue
  fi
  printf 'gap: %s of %s headers compile, then\n\n' "$((seen - 1))" "$total"
  cat "$err" >&2
  rm -f "$err"
  printf '\ngap: repair it in src/gen or src/rt. A fix inside apps/ does not survive the next run.\n'
  exit 1
done < "$OUT/files"

printf 'gap: all %s generated headers compile.\n' "$total"
