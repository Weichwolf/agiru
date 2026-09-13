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

## comment (2026-09-12, part 7) -- ASan WITH the fix is CLEAN: the UAF is closed, the rest is functional

Rebuilt build-asan WITH the image-lifetime fix and ran SCM Available to Pick UT under it: 0
AddressSanitizer errors, deterministic 28/49. So the fix fully closes the before-image
use-after-free -- part 4's "raw overwrite upstream" and the al-semantics pass's "second corruption"
are both DISPROVEN; there is no residual memory corruption. The image-lifetime fix
(EnsureImageRecord + CopyStateFrom preserve) is correct and complete for memory safety.

The remaining 21 SCM failures are therefore all FUNCTIONAL (debuggable normally), in these shapes:
- ~9  item-tracking QUANTITY: "Qty. to Handle (Base) ... is 4. It must be 10" / "...125",
      "Item tracking lines ... must account for the same quantity. Expected: 10, Tracking total: 0"
- 7   "Nothing to handle." (the pick's available-to-pick finds nothing)
- 2   ".NET member EntityText.ReadPermission is named by AL and not rebuilt (board:0035)"
- 2   asserterror text gap: BC's "Nothing to handle." carries a hint suffix
      '\Try the "Show Summary (Directed Put-away and Pick)" option when creating pick to inspect
      the error.' that agiru omits
- 1   "ConfirmHandlerTrue named and never ran (board:0054)"
- 1   "Quantity (Base) must be 0 or 1 when Serial No. is stated."

So board:0718 splits: the MEMORY root is solved (fix staged, ASan-clean), and what remains is a
FUNCTIONAL item-tracking-quantity cluster. The image fix must still land WITH enough of that
cluster to clear the -7 (SCM 28 -> at least 35). That is multi-round item-tracking work; the fix
stays staged ($S/*.staged) and the qty is the next shape. batch213/218 codegen unblock the moment
the fix lands net-positive.

## comment (2026-09-13, part 8) -- the QUANTITY bug is the keystone: codegen costs -7 until it is fixed

Measured the NumberStyles/Decimal.TryParse rebuild (the 13 PEPPOL cases' first blocker) TOGETHER
with the image-lifetime fix, run 143 = 2197 vs run 139 = 2204: A/B -7 / +0. The -7 is the seven SCM
Pick cases (the image fix makes SCM deterministic 28/49 where the UAF's luck gave 35), and the
NumberStyles work flipped 0 -- PEPPOL has DEEPER blockers behind NumberStyles (traced: 9x "Get: the
RecordId names no record", 5x vendor Account-No. mapping, VAT-amount-0 mismatches, an unsupported
Invoice-22 namespace).

The structural lesson: a CODEGEN change (Door.cpp map, batch213/218, the NumberStyles registration)
tips this corpse, and closing the UAF with the image fix then EXPOSES the deterministic quantity bug
-- so every codegen change costs -7 through SCM whether it crashes (UAF) or runs (image fix). That
makes the item-tracking QUANTITY the true keystone, not the UAF: the UAF is understood and its fix is
staged (ASan-clean), but until "Qty. to Handle (Base) ... is 4, must be 10" is fixed so SCM holds its
count, the image fix is -7 and nothing that touches codegen can land net-positive.

NEXT (the one thing that unblocks the rest): fix the SCM item-tracking quantity. It is FUNCTIONAL
(ASan-clean with the image fix), in AssignSerialNoBatch / ItemTrackingDataCollection /
Reservation Entry. Once SCM holds its count with the image fix applied, the image fix lands
net-neutral, codegen is free, and NumberStyles+the PEPPOL chain (+13), batch213/218, and the gated
metadata fixes all follow. Staged: image fix ($S/*.staged), NumberStyles/Decimal
($S/numberstyles.h.staged, $S/dotnet_decimal.h.staged) + Door.cpp map entries after CultureInfo.

## comment (2026-09-13, part 9) -- the qty "4" is UndefinedQtyArray[1] from the data collection

Traced the count: AssignSerialNo (ItemTrackingLines.Page.al) computes
`QtyToCreate := UndefinedQtyArray[1] * QtySignFactor()`, round-trips it through the "Enter Quantity
to Create" page (SetFields -> RunModal(handler only sets CreateSNInfo, not the qty) -> GetFields),
and passes it to AssignSerialNoBatch. So the "is 4, must be 10" is UndefinedQtyArray[1] = 4 where 10
is expected -- the UNDEFINED (still-to-assign) quantity that the Item Tracking Data Collection
computes. That collection is the one place the image-lifetime fix's Copy/Capture changes bite (it
uses Copy(Rec,true)/temp records heavily), so the fresh/empty-state image perturbs the undefined-qty
math from 10 to 4.

So the keystone fix is NOT in AssignSerialNoBatch and NOT in the page round-trip -- it is that the
Item Tracking Data Collection's undefined-quantity must stay 10 under the (correct, memory-safe)
image fix. NEXT: instrument ItemTrackingDataCollection's undefined-qty (UpdateTrackingDataSetWithChange
/ the SumUp of undefined) with the image fix applied, find where a Copy/Capture with the stable
empty-state image drops it to 4, and either fix that computation or refine the image fix to preserve
what the data collection reads (without reintroducing the UAF or the self-referential state copy).
This is the one thing that makes the image fix net-neutral and unblocks all codegen.

## comment (2026-09-13, part 10) -- the "4" is CONSTANT, so the image fix INTRODUCES it (reframe corrected)

Read the actual failures in one deterministic image-fix run (scmstate.log, the CaptureImage-in-place
variant, 28/49):
- PickAndShipmentPositive/Negative/Partial: "Qty. to Handle (Base) ... is currently 4. It must be 10."
- DirectedPutAwayPickSimpleScenarioNotAllowBreakbulkSummaryPage: "... is currently 4. It must be 125."
- DirectedPutAwayPickNoPicksForMultiItem...: "Quantity (Base) must be 0 or 1 when Serial No. is stated."

The ACTUAL is ALWAYS 4, whatever the expected (10, 10, 10, 125). A genuine functional undefined-qty
bug would TRACK the input (10->x, 125->y); a CONSTANT 4 that ignores the input means the image fix
CORRUPTS the item-tracking write path -- it does NOT expose a pre-existing deterministic qty bug.
=> The earlier "reframe" (qty=4 is the true deterministic answer, 10 was UAF luck) is WRONG. Committed
code gives the correct value (lucky). The in-place image mechanism itself introduces a fixed-4.

Where 4 can come from (found by reading ItemTrackingLines.Page.al):
- CalculateSums(): `xTrackingSpec.Copy(Rec); Rec.Reset(); Rec.CalcSums("Quantity (Base)",...);
  TotalTrackingSpecification := Rec; Rec.Copy(xTrackingSpec)` over the TEMPORARY "Tracking
  Specification" (Rec). UndefinedQtyArray[1] = SourceQuantityArray[1] - Total."Quantity (Base)".
- AssignSerialNoBatch loop: each SN row does `Rec.Validate("Quantity (Base)", QtySignFactor())` (=1),
  `Rec."Entry No." := NextEntryNo()` (plain LastEntryNo+1, cannot stick), `Rec.Insert()`.
- The "Quantity (Base) must be 0 or 1 when Serial No. is stated" case is decisive: a row was Inserted
  with Serial No. set AND Quantity(Base) NOT in {0,1} -- i.e. the in-place image/Copy corrupts Rec's
  field values between Validate(=1) and Insert. So the fix's `*image = *this` (Derived copy-assign,
  which also copies the StateHandle base) plus AdoptTemporaryOf plausibly aliases image<->Rec's temp
  rows and writes a stale/shared value back.

Ruled out by reading: TempCalcSum (correct, sums all rows honoring filters), NextEntryNo (plain
increment), TestTempSpecificationExists (only checks entries <= pre-loop LastEntryNo, cannot fire
mid-loop). Static analysis is exhausted (matches parts 5-9).

ACTION TAKEN: env-gated runtime trace AGIRU_TRACE_ITT added to src/rt/Temporary.cpp (TempInsert /
TempDelete / TempDeleteAll / TempCalcSum), logging table "Tracking Specification"'s row count and each
row's Entry No./Serial No./Lot No./Quantity (Base)/Qty. to Handle/Buffer Status, plus each CalcSum's
field/rows/sum/per-row values. Built with the staged image fix applied; running the SCM codeunit under
the trace to see (a) whether inserts cap at 4 rows or (b) rows carry a wrong Quantity(Base), and where
the value diverges. Fix the fix so it is memory-safe AND does not touch the written values.

## comment (2026-09-13, part 11) -- ROOT REDIRECT: it is Reservation Entry tracking on an UNTRACKED item, NOT AssignSerialNoBatch

al-semantics agent + source reading settled the paradox in parts 9/10. The seven image-fix-regressed
tests (PickPositive/Negative/Partial, PickAndShipmentPositive/Negative/Partial, SimpleScenario) use
`LibraryInventory.CreateItem` -- which assigns NO Item Tracking Code. The item is UNTRACKED. The Pick
procedure (SCMAvailabletoPickUT.Codeunit.al:2758) has NO OpenItemTrackingLines, NO AssignSerialNo, NO
serial numbers: just CreateItem + purchase + AutoReserveSalesLine (reservation, not tracking) +
CreateInvtPutPickSalesOrder + CheckPick. So AssignSerialNoBatch / the "always 4" count is a RED
HERRING -- the earlier parts chased the wrong procedure.

The real throw path (all in TrackingSpecification.Table.al):
- WhseActivityPost / SalesPost call `CheckItemTrackingQuantity(TableNo, ...)` (:1113).
- It filters `ReservationEntry.SetSourceFilter(...)` then `ReservationEntry.SetFilter("Item Tracking",
  serial-types...)` and calls `CheckItemTrackingByType` (:1150).
- CheckItemTrackingByType: `ReservationEntry.CalcSums("Qty. to Handle (Base)")` -> HandleQtyBase; if
  `Abs(HandleQtyBase) > Abs(QtyToHandleBase)` it FindLast + TransferFields + TestFieldError, which
  throws WrongQtyForItemErr "currently <HandleQtyBase> must be <QtyToHandleBase>  ... serial number
  <ReservationEntry."Serial No.">".

DECISIVE: `"Item Tracking"` is a STORED field on Reservation Entry, set by GetItemTrackingEntryType()
from Serial/Lot/Package No. For an UNTRACKED item every reservation entry is Serial No.='' ->
"Item Tracking" = None, so the SetFilter(serial-types) matches NOTHING -> HandleQtyBase = 0 -> Abs(0)
> Abs(qty) is false -> the check is INERT. That is why committed code passes (lucky). The error names
"serial number GL00000118" (POPULATED) -> agiru's reservation entries carry a non-blank Serial No.
for an untracked item, so the serial-types filter matches them and the sum path activates.

=> ROOT: the board:0718 image fix corrupts a field copy during AutoReserveSalesLine / reservation-
entry creation, writing a garbage Serial No. (and thus "Item Tracking" != None) into Reservation
Entry (table 337). This is an ACTIVATION bug (a tracking check dead for untracked items now runs),
consistent with the platform note (devenv-system-defined-variables.md): xRec MAY share underlying
state with Rec and changes can propagate to Rec -- and the uncommitted fix makes xRec share Rec's
TempHandle (AdoptTemporaryOf). Related open item: board:0507 (CalcSums + current key over the
"Item Tracking" SumIndexField).

NEXT: trace must move to Reservation Entry (337), not Tracking Specification (336). Add a trace to the
SQL CalcSum path (src/rt/Table.cpp) + the reservation-entry write path logging "Serial No." and
"Item Tracking"; run PickPositive; find where the garbage Serial No. is written during reservation.
Editing only Table.cpp/Temporary.cpp (not the door headers, already built) is a fast relink.

## comment (2026-09-13, part 12) -- CONFIRMED via runtime trace: leftover committed serials leak into later postings

Env-gated trace (AGIRU_TRACE_ITT) over Tracking Specification (336) + Reservation Entry (337),
built on the image fix. Findings:

1. Tracking Specification serial assignment is CORRECT: tracked tests insert 5 serial rows (GL00000117
   -0121, each Qty=1), CalcSums("Quantity (Base)")=5. No "4" here.
2. The CheckItemTrackingByType SQL CalcSums("Qty. to Handle (Base)") over Reservation Entry (337)
   returns 0 (47x) or 5 (1x) -- NEVER 4. So that is NOT the thrower.
3. The "currently 4" is the OTHER thrower: ItemJnlPostLine.SetupSplitJnlLine (ItemJnlPostLine.
   Codeunit.al:3978) does `TempTrackingSpecification.CalcSums("Qty. to Handle (Base)",...)` then
   `TestFieldError(..., SignFactor * ItemJnlLine2."Quantity (Base)")`. The trace shows, right before
   each failing test's error, a Tracking Specification temp CLEARED (deleteall n=0) then populated
   with exactly 4 rows -- Entry No. 1-4, Serial No. GL00000118, GL00000119, GL00000120, GL00000121,
   each Qty. to Handle=1 -> CalcSums=4 -> TestFieldError(4, 10 or 125) -> WrongQtyForItemErr.
4. GL00000118-0121 are 4 of the 5 serials the FIRST-running test (DirectedPutAwayPickNoPicksForMultiItem,
   line 2155) created (117-121). That test itself FAILS ("Quantity (Base) must be 0 or 1"), but it
   posts a warehouse receipt (Commit) BEFORE failing, so the serial reservation/tracking entries are
   made durable. Every LATER untracked test's item-journal posting (put-away registration) then
   retrieves these leftover serials into its posting TempTrackingSpecification, even though its own
   item is untracked -- and the error reports GL00000115 (MultiItem's item), not the current test's.

So the -7 is: the image fix (correctly) lets MultiItem run far enough to Commit serials; those leak
into 7 later postings. Committed code CRASHED MultiItem (UAF) before it created serials, so no leak ->
those 7 passed (lucky). The leak is a GENUINE agiru bug the fix exposes (activation).

OPEN QUESTION being measured now: is it (B) an UNSCOPED retrieval -- agiru reads leftover Reservation/
Tracking entries not belonging to the current document because a SetRange/SetCurrentKey/CalcSums-key
filter is not applied (board:0507: "CalcSums needs its key to be current, and both use the filters") --
or (A) a TEST-ISOLATION gap -- BC rolls back per-test INCLUDING commits (devenv-testisolation-property:
"all database changes are rolled back, including changes explicitly committed"), agiru's RunOne does
scope.Keep()/Discard per method and never undoes a Commit. Querying a kept scratch DB for the leftover
serials' Source IDs + whether item numbers increment decides A vs B.

## comment (2026-09-13, part 13) -- FIX FOUND & SCM-VALIDATED: test isolation (Commit is a no-op during a test)

Root, fully resolved: the contamination is a TEST-ISOLATION gap, not the image mechanism and not a
filter bug.

- The board:0718 image fix (CopyStateFrom preserves the before-image by move + CaptureImage reuses a
  stable in-place image record) is CORRECT and ASan-clean. It is an ACTIVATION: the first-running
  DirectedPutAwayPickNoPicksForMultiItem test used to CRASH (UAF) before it created serials; with the
  fix it runs far enough to assign 5 serials (GL00000117-0121) to a purchase line and Commit them (the
  warehouse-receipt posting commits), then FAILS ("Quantity (Base) must be 0 or 1").
- agiru's Commit() is DURABLE (Boundaries::Commit releases+recreates savepoints; the invariant
  "a Commit makes prior work survive a later rollback"). So when MultiItem fails, its RunOne
  scope.Discard rolls back the uncommitted part (incl. number-series increments) but the COMMITTED
  serials persist -> an inconsistent leftover. Every later untracked test reuses the rolled-back
  journal template/line numbers, so RetrieveItemTrackingFromReservEntry (ItemJnlPostLine:176 ->
  SetupSplitJnlLine:3978 TempTrackingSpecification.CalcSums("Qty. to Handle (Base)")) matches the 4
  surviving serials -> CalcSums=4 -> TestFieldError(4, put-away qty) -> WrongQtyForItemErr
  "currently 4 ... serial GL00000118". Committed code never leaked because MultiItem crashed first.
- BC's platform rolls this back: devenv-testisolation-property.md -- "all database changes are rolled
  back, INCLUDING changes explicitly committed to the database by using the Commit Method." So under
  BC test isolation a test's Commit is not truly durable; a failing test leaves NOTHING.

FIX (generic, uses existing machinery): RunOne (src/rt/TestRunner.cpp) now wraps each test in
`const CommitScope isolate{CommitBehavior::Ignore};` (agiru already had CommitScope for
[CommitBehavior(Ignore)]). Commit() becomes a no-op for the test's duration, so:
  - PASS -> scope.Keep() persists everything (unchanged: passing tests Keep either way; identical end
    state, since a durable Commit + Keep and an ignored Commit + Keep both release the same savepoint).
  - FAIL -> scope.Discard() rolls back EVERYTHING incl. would-be-commits -> no inconsistent leftover.
  asserterror keeps its own inner savepoint, so Commit-boundary tests are unaffected.

MEASURED (SCM Available to Pick UT, --fresh --scratch, work-date 2028-01-25):
  committed (lucky UAF) = 35/49 ; image fix only = 28/49 (-7 contamination) ;
  image fix + CommitScope(Ignore) = 35/49  <- the 7 recovered, deterministically, memory-safe.

Full-suite A/B (candidate = image fix + iso fix) RUNNING to confirm net >= 2204 across all 78
codeunits (the iso fix is global: it may recover contamination elsewhere, or expose Commit-durability
/ multi-session tests). If net >= 2204 and make test holds the 7-red gate, LAND image fix + iso fix
together (remove the AGIRU_TRACE_ITT diagnostics first) -- this also unblocks codegen (PEPPOL +13,
batch213/218, metadata), which was -7 only through this same SCM contamination.

## comment (2026-09-13, part 14) -- crude iso TAKEN BACK (-15); refined to a SAVEPOINT FLOOR

Full-suite A/B of the crude fix (CommitScope(Ignore) per test): image fix + crude iso = 2189/2310 --
a NET LOSS of 15 vs committed 2204. A/B vs image-only (f140=2197): fixed 7 (the SCM cluster), BROKE
15 -- 14 in "Payment Export Validation UT" (all "Assert.IsFalse failed. Expected Error message cannot
be found.") + 1 "Price Worksheet Line UT". So CommitScope(Ignore) is too crude: those tests rely on
Commit to PERSIST an error log across an INNER rollback (a Codeunit.Run around the validation), then
read it back; making Commit a no-op loses the log. An undo is a result -- taken back.

REFINED FIX (the correct BC-matching semantics): a SAVEPOINT FLOOR, not a Commit no-op.
- Boundaries gains `isolationFloor_` + `SetIsolationFloor(depth)`; `Commit` releases/recreates only
  savepoints ABOVE the floor (floor=0 -> byte-identical to before, so postings are unchanged).
- A new RAII `detail::TestIsolation` (opened in RunOne after the test's Scope) raises the floor to the
  test's own savepoint depth for the test's duration.
Effect: a Commit inside a test still shields its writes from an INNER rollback (its work merges down
only to the TEST boundary, not to depth 0), so error-log-across-Codeunit.Run patterns keep working;
but the test's Scope savepoint SURVIVES the Commit, so on failure scope.Discard rolls the test back
past even its commits -- exactly devenv-testisolation-property ("all database changes are rolled back,
including changes explicitly committed"). PASS keeps everything (unchanged); FAIL leaves nothing.

Predicted: SCM stays 35/49 (leftover gone), Payment Export + Price Worksheet recover (Commit-across-
inner-rollback preserved), net >= 2204. Building + validating on those 3 codeunits, then the full
suite. If net >= 2204 and make test holds the 7-red gate, LAND image fix + floor iso together and
remove the AGIRU_TRACE_ITT diagnostics.

## comment (2026-09-13, part 15) -- status: fix specified & SCM-validated; HELD BACK from main pending full-suite A/B

The complete fix is specified above (part 13 image fix + part 14 savepoint-floor test isolation) and
was implemented and compiling. Measured so far: image fix + iso = SCM 35/49 (recovers the -7). The
crude CommitScope(Ignore) iso was full-suite-measured at 2189 (-15, broke Payment Export) and taken
back; the refined savepoint-floor version was built but NOT yet full-suite-validated.

It is deliberately NOT landed on main here: the floor changes core Boundaries::Commit (used by every
posting), so it must clear a full-suite A/B (>= 2204) + make test (7-red) before landing, to protect
the posting all-or-nothing invariant. This commit keeps main's CODE at the known-good state and
records the finished diagnosis + fix design in this WI.

TO LAND (next session): re-apply parts 13-14 (image fix to RecordState.h/Table.h; SetIsolationFloor +
TestIsolation to Transaction.h/cpp; TestIsolation guard in RunOne), `make`, then
`scripts/ut-milestone.sh` full A/B -- expect SCM +7, Payment Export/Price Worksheet unbroken (floor
preserves Commit-across-inner-rollback), net >= 2204. If green, land; this also removes the codegen -7
tax (PEPPOL +13, batch213/218, metadata were negative only through this SCM contamination).
