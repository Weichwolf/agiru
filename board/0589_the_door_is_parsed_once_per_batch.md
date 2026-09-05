
## What the slice costs now, measured 2026-09-05 over the settle rounds

| | |
|---|---:|
| slice sources | 1 545 |
| full build, unity, two cores | 622-680 s (stop at first error) |
| full build with `KEEP=1` | 1 667 s (every unit, every error) |
| transpile | 51 s |
| one added source, incremental | 3-13 s |

The stop-at-first-error build is HALF the keep-going one, which is the argument for both: the
default stops, and a round that is hunting a CLASS of failures asks for `KEEP=1` and gets every
unit in one wait instead of one per quarter hour. The 1 667 s round that found three errors in one
file is what a green tree costs to prove.

**The next cut is the per-file door parse over 1 545 sources, and the unity groups already take it
from 8.2 s to 2.9 s per translation unit.** What is left is the door itself: `Builtins.h` reaches
`RecordRef.h` reaches `meta/Ids.h` reaches `<compare>`, and a generated file that names none of
them still pays for them through the precompiled header.
