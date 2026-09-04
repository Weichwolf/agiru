Type: root
State: open
Area: rt, gen

**FILED AS 0071 AND RENUMBERED TO 0580.** Two branches issued the number at the same hour: this
item on `main` and the documentation sweep's own, which carries a ledger 468 items reference by
number. A number is issued ONCE, so the smaller side moved -- and it is written here rather
than nowhere, because the commits that made this item still say the old one.

# The slice links, or `make` is red -- and the ApplicationArea chain is what it is waiting for

## Measured 2026-09-04: the slice is OPEN by 1 072 symbols and the loader picks which one kills the run

`nm -uC build/libagiru_slice.so | grep agiru::app` counts **1 072 undefined symbols** over some
sixty objects. That is not new -- it was true while the run reported 7 of 61 -- and the reason the
run worked at all is that nothing REACHED any of them. Implementing `Database.UserId()` moved one
into reach and the process died at load with `symbol lookup error`, which is what board:0071
originally recorded as "the run dies at the first call".

**So the criterion is not "the slice compiles" and it is not "the slice is closed" either -- it is
whether a symbol a test reaches has a definition linked**, and no static check answers that without
a call graph. Three findings from working it:

- **Shrinking the slice is the wrong move and it was tried.** A script that dropped whichever slice
  entry named the missing owner took out `ERMVATToolUT` and `ERMDocumentTotalsUT`, which are
  COMMITTED entries, and the run fell from 7 of 61 to 7 of 18. The slice may only grow; the fix is
  always on the callee's side.
- **The callees are the TEST LIBRARIES and most of them compile.** `LibrarySales`,
  `LibraryERMCountryData` and `EnvironmentInfoTestLibrary` are clean today;
  `LibraryNonDeductibleVAT` failed on one `-Wunused-variable` in generated code and `LibraryERM` on
  one `Validate(Field, Variant)`. Both were generic gaps and both are closed now -- which is the
  shape this item should expect: **a library that does not link names one gap in `src/`, not a
  reason to leave it out.**
- **`-Wl,--no-undefined` is still the wrong gate at 1 072.** It would make `make` red until the
  whole transitive closure compiles, and `make` is the one-second loop. What fits is a COUNTER: the
  undefined-symbol count beside the slice's line count, which a commit may lower and never raise.

`test/slice` names the generated sources linked into `agiru`, and CLAUDE.md gives it one rule: it
may only GROW, and `make` is red the day one of them stops compiling. **It says nothing about
LINKING, and that gap has a shape:** a slice with an unresolved symbol builds green, and the run
dies at the first call with

```
./build/agiru: symbol lookup error: libagiru_slice.so: undefined symbol:
  _ZN5agiru3app9codeunits31ConfPersonalizationMgt_Codeunit24GetCurrentProfileNoErrorE...
```

which reports NOTHING -- not `0 of 61`, not a failed test, the process is gone. That is the worst
shape a gate takes: it is not even a wrong number.

**`-Wl,--no-undefined` on the slice library turns it into a link error**, which is where it belongs.
It is not applied yet, and the reason is a rule collision that has to be decided rather than
guessed: with it, a slice member whose closure is incomplete cannot be in the slice -- and the slice
may only GROW, so nothing may leave. Today `ERM Document Totals UT` is in it and its closure is not
complete, so the two rules cannot both hold.

## What the chain actually is, measured 2026-09-04

`Clear(Any)` was a door refusal and is now implemented, which let `ERM Document Totals UT` run
further than it ever had. Each step exposed the next:

| reached | needed | state |
|---|---|---|
| `Clear(Rec)` | the field walk | **done** |
| `ApplicationAreaMgmtFacade.SetupApplicationArea` | that codeunit compiling | **done**, and it cost five generic fixes |
| `ApplicationAreaMgmt` | `Page<>::RunModal`, `TableTraits<Temporary<T>>`, `List<T>` from `List<U>`, `RecordRef.SetTable(Record)`, `[TryFunction]` | **done**, all five |
| `Database.UserId()` | the session's user | **taken back, see below** |
| `ConfPersonalizationMgt` | 22 errors, not yet counted into classes | OPEN |

**`UserId()` IS IMPLEMENTABLE AND WAS TAKEN BACK, WHICH IS THE ACTIVATION RULE WORKING.** The
session holds the user and `SYSTEM` is the measured answer -- the predecessor returns exactly that
constant unless a test overrides it and reached 97.0 % of the UT subset over it. Implementing it
moved `agiru run-tests` from `7 of 61` to a process that dies, because the next call in the same
body reaches `ConfPersonalizationMgt`. CLAUDE.md: "activation -- a previously dead path now runs,
often net negative because cases were green over the no-op, so always a full A/B, and on a loss the
list names the deeper roots and those come first." The deeper root is this chain, and it is named
here.

## The choice

- **`--no-undefined` goes on when the slice's closure is complete**, and until then the slice's
  own rule needs the other half written down: a member may only leave when the AL object is gone,
  AND a member may only ENTER when its closure links. The second half is what was missing.
- **The chain is closed from the far end**, not from the near one: `ConfPersonalizationMgt` first,
  then `UserId` goes back in, and the A/B is `agiru run-tests` before and after.
