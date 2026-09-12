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

## comment (2026-09-12, part 2) -- the ShareTable sites, and why reading is not enough

The `Item Tracking Lines` page shares its `Rec` temp table into locals with `Copy(Rec, true)` in at
least three places (`ItemTrackingLines.Page.al:957` `CountLinesWithQtyZero`, `:1444`, `:1500`), and
`ContinuousItemTracking`/`ContinuousScanningLine` share it into `Rec` the other way. Every one of
these was traced by hand against `RuntimeShareTemporary` and `TempHandle`'s assignment: the local
default-constructs its own table, `Copy(_, true)` releases that (refcount 0 -> delete) and acquires
the page's (refcount 2), and the local's destructor releases it back to 1 -- balanced. The same
holds for `Forget`, `kTempOps::insert/load`, `StateHandle`'s copy/assign, and `Record::Copy`'s
`CopyStateFrom`. So the premature free is NOT in any refcount path a reader can see; every one is
correct.

That is the signal to stop reading and measure: the next step is a sanitizer build (`-fsanitize=
address`) of `src/` + the `SCM Available to Pick UT` slice run under the codeunit, which names the
allocation and the free with stacks. This box does not carry that build yet; standing it up (a
separate CMake config, ASan-instrumented libc++), is the work that closes this. Until then the
milestone retry keeps the measure honest and 213 (`OnSourceText`) stays shelved because its page-code
growth is what tips the corpse from a caught exception onto an unmapped page.

## comment (2026-09-12, part 3) -- it gates EVERY codegen change, not just 213

board:0718 was taken to be 213-shaped (213 regenerates the item-tracking page). It is worse than
that. batch218 (a generic transpiler fix: a table's/page's own `[TryFunction]` called `if not X()`
is wrapped in `Tried`) touches 175 generated units and NONE of them is `ItemTrackingLines` -- yet
build C (202-212+214+215+218) SEGFAULTs `SCM Available to Pick UT` on BOTH standalone runs, where
build B (202-212+214+215, runtime-only) was a deterministic 35 of 49 on both. Milestone run 137
finished 2 194 of 2 310 only because the retry carried the crash; the A/B is -7 / +0, and every one
of the seven is an item-tracking Pick case (`PickPositive`, `PickAndShipment*`, the breakbulk
summary) failing on `Qty. to Handle ... is 4, must be 10` -- the dead read handing back a partial
quantity.

So ANY change that shifts the heap -- a codegen change to unrelated objects is enough -- tips this
corpse from the lucky layout (build B, 35) to a crash. That makes board:0718 the gate on all
transpiler work, not a side issue: 213 and 218 are both correct and both unshippable until the
premature free is found. batch218 is taken back (reverted on the tree, kept as `$S/batch218.py`) and
waits on this; batch213 the same. The measurement stands: two correct fixes, both -7 through the
same corpse.

The fix needs a sanitizer build. Runtime-only fixes (214, 215) do NOT tip it -- they keep the
build-B layout -- so work that does not change codegen can still ship while this is open.

## comment (2026-09-12, part 4) -- diagnosed with a live-set tracer: Rec's StateHandle is CORRUPTED

Built a `thread_local` live-set / freed-map tracer of `TempTable` (env `AGIRU_TRACE_TEMPFREE`,
hooks in `TempTable` ctor/dtor and `TempHandle::Acquire`) plus a `StoredImage` branch log and
`PushBefore`/`PopBefore` logging. It CAUGHT the bad access and localised it precisely:

- The dead `Acquire` fires while copying a `Tracking Specification` in
  `ItemTrackingLines.AssignSerialNoBatch`, at the `OnAfterAssignNewTrackingNo(Rec, xRec, ...)` raise
  whose 2nd (`xRec`) parameter is BY VALUE, so `xRec` is copied.
- `xRec` came from `StoredImage()`'s `OutermostBefore` branch -- a live before-image
  (`Derived before = *this` in a `Rec.Validate("Lot No.", ...)` inside `AssignNewLotNo`), pushed and
  not yet popped, at a stack address. So the before-image is NOT dangling; it is a faithful copy of
  `Rec`.
