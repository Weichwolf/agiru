Type:     bug
Status:   open
Parent:   0014
Area:     rt
Source:   ~/Git/BCApps/src/Layers/W1/BaseApp/Inventory/Tracking/ItemTrackingLines.Page.al; SCM Available to Pick UT
Verdict:  measured
Class:    determinism

# A temporary record's table outlives every handle, so a later copy reads freed memory

**The symptom is non-deterministic and item-tracking-shaped.** `SCM Available to Pick UT` reads a
temporary `Tracking Specification` whose `TempTable` has already been freed. The dangling read lands
in `TempHandle::Acquire()` -- `++table_->held` on a deleted table -- and the value it finds decides
the failure mode:

- a length garbage-large out of the freed bytes -> `std::bad_array_new_length` from
  `AlArray::Take` (build 195, run 133: caught, the codeunit finished 32 of 49);
- a length under the guard but still a dead view -> `std::bad_alloc` (caught);
- an unmapped page -> `SIGSEGV`, which kills the whole codeunit process and takes its 49
  procedures out of the milestone denominator (build 196, run 134: LOST -> ABORT).

Because the value is whatever the allocator left there, the SAME binary crashes on one run and
finishes on the next. Two standalone runs of the codeunit on build 196: one finished 28 of 49, the
next SIGSEGV'd. That is a determinism defect (CLAUDE.md's compulsory invariant), not a test bug.

## Where it is, measured with `addr2line` over the SIGSEGV backtrace

```
TempHandle::Acquire()                                   ++table_->held on a freed TempTable
TempHandle(const TempHandle&)
RecordState(const RecordState&)
StateHandle(const StateHandle&)
TrackingSpecification_Table(const TrackingSpecification_Table&)   copying the page's temp Rec
TestPage<ItemTrackingLines_Page>::RunTrigger_(..., Lookup, ...)   `const auto before = Record_();`
TestPage<ItemTrackingLines_Page>::RunControlTrigger(...)
Page<ItemTrackingLines_Page>::RunModal()
```

`RunTrigger_`'s lookup branch copies `Record_()` before running the trigger (the save-if-validated
image). The copy is innocent; the page's `Rec` -- a `Temporary<Tracking Specification>` -- already
carries a `TempHandle` to a table that has been `delete`d. So the free happens earlier, during the
item-tracking data collection's setup of that page, and the copy is only where the corpse is
touched.

## What is ruled out

- **The refcount paths are correct.** `TempHandle`'s copy/move/assign/dtor balance `held`;
  `StateHandle::operator=` copies no state (fields only); `StateHandle(const&)` deep-copies with a
  matching `Acquire`; `CopyStateFrom` (Rec.Copy) was traced by hand and keeps the target's own
  table with the refcount consistent; `kTempOps::insert`/`load` `Forget()` the stored row's state
  so a stored row never carries a live handle; `RuntimeShareTemporary` shares through the handle's
  assignment. None of these frees a table another handle still points at.
- **It is not one of this round's fixes by itself.** batches 210-212 (temp `CalcSums`, ANSI
  encoding, `LogMessage`) were in the STABLE build 195 that finished SCM four runs running
  (130-133). The crash became frequent (~50 %) only in build 196, which added 213 (`OnSourceText`),
  214 (the `AlArray::Take` length guard) and 215 (packed/localized `DateFormula`). One of those
  three shifts the heap enough to move the dead read from a caught exception onto an unmapped page;
  the underlying free is older than all of them (board:0633's `bad_array_new_length` is the same
  corpse). Bisection of 213/214/215 is the next measurement.

## The fix is the premature free, not a guard

`AlArray::Take`'s length guard (214) only renames one manifestation. The table must not be freed
while a record still references it: the item-tracking setup hands the page a temporary whose store
belongs to a codeunit-local that has gone out of scope, or a `var` temporary parameter is being
stored by value past its owner's life. The store is the ItemTrackingDataCollection's; the exact
call that lets the page outlive it is what this item closes. Until then the milestone runner should
count a LOST codeunit against the full text denominator and re-attempt a codeunit whose process
died, so one corpse does not abort the whole measurement (a run with any LOST is still an ABORT).

## comment (2026-09-12) -- the base is stable, and the bisection has a shape

Reverting 213/214/215 and rebuilding 202-212 restored a DETERMINISTIC SCM Available to Pick UT:
two standalone runs, 35 of 49 both times, the SAME 14 failures, no crash. Milestone run 135 over
that build finished 2 200 of 2 310, 80 codeunits, 0 LOST -- the retry the runner grew this round
carried the flaky codeunit across the occasional dead read without a single lost codeunit.

The bisection of what tips the corpse is narrowing by construction, not yet by measurement: only
213 (`OnSourceText`) regenerates the `Item Tracking Lines` page -- it gives its expression controls
`AvailabilitySerialNo`/`AvailabilityLotNo` (source `TrackingAvailable(Rec, ...)`) an `OnSourceText`
method, growing the page object and shifting where the freed table's memory lands. 214 (AlArray
guard) and 215 (packed/localized DateFormula) change no item-tracking app source -- `Tracking
Specification` has no DateFormula field -- so the same-round build B (202-212 + 214 + 215) is the
test of whether a runtime-only heap shift is enough to tip it, or whether 213's page-code growth is
the trigger. Either way the free is board:0718's and predates all three.

A mitigation for 213 when it returns: synthesize `OnSourceText` only for a source that is NOT a
top-level function call (114 of the 477 expression-source field controls call a function; the board
targets -- `Vendor.Name`, `Balance + Overdue` -- are field access and arithmetic). That both shrinks
213's footprint on the item-tracking page and stops running a side-effecting function on every
control read. It is a mitigation, not the fix; the fix is finding the premature free, which needs a
sanitizer build this box does not yet carry.
