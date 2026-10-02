#!/bin/sh
# THE DOOR'S GENERATOR REPRODUCES THE DOOR. `scripts/gen_builtins.py` writes `include/Builtins.h`
# and `src/rt/Builtins.cpp`, and for a long while running it DESTROYED work: twelve of those
# functions had been written into afterwards, and each came back a refusal. Silently -- the build
# stayed green and the behaviour went away.
#
# They live in `BuiltinsWritten.h` and `BuiltinsWritten.cpp` now, which the generator excludes by
# construction: it skips whatever another door header declares. This asserts that it stays true.
set -eu
cd "$(dirname "$0")/.."

candidate=$(mktemp -d)
trap 'rm -rf "$candidate"' EXIT HUP INT TERM
python3 scripts/gen_builtins.py --output-root "$candidate" >/dev/null
for file in include/Builtins.h src/rt/Builtins.cpp; do
  if ! cmp -s "$file" "$candidate/$file"; then
    printf 'door: generator does not reproduce %s\n' "$file" >&2
    diff -u "$file" "$candidate/$file" || true
    exit 1
  fi
done

# AND NOTHING WRITTEN REFUSES OUTRIGHT. A written function whose FIRST statement is `RefuseDoor` is
# one the generator took back, which is the failure this whole split exists to stop -- and it would
# leave the digest above unchanged, because by then the refusal IS what is on disk.
#
# A CONDITIONAL REFUSAL IS NOT THAT. `GuiAllowed` answers true when a test has installed handlers
# and refuses otherwise; `Hyperlink` hands the URL to a handler and refuses when none takes it.
# Both compute first, and both are written precisely so the generator cannot take the computation
# away. So the test is on the FIRST statement of the body, not on the presence of the word.
outright=$(awk '/\{$/ { open = 1; next } open && /^  RefuseDoor\(/ { count++ } { open = 0 } \
  END { print count + 0 }' src/rt/written/BuiltinsWritten.cpp)
if [ "$outright" -gt 0 ]; then
  printf 'door: %s written builtin(s) refuse outright. Overwritten by the generator.\n' \
    "$outright" >&2
  exit 1
fi

# A COUNT OF 0 IS AN ABORT AND NOT A PASS. An empty written file reproduces itself perfectly and
# refuses nothing, which is the greenest way this gate could lie.
written=$(grep -c '^}$' src/rt/written/BuiltinsWritten.cpp || true)
written=$((written - 1))
if [ "$written" -lt 1 ]; then
  printf 'door: no builtin is written at all. There were twelve. ABORT, not a pass.\n' >&2
  exit 1
fi
printf 'door: reproduces itself, and %s written builtin(s) still compute.\n' "$written"