- Therefore `Rec` ITSELF -- the page's `Temporary<Tracking Specification>` member -- has a
  `StateHandle::state_` that is a GARBAGE non-null pointer (value seen: 0x20, 0x7f4...): `new
  RecordState(*Rec.state_)` reads junk, and its `temporary.table_` is "never born" (not a real
  `TempTable`, not a freed one). Every temp-handle path (`Acquire`/copy/assign/move/`Forget`/
  `RuntimeShareTemporary`/`kTempOps::load`/`CopyStateFrom`) was traced by hand and sets `state_` only
  to null or a valid pointer -- so the garbage arrives by a RAW MEMORY OVERWRITE into the page Rec's
  first bytes (State_Block is at offset 0), data-dependent (only some item-tracking cases; the
  codeunit still reaches 35/49). No `memcpy`/`memset` in `src/rt` touches a record, so the write is
  in generated code or a field/array write overrunning into offset 0.

**This needs a sanitizer or a hardware watchpoint on `&Rec.State_Block` -- the one tool this box
lacks.** The tracer stays as `$S/tempdiag.py` + `$S/TempTableDiagnostic.cpp` for the ASan session:
build `-fsanitize=address`, run `SCM Available to Pick UT`, and the redzone write is named.

A SEPARATE real bug found on the way, kept for later: `Table<Derived>::CaptureImage` did
`copy->State_Block = detail::StateHandle{}` to blank the image's state, but `StateHandle::operator=`
copies nothing, so the image (xRec) wrongly retained a deep-copied state with a temp handle. The fix
is to update the image record IN PLACE (a stable pointer) with a blank state -- staged in the
scratchpad; it does NOT fix this corpse (the garbage is upstream in Rec) and waits with the rest.

## comment (2026-09-12, part 5) -- ASan built and run: it is a clean UAF of the before-image, and the fix is -7

