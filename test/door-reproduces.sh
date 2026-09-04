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
before=$(cat include/Builtins.h src/rt/Builtins.cpp | sha1sum | cut -d' ' -f1)
python3 scripts/gen_builtins.py > /dev/null
clang-format -i include/Builtins.h src/rt/Builtins.cpp 2> /dev/null || true
after=$(cat include/Builtins.h src/rt/Builtins.cpp | sha1sum | cut -d' ' -f1)

if [ "$before" != "$after" ]; then
  printf 'door: the generator does not reproduce the door it is standing on.\n' >&2
  printf 'door: `git diff include/Builtins.h src/rt/Builtins.cpp` says what moved.\n' >&2
  exit 1
fi

# AND NOTHING WRITTEN REFUSES. A written function that ends in `RefuseDoor` is one the generator
# took back, which is the failure this whole split exists to stop -- and it would leave the digest
# above unchanged, because by then the refusal IS what is on disk.
if grep -q RefuseDoor src/rt/written/BuiltinsWritten.cpp; then
  printf 'door: a written builtin refuses. It was overwritten by the generator.\n' >&2
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
