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

# THE FIXED POINT IS GENERATE AND THEN FORMAT, because that is what `make` does: it formats every
# file that differs from HEAD, so a generator whose output is not already formatted would fail this
# on the second build and pass on the first.
# THE DOOR'S MTIME IS PART OF THE BUILD. A regenerated `Builtins.h` with the same bytes and a new
# time stamp rebuilds every generated translation unit on the next `make` -- fifteen minutes for
# nothing, once per gate run (measured 2026-09-11). So the originals are kept beside the run and
# put back, time stamp and all, whenever the generator reproduced them.
before=$(cat include/Builtins.h src/rt/Builtins.cpp | sha1sum | cut -d' ' -f1)
keep=$(mktemp -d)
cp -p include/Builtins.h "$keep/Builtins.h"
cp -p src/rt/Builtins.cpp "$keep/Builtins.cpp"
python3 scripts/gen_builtins.py > /dev/null
for f in include/Builtins.h src/rt/Builtins.cpp; do
  clang-format "$f" 2> /dev/null | cmp -s - "$f" || clang-format -i "$f" 2> /dev/null || true
done
after=$(cat include/Builtins.h src/rt/Builtins.cpp | sha1sum | cut -d' ' -f1)
if [ "$before" = "$after" ]; then
  cp -p "$keep/Builtins.h" include/Builtins.h
  cp -p "$keep/Builtins.cpp" src/rt/Builtins.cpp
fi
rm -rf "$keep"

if [ "$before" != "$after" ]; then
  printf 'door: the generator does not reproduce the door it is standing on.\n' >&2
  printf 'door: `git diff include/Builtins.h src/rt/Builtins.cpp` says what moved.\n' >&2
  exit 1
fi

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
