Type:     root
Status:   open
Area:     rt, cli
Source:   the first `agiru run-tests` that got past its own start, 2026-09-07
Class:    activation

# An unlinked AL procedure raises instead of killing the process

**THE RUNNER STARTS AND LISTS 855 CODEUNITS, AND THE FIRST RUN DIES ON THE THIRD KIND OF SYMBOL.**

```
./build/agiru: symbol lookup error: libagiru_slice.so:
undefined symbol: agiru::BackupManagement_Codeunit::DeleteAll()
```

The data symbols and the vtables are closed -- `ldd -r build/agiru` names 0 of each. What is left is
**2 050 undefined FUNCTIONS**, which the loader binds LAZILY: they cost nothing until a body calls
one. board:0601 wrote down the assumption that this would be survivable -- "a test that walks into
one gets a loader error, which is loud and which the run counts as a failure rather than a crash".

**THAT ASSUMPTION IS WRONG AND THIS ITEM EXISTS TO CORRECT IT.** A lazy binding failure is not a C++
exception; the dynamic loader writes to stderr and calls `_exit`. Nothing catches it, nothing counts
it, and the run ends at the FIRST test that reaches an unlinked procedure -- so 2 291 of 2 291 is
gated behind linking the whole tree rather than behind the tests themselves.

## The population, measured 2026-09-07

| | |
|---|---|
| undefined function symbols | 2 050 |
| classes owning them | 254 |
| their sources not yet in the slice | 255 |
| of those, sources that COMPILE today | **25** |

The 25 are in. The remaining 230 are the compile-fix loop's own backlog, and each is a generic gap
in `src/` -- but the runner should not have to wait for all 230 to answer for one test.

The heaviest owners say what kind of work is behind them: `ItemJnlPostLine` 104 symbols,
`CryptographyManagementImpl` 57, `CarryOutAction` 52, `EmailMessageImpl` 49, `TypeHelper` 46,
`SalesPriceCalcMgt` 45.

## What is wanted

**A CALL TO AN AL PROCEDURE THAT IS NOT LINKED MUST RAISE, the way every other refusal in this tree
raises**, so `agiru run-tests` counts that test as failed and runs the next one. Then the milestone
is measured against what the RUNTIME does, and the slice grows because a test needs it rather than
because a symbol list does.

The shape, and it is a build step rather than a runtime one:

1. after the slice links, read its undefined symbols (`nm -u`, or `ldd -r`) and keep the ones in
   `agiru::`;
2. emit a translation unit defining each of them, as a weak symbol whose body raises an `Error`
   naming the DEMANGLED symbol;
3. link that beside the slice, with the executable exporting its dynamic symbols so the shared
   object's lazy lookups find them.

**A body that never returns needs no signature**, which is what makes step 2 possible at all: the
stub throws, so no return value is ever produced and no calling convention has to be reproduced.
That is the one trick in it, and it is worth writing down because it is what turns 2 050 mangled
names into a mechanical emission.

**IT MUST NOT BE SILENT.** The count of stubbed symbols is printed by the build and belongs beside
the slice's own count in `build/times.log`: a runtime that answers every unknown call with a
refusal is one bad step from a runtime that answers every call with one.

## STANDING: DONE, and the run reports 376 tests (2026-09-08)

`scripts/unlinked.py` reads the slice's undefined symbols, subtracts what `agiru_rt`, `agiru_net`
and `agiru_db` define, and writes **1 926 definitions** that raise, naming the procedure. CMake
generates it after the slice links and compiles it into `agiru`, which now carries `ENABLE_EXPORTS`
so the shared object's lazy lookups find them.

**AND THE RUN PRINTED NOTHING UNTIL IT WAS OVER, which is why the first crash looked like a crash
at the start.** `RunRegisteredTests` collected every result and the CLI printed afterwards, so a
hard failure took the whole report with it: 0 lines for a run that had done most of its work. It
reports each procedure as it finishes now, flushed.

**376 tests report, and the failures are a ranked backlog rather than one wall:**

| count | shape |
|---|---|
| 130 | a door builtin declared and not implemented (board:0035) |
| 48 | `RecordRef.GetTable(Any)` needs a Variant that can hold a record |
| 38 | `System.Power(Decimal, Decimal)` |
| 27 | "The General Ledger Setup does not exist" -- setup DATA, not a gap in the runtime |
| 21 | the .NET member `TableMetadata.SetFilter` |
| 18 | the .NET member `Encoding.UTF8` |
| 15 | `System.Randomize(Integer)` |
| 8 | `System.Abs(Decimal)` |
| 6 | `System.DMY2Date(Integer, Integer, Integer)` |

**WHAT IS LEFT ON THIS ITEM IS THE OTHER HARD FAILURE.** The run still ends in a SEGMENTATION FAULT
after `Test Data Exch.Import - XML / InsertFieldRecWithLongXMLNodeValue`, and a segfault is no more
catchable than the loader error was. The same argument applies one level up: a run over 855
codeunits cannot be gated on every one of them being memory-safe, so the runner needs to survive a
crashing test -- which means running each codeunit in a CHILD PROCESS and reporting the child's
death as that codeunit's failure. That is the half that is standing.

## What proves it

`agiru run-tests` reaches the END of its run -- every codeunit attempted, a count printed -- with
the tests that walk into an unlinked procedure counted as FAILED and named by the symbol they
reached. The negative control is the symbol list itself: with the stub object left out, the same run
dies at `BackupManagement_Codeunit::DeleteAll` again.
