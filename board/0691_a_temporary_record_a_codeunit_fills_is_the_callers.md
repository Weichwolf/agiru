# 0691 A temporary record a codeunit fills is the caller's

**The finding.** 36 UT cases -- the largest shape after JSON -- fail with `There is no Payment
Application Proposal within the filter`. The AL is a pattern the BaseApp uses everywhere:

```al
CODEUNIT.RUN(CODEUNIT::"Get Bank Stmt. Line Candidates", TempPaymentApplicationProposal);
TempPaymentApplicationProposal.FindFirst();
```

The codeunit's whole job is to FILL the temporary record the caller handed it.
`codeunit-run-integer-table-method.md` writes that parameter `var Record`, so it is passed by
reference and what the callee inserts is what the caller reads.

**The cause, and it was a deliberate rule one level down.** `TakeIn_` gives the codeunit's `Rec`
the caller's record with `Copy`, and `StateHandle::CopyStateFrom` deliberately KEEPS the target's
own temporary store: "a temporary record keeps its own rows, and a database record copied from a
temporary one stays a database record". That is right for `Rec2 := Rec1` and wrong for a `var`
parameter, where there is only one variable.

**The choice.** `detail::RuntimeBorrowTemporary` points the callee's record at the caller's rows
and leaves its FILTERS alone -- the callee looks through its own view at the caller's store. It
runs only when the incoming record IS temporary, so a database record still travels as before,
and `RuntimeAdoptTemporary` (which clears the view) stays what `RecordRef.GetTable` needs.

**Classification: activation.** A path that inserted into a store nobody read now writes where the
caller looks, so the A/B is over the whole suite.

**The proof.** `test/gate/TemporaryGate.cpp` `RowsAddedByABorrowerAreTheOwnersRows`: the owner
finds what the borrower inserted, counts it once, and a plain `Copy` still shares nothing. The
negative control was run -- with the borrow neutered the insert goes to the database instead and
the case fails loudly.

**Measured.** Chain 114, A/B against chain 113.
