Type:     task
Status:   open
Area:     build
Source:   a syntax check over every generated source, 2026-09-07
Verdict:  teilweise
Class:    activation

# The slice is every generated source that compiles, and the sweep says which

**The slice was small for the wrong reason.** The settle loop adds a source only when one of its
symbols is MISSING from the link, so it walks the undefined-symbol closure and never asks the other
6 049 sources whether they would compile. They were never tried.

## Measured 2026-09-07, one `-fsyntax-only` per source, 2h30 on two cores

| | |
|---|---:|
| generated sources | 7 913 |
| in the slice before | 2 064 |
| swept (the rest) | 6 049 |
| **compiled clean** | **4 914 -- 81 %** |
| did not | 1 135 |

**The slice is 6 784 now**, and the two that the full build then found -- `PermissionSetList.cpp`
and `HttpContentImpl.cpp`, both `no member named ... in a generated codeunit` -- are out of it
again. They were clean in the sweep and dirty afterwards because the sweep ran against the tree
BEFORE the round's last transpile, which is the sweep's one methodological hole: it measures a tree
that the next generator change invalidates.

## What the 1 135 are, classified over the first 850

| class | share |
|---|---:|
| a member missing on an absent AL object | 36 % |
| a member missing on a .NET stub or a refusal | 18 % |
| an array bound by reference across element lengths (board:0597) | 9 % |
| an ambiguous overload where a refusal is the argument | 6 % |
| a member missing on a generated page, table or codeunit | 20 % |
| the rest, one-offs | 11 % |

## What is not proved

**The full build with the grown slice has not finished green.** 1 698 s for 6 778 sources is the
measurement of the round's build change -- 3.3x the sources in 74 % of the time -- and it ended on
those two files. The next run is what says whether the link closes, which is the whole point:
`agiru run-tests` has never started, and 837 undefined symbols is why.