The box now carries an AddressSanitizer build (`build-asan`, `-fsanitize=address -fno-omit-frame-
pointer -O1`). Run over `SCM Available to Pick UT` it named the corpse EXACTLY, and corrects part 4:
it is NOT a raw overwrite of `Rec.State_Block`. It is a `heap-use-after-free`, READ in
`StateHandle::StateHandle(const&)` (RecordState.h:376), of the 1248-byte BEFORE-IMAGE record (the
`HeldImage`'s `record_`, a `Tracking Specification`). Two paths free+reallocate that record while a
generated body holds a reference into it:

- A generated procedure binds the before-image ONCE: `auto &XRec = Rec.StoredImage();` at the top of
  `AssignSerialNoBatch`, then loops.
- Free path 1: `Rec.Copy(...)` -> `CopyStateFrom` replaced the whole state and freed the old image
  (ASan's named free: `TestTempSpecificationExists` -> `Copy` -> `CopyStateFrom`).
- Free path 2: `Rec.Insert()/Modify()/Read()/Next()` -> `CaptureImage` did `image.Hold(new Derived)`
  and `Hold()` `Reset()`s (frees) the old record. The loop `Insert`s each pass, so the next pass's
  `OnAfterAssignNewTrackingNo(Rec, XRec, ...)` copies a freed `XRec`. (This DISPROVES part 4's aside
  that the CaptureImage realloc "does not fix this corpse" -- it is one of the two frees.)

The retry harness masked it as 0 LOST while the codeunit still SIGSEGV'd standalone (EXIT 139).

FIX TRIED (both free paths): make the image record's address STABLE for the variable's life --
`CopyStateFrom` preserves the destination's `HeldImage` by move (Copy never brings xRec:
record-copy-method.md), and `CaptureImage`/`BlankImage` go through a new `EnsureImageRecord()` that
holds one blank record and overwrites its FIELDS in place (never reallocates). This also fixes part
4's separate `State_Block = StateHandle{}` no-op (the image now starts blank and stays blank-stated).

RESULT, measured: the CRASH IS GONE and SCM is DETERMINISTIC -- 28 of 49 on three standalone runs,
0 crash markers. But run 140 (whole milestone) was 2197 of 2310 vs run 139's 2204: A/B is -7 / +0,
every one an item-tracking Pick case (`PickPositive/Negative/Partial`, `PickAndShipment{Pos,Neg,
Partial}`, `DirectedPutAwayPickSimpleScenarioNotAllowBreakbulk`) now throwing
"Qty. to Handle (Base) ... is currently 4. It must be 10." So the "4 vs 10" is a COUPLED item-
tracking quantity bug the UAF's luck was masking (a lucky read of the pre-loop image yielded 10);
stabilizing the image makes it deterministically wrong. TAKEN BACK (an undo is a result): the
image-lifetime fix is reverted from the tree, staged at `$S/recordstate_copystatefrom_fix.h.staged`
and `$S/table_captureimage_fix.h.staged`, and the ASan report at `$S/board0718_asan_report.txt`.

So the earlier "gates every codegen" framing is now precise: the MEMORY-SAFETY root (image lifetime)
IS understood and fixable runtime-only, but it must land WITH the "4 vs 10" quantity fix or it is
-7. The remaining root is the AL semantic of `xRec` during a page batch-insert loop (is it the
fresh per-`Insert` before-image, or the pre-loop snapshot?) and how the item-tracking qty is summed
from it -- under investigation. batch213/218 stay shelved until the image fix can land net-positive.

## comment (2026-09-12, part 6) -- the image-lifetime fix is CORRECT; the qty root is the next measurement

An `al-semantics` pass (documentation + AL source + both boards) settled the xRec question and
narrowed the qty root:

- **The image-lifetime fix must be KEPT, not reverted for its own sake.** `xRec` is the record
  variable's OWN before-image, refreshed after every successful `Get`/`Find`/`Next`/`Insert`/
  `Modify` THROUGH that variable (`devenv-al-variables.md`; openerp WI-1156 refresh-after-Insert,
  WI-1078 not-a-mirror, WI-1242 frozen only across a nested Validate cascade, WI-781; agiru already
  chose this in board:0042/0526). So `xRec` updating on each `Rec.Insert()` in AssignSerialNoBatch
  is the DOCUMENTED behaviour, and the stable-address fix (EnsureImageRecord + CopyStateFrom
  preserve) is a correct close of a real dangling-reference UAF that 2 482 `StoredImage()` sites and
  1 209 `.Copy(` sites in `apps/` depend on. It is reverted from the tree ONLY because it is -7
  without the qty fix; it is staged and lands WITH that fix.
- **`OnAfterAssignNewTrackingNo` has ZERO subscribers in BCApps** and its `xRec` param is by value,
  so xRec's value there is read by nothing -- not the qty cause.
- **The qty label** "Qty. to Handle (Base) ... is currently N. It must be M." is
  `TrackingSpecification.Table.al:562` (`WrongQtyForItemErr`), raised via `TestFieldError` from
  `CheckItemTrackingByType:1150-1194` off `ReservationEntry.CalcSums("Qty. to Handle (Base)")`. It is
  also hand-declared as a Label in ~15 test codeunits, so the raising stack must be read live.
- **openerp is no reference here**: its page `xrec` is a permanent blank stand-in (page.py:342-368),
  and the identical qty cluster is openerp board 452/456, status OPEN, 49 tests, never root-caused.
- **DECISIVE NEXT (this box now has ASan):** rebuild `build-asan` WITH the image-lifetime fix applied
  and run `SCM Available to Pick UT` twice. My first ASan run halted at the first error (the before-
  image UAF); with that closed, ASan either (a) reports NOTHING more -> "4 vs 10" is a functional
  quantity bug in the CheckItemTrackingByType/CalcSums path, debuggable normally, or (b) reports a
  SECOND write landing garbage at offset 0 of the page `Rec` -> part 4's "upstream overwrite" is
  real and is the qty root. Either way the answer is one ASan run, not more source-reading.
- A gate defending the image fix (write it when the fix lands): bind `auto &XRec = Rec.StoredImage()`,
  loop `Validate`+`Insert` through the same variable with an intervening `Rec.Copy(other)`, assert
  `XRec` stays valid and shows iteration k-1's values after iteration k's `Insert`. A "4 vs 10" gate
  would be BLIND while the codeunit is non-deterministic.
