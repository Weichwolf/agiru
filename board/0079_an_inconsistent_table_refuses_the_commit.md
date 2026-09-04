Type: root
State: open
Area: rt, db
Tags: navision, semantics, blocker

# A table marked inconsistent refuses the commit, and the posting routine is what marks it

CLAUDE.md's first invariant is that a posting is all or nothing. **AL has a primitive for exactly
that, and it is a door refusal**: `Record.Consistent(Boolean)`.

`record-consistent-method.md`:

> Marks a table as being consistent or inconsistent. ... Usually this method is only used for
> accounting routines. If your accounts do not balance, then the accounts are inconsistent. This
> method makes sure that inconsistent changes are not made in your accounts.
>
> **If an attempt is made to commit a write transaction when a table is marked as inconsistent, then
> a message is displayed and all updates that were made in the write transaction are ended.**
>
> A typical example of an inconsistency occurs when the sum of all the entries in a table that
> contains general ledger entries does not balance -- is not equal to zero.

`include/runtime/Table.h` declares it and throws.

## The BaseApp uses it where it matters, measured 2026-09-04

Nine files under `Layers/W1`, and the first of them is the one this tree cannot be wrong about:

| call site | what it does |
|---|---|
| `Finance/GeneralLedger/Posting/GenJnlPostLine.Codeunit.al:2056` | `GlobalGLEntry.Consistent(IsTransactionConsistent);` -- **the G/L posting routine hands the platform its own balance check** |
| `Finance/VAT/Ledger/DateCompressVATEntries.Report.al:436` | `GLEntry.Consistent(GLEntry.Amount = 0);` |
| `Finance/GeneralLedger/Preview/PostingPreviewEventHandler.Codeunit.al:282` | `SalesInvoiceHeader.Consistent(false);` -- the posting PREVIEW makes its transaction uncommittable on purpose |
| `Foundation/NoSeries/SequenceNoMgt.Codeunit.al:300, 314` | the same trick, with the reason in a comment: `// make sure we cannot commit the transaction` |

**So the mechanism has two uses and both are load-bearing.** The first is the balance check: the
posting routine computes whether debits equal credits and tells the PLATFORM, which then refuses the
commit. The second is a deliberate poison pill -- a preview or a dry run marks the table
inconsistent so that nothing it did can reach the disk, whatever the code above it does.

**A runtime where `Consistent(false)` refuses loudly is safe. A runtime where it did nothing would
COMMIT A POSTING PREVIEW**, which is a wrong ledger written by a feature whose whole purpose is not
to write one. Today it raises, which is the right failure -- and it means the posting preview and
`GenJnlPostLine` cannot run at all, so this is a phase-3 blocker rather than a live defect.

## What the references say beyond the page

The AL source shows the shape the runtime owes: the flag is set on a RECORD VARIABLE of the table,
it is remembered per TABLE for the transaction, and it is read at the COMMIT point. The comment in
`SequenceNoMgt` -- "make sure we cannot commit the transaction" -- says the effect is on the
transaction and not on the record.

`~/Git/openerp` has nothing here: grep finds no `consistent` in its runtime, which is consistent with
a UT subset that does not post a G/L entry. **Its silence is not evidence that this is small.**

## The choice

- **The flag lives in the TRANSACTION, keyed by table**, beside the isolation state board:0012 needs
  per table. `Consistent(false)` sets it, `Consistent(true)` clears it, and it dies with the
  transaction.
- **`Commit()` checks it before it commits**, and refuses -- with BC's own wording, which board:0055
  owns -- rather than committing and reporting afterwards. "All updates made in the write transaction
  are ended" is a rollback, so the refusal rolls back rather than leaving the caller a choice.
- **It is checked at EVERY commit point**, which is `Commit()`, the implicit commit at the end of AL
  execution, and the implicit commit `Codeunit.Run` performs when its answer is used (board:0077).
  One check in `Boundaries::Commit` reaches all three, which is why this is small.
- **It is a PLATFORM mechanism and names no AL object.** The runtime never knows what a G/L entry is;
  it knows a table was marked and a commit was attempted.

## Gate

A table marked inconsistent: a `Commit()` refuses and the rows written in that transaction are gone.
Marked consistent again before the commit: the rows are there. The mark does not survive into the
next transaction. A second table left unmarked does not block the commit.

**Negative control**: make `Consistent(false)` a no-op and require the first case to go red -- and
assert the ROWS, not the return value, because a commit that reports failure and writes anyway is
the exact failure this exists to prevent.
