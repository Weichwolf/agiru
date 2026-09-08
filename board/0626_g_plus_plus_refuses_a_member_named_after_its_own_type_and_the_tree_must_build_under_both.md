Type:     root
Status:   open
Area:     gen, rt
Source:   the first g++ build of the tree in weeks, made by accident while chasing board:0625, 2026-09-08
Class:    silent-wrong-data

# g++ refuses a member named after its own type, and the tree must build under BOTH front ends

**THE TREE DOES NOT BUILD UNDER g++-14, AND NOBODY KNEW**, because every build for weeks was
clang++-19. CLAUDE.md commits to both -- "clang++-19 is the reference compiler, g++-14 must
translate the same tree" -- and the second front end is "most of the enforcement" for the
portability rule. It was found by configuring a sanitizer build without naming the compiler:

```
include/platform/Field.h:205:25: error: declaration of
  'agiru::Option<agiru::platform::ObsoleteState> agiru::platform::Field::ObsoleteState'
  changes meaning of 'ObsoleteState' [-Wchanges-meaning]
```

`Option<ObsoleteState> ObsoleteState;` -- a field whose AL NAME is the same word as its option's
TYPE. Inside the class the member hides the type from that line on, and g++ makes the reuse a
hard error under `-Werror`; clang accepts it. **Both are within the standard** ([basic.scope.class]
makes the program ill-formed, no diagnostic required), which is exactly the kind of difference a
second front end exists to find.

## Population, measured 2026-09-08

| where | members spelled `Option<X> X;` or `Enum<X> X;` |
|---|---|
| the door (`include/`) | 1 |
| the generated tree (`apps/`, headers) | 0, in 0 headers |

**IT IS ONE LINE, AND THE COUNT IS THE POINT.** The first reading of this sweep -- an `ugrep` that
does not know a backreference -- reported 335 in the generated tree. A real regex engine reports
ZERO there and one in the door: the generator already spells an option's type through its
synthetic name (`Option<OptionVisitPreUpdatePostUpdate> ...`), so the AL field name can never
equal it. The one is `platform/Field.h:205`, a hand-written platform table. That is CLAUDE.md's
"a row a script filled can be wrong", one more time.

## What is wanted

The one member's type spelled so that the member no longer hides it -- `::agiru::platform::
ObsoleteState` -- and **a g++ leg on `make`**, because a commitment nobody runs lapses: this tree
built under g++ on 2026-09-03 and stopped building under it some day nobody can name.

## What proves it

`CXX=g++-14 make` and `make` both at 0 errors over the same tree, and the count above at 0 in
`apps/`. The negative control is this very header before the fix.
