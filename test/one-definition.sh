#!/bin/sh
# ONE DEFINITION PER SYMBOL ACROSS THE TIERS. `src/rt/Door.cpp` carried a refusing body for every
# `TextBuilder` method beside the real ones in `src/net/TextBuilder.cpp`, and the loader took the
# refusal: a whole type answered "not implemented yet" while its implementation sat in the next
# library (15 symbols, 2026-09-09). The linker cannot see it -- each shared object is consistent
# on its own -- so this compares what the tiers EXPORT and refuses the first duplicate.
set -eu
B="${B:-$(dirname "$0")/../build}"
libs=$(find "$B" -maxdepth 1 -name 'libagiru_*.so' -type f | sort)
if [ -z "$libs" ]; then
  printf 'one-definition: no libagiru_*.so under %s -- ABORT, not a pass\n' "$B" >&2
  exit 1
fi
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
: > "$tmp/symbols"
for lib in $libs; do
  nm -C --defined-only "$lib" > "$tmp/nm" || exit 2
  awk -v l="$(basename "$lib")" '$2=="T"{$1="";$2=""; print substr($0,3) "\t" l}' "$tmp/nm" >> "$tmp/symbols"
done
sort -u "$tmp/symbols" > "$tmp/unique"
# A CONSTRUCTOR IS TWO SYMBOLS WITH ONE NAME (the complete and the base object variants), and both
# live in the same tier; the pair is folded first so only a name in two TIERS counts.
dups=$(cut -f1 "$tmp/unique" | uniq -d | grep -v '^$' || true)
if [ -n "$dups" ]; then
  n=$(printf '%s\n' "$dups" | wc -l)
  printf 'one-definition: %s symbol(s) defined in more than one tier:\n' "$n" >&2
  printf '%s\n' "$dups" | head -20 | while IFS= read -r s; do grep -F "$s	" "$tmp/unique" | head -2 | sed 's/^/        /' >&2; done
  exit 1
fi
printf 'one-definition: every exported symbol has one tier\n'
