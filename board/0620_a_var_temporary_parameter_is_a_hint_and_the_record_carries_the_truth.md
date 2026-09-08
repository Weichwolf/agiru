Type:     task
Status:   open
Area:     rt, gen
Source:   26 UT failures on `Record.Copy(From, true)`, 2026-09-08
Class:    silent-wrong-data

# A `var ... temporary` parameter is a HINT, and the record carries the truth

**26 UT PROCEDURES FAIL ON ONE REFUSAL:**

```
Record.Copy(From, true) shares temporary rows, and one of the two records is not temporary
```

**AND THE REFUSAL IS RIGHT ABOUT THE RULE.** `record-copy-method.md`: "If `ShareTable` is true,
then both `Record` and `FromRecord` must be temporary; otherwise an error will occur." So the
question is not whether to check, it is which of the two records lost its temporariness.

## What was measured, and what it took back

**THE PARAMETER'S `temporary` IS DROPPED BY THE GENERATOR, ON PURPOSE.** `Signature` strips
`Temporary<>` from every parameter type, so
`var TempLedgerEntryMatchingBuffer: Record "Ledger Entry Matching Buffer" temporary` becomes
`LedgerEntryMatchingBuffer_Table &`.

Making it `Temporary<T> &` for a `var` parameter -- which looks like the obvious fix and was
tried -- **does not compile, and the AL says why**:

```AL
local procedure OnAfterDemandToInvProfile(var ReservEntry: Record "Reservation Entry")
begin
    TransServLineToProfile(InventoryProfile, Item, ReservEntry, NextLineNo);   // var ... temporary
end;
```

`ServiceLineInvtProfile` hands a NON-temporary record to a `var ... temporary` parameter, and BC
ships it. **So `temporary` on a parameter is not part of the binding contract in AL** -- it is a
hint to the reader, and the truth travels with the record at run time. The strip is correct and the
change was taken back.

## What is therefore still open

Both sides of the failing `Copy` ARE declared temporary in the generated tree:

```cpp
Temporary<LedgerEntryMatchingBuffer_Table> TempLedgerEntryMatchingBuffer{};              // local
Instance<Temporary<LedgerEntryMatchingBuffer_Table>> TempCustomerLedgerEntryMatchingBuffer;  // global
TempLedgerEntryMatchingBuffer.Copy(TempCustomerLedgerEntryMatchingBuffer, true);
```

`Temporary<T>`'s constructor calls `RuntimeMakeTemporary`, so a fresh one is temporary before
anything touches it. **One of the two loses it between the declaration and the call**, and the
candidates are named rather than guessed:

- **the `Instance<>` conversion** -- what `Copy` receives is the handle converted to `const T &`,
  and a conversion that MAKES a fresh instance rather than returning the held one would hand over a
  record that is temporary but not the same one;
- **`Temporary &operator=(const T &)`** -- `T::operator=` runs `StateHandle`'s assignment, and if
  that replaces the state the temporariness goes with it;
- **the copy constructor**, which is `= default` and therefore copies whatever `StateHandle`'s copy
  does.

## What proves it

A gate case with a temporary local, a temporary global reached through an `Instance`, and a `Copy`
with `ShareTable` between them -- and the same three after an assignment and after a copy. The
negative control is a genuinely non-temporary target, which must still refuse.

## Standing (2026-09-08): the record lost its rows in `Reset`, not in any of the three candidates

None of the three candidates above was it. The temporary gate now proves FOUR share shapes -- a
global reached through an `Instance`, a temporary handed on by value, a `var` base-typed
parameter, a global made inside the `Copy`'s own argument conversion -- and every one shared.
What did not was a Reset in between: `StateHandle::Forget()` deleted the whole `RecordState`,
and the temporary table lives in it, so `TempRec.Reset()` turned the variable into a database
record. `record-reset-method.md` lists what `Reset` clears and the rows are not on it;
`detail::RuntimeReset` keeps the `TempHandle` and forgets the rest. `Forget()` itself stays as it
was, because the temporary store calls it on a ROW COPY to strip the state a stored row must not
carry -- keeping the handle there would make every row hold its own table.

The refusal now names the side (`the source is not temporary`), which is what turned four
guesses into one measurement.

