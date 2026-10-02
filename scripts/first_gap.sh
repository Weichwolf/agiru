#!/bin/sh
# Rank recorded header roots; SOURCE=1 or SWEEP=1 checks a complete file inventory directly.
# A sweep stops at its first failure. A spent census requires a fresh tree measurement.
set -eu
cd "$(dirname "$0")/.."

APPS=${1:-apps}
OUT=build/first-gap
PCH=$OUT/agiru.pch
CENSUS=build/tree-syntax/roots
WARNINGS='-Wall -Wextra -Wpedantic -Werror'

[ -d "$APPS" ] || {
  printf 'gap: %s does not exist -- run `make transpile` first\n' "$APPS" >&2
  exit 2
}

includes="-Iinclude"
for d in "$APPS"/*/; do
  [ -d "$d" ] && includes="$includes -I${d%/}"
done

mkdir -p "$OUT"
clang++ -std=c++23 -O2 $WARNINGS $includes -x c++-header -o "$PCH" cmake/Precompiled.h 2>"$OUT/pch.log" || {
  printf 'gap: the door does not precompile -- see %s\n' "$OUT/pch.log" >&2
  exit 1
}
includes="$includes -include-pch $PCH"

unit=$(mktemp --suffix=.cpp)
err=$(mktemp)
trap 'rm -f "$unit" "$err"' EXIT

compiles() {
  # A SOURCE IS COMPILED AND A HEADER IS INCLUDED. `#pragma once` has no effect in the main file, so
  # a header compiled directly that is reached through one of its own includes is read twice.
  if [ -n "${SOURCE:-}" ]; then
    clang++ -std=c++23 -O2 -fsyntax-only -ferror-limit=1 $WARNINGS \
      $includes "$1" 2>"$err"
    return
  fi
  case $1 in
    /*) printf '#include "%s"\n' "$1" > "$unit" ;;
    *) printf '#include "%s/%s"\n' "$PWD" "$1" > "$unit" ;;
  esac
  clang++ -std=c++23 -O2 -fsyntax-only -ferror-limit=1 $WARNINGS \
    $includes "$unit" 2>"$err"
}

KIND='*.h'
LABEL=headers
if [ -n "${SOURCE:-}" ]; then
  KIND='*.cpp'
  LABEL=bodies
fi

sweep() {
  python3 -c "
import json
apps = json.load(open('apps.json'))['apps']
if not apps:
    raise SystemExit('gap: no declared apps -- ABORT')
print('\n'.join(a['name'] for a in apps))" > "$OUT/apps"
  while IFS= read -r app; do
    [ -d "$APPS/$app" ] || {
      printf 'gap: declared app %s does not exist under %s -- ABORT\n' "$app" "$APPS" >&2
      exit 2
    }
  done < "$OUT/apps"
  find "$APPS" -type f -name "$KIND" > "$OUT/files"
  sort -o "$OUT/files" "$OUT/files"
  total=$(wc -l < "$OUT/files")
  [ "$total" -gt 0 ] || {
    printf 'gap: no generated %s under %s -- ABORT\n' "$LABEL" "$APPS" >&2
    exit 1
  }
  seen=0
  while IFS= read -r file; do
    seen=$((seen + 1))
    compiles "$file" && continue
    printf 'gap: %s of %s generated %s compile, then %s\n\n' "$((seen - 1))" "$total" "$LABEL" "$file"
    cat "$err" >&2
    printf '\ngap: repair it in src/gen or src/rt. A fix inside apps/ does not survive the next run.\n'
    exit 1
  done < "$OUT/files"
  printf 'gap: all %s generated %s compile.\n' "$total" "$LABEL"
}

[ "${SWEEP:-0}" = 1 ] || [ -n "${SOURCE:-}" ] && { sweep; exit 0; }
[ -s "$CENSUS" ] || {
  printf 'gap: no census under %s -- `make gap` must create it before selecting a root.\n' "$CENSUS" >&2
  exit 2
}

# THE RANKING IS BY DEPENDENTS AND THEN BY NAME, so the same census always names the same root.
ranked=$(cut -f2 "$CENSUS" | sort | uniq -c | sort -k1,1nr -k2,2)
roots=$(printf '%s\n' "$ranked" | wc -l)
printf '%s\n' "$ranked" | while read -r blocked root; do
  [ -f "$root" ] || {
    printf 'gap: census root %s does not exist -- `make tree`.\n' "$root" >&2
    exit 2
  }
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
exit 2
