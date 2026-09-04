Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onaftergetrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-onaftergetrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnAfterGetRecord` runs PER ROW, and it is the busiest page trigger in the tree

```al
trigger OnAfterGetRecord()
```

"Runs after a record is retrieved from a table **but before it is displayed to the user**." It is
where a page computes what it shows but does not store -- a style expression, a derived caption, a
FlowField it wants calculated once.

**It runs once per ROW.** A list showing 50 rows runs it 50 times, and that is what makes its
population the number that decides the page runtime's cost.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterGetRecord()` on a page or pageextension: **8 136 declarations** -- the largest of
the fourteen page triggers by a factor of two, and larger than any table trigger in the tree.

## The IST-state

No page runtime: `include/runtime/test/TestPage.h` derives from the generated page class and every
`TestField` body reaches `Unopened()` (`src/rt/TestPage.cpp`). The trigger is emitted as a member of
the generated page class -- `src/gen/PageWriter.cpp` writes the page's triggers -- and nothing calls
it.

## The choice

The call sits in the page's row loop, after the record is read and before the row is rendered, and
it is the same call site the per-row event `OnAfterGetRecordEvent` (0261) uses -- one place, trigger
first then event, which is the order every other pair in this family follows.

**At 8 136 declarations and 50 rows a render, this is the hot path of the whole UI.** board:0009's
code-locality argument applies here before anywhere else, and whatever the page runtime does per row
is multiplied by both numbers.

## Ordering

Blocked on board:0030. First of the fourteen by population.

## Gate, and its negative control

A list page over five rows whose `OnAfterGetRecord` counts its calls: the count is five, and each
call saw a different record.

**The negative control is "a different record"** -- a runtime that calls it once per page passes a
test that only asserts it ran, and every derived value on the list is then the first row's.
