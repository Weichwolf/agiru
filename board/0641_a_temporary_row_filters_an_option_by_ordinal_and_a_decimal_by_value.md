# A temporary row filters an option by ordinal and a decimal by value

**Finding (2026-09-09).** `Sales-Post` refused 42 UT cases with "There is nothing to post because
the document does not contain a quantity or amount" once posting reached `CheckAndUpdate` for
the first time (ut_seeded40; before that the same cases died earlier on `Database.SessionId`).
A probe over the generated `CopyToTempLines` shows the cause: `TempSalesLine := SalesLine`
carries the source's filters across (`"Document Type" = 2, "Document No." = 1004`, which is what
AL's record assignment does), and the temporary store then evaluates `2` against the row's
rendering of the field, which is the member name `Invoice`. Nothing matches, the temp table
counts zero, `CalcInvoice` and `CheckTrackingAndWarehouseForShip` both answer false, and the
header has nothing to post. The same comparison makes `SetRange(Quantity, 3)` miss a temporary
row whose decimal renders as `3.00000000000000000000`.

**Reference.** `option-data-type.md`: an option is a zero-based enumerator and the column holds
the number; `record-setrange-method.md` and `record-setfilter-method.md` accept either the
ordinal or the member name in a filter. The SQL side already does this (`src/rt/Where.cpp` binds
`MemberOrdinal` for an option column); only the in-memory evaluator compared text.

**Predecessor.** openerp WI-1008 is the same value/caption confusion on the other side (a caption
reaching the value side of a filter).

**Choice.** `src/rt/Filter.cpp` canonicalises both sides of an Option, Enum or Boolean comparison
to the ordinal before comparing, and compares numeric fields (Integer, BigInteger, Decimal,
Option, Enum) by value for `=` and `<>` the way it already did for `<` and `>`. Wildcards stay
textual. Gate cases in `FilterGate` (pure evaluator) and `TemporaryGate` (through a temporary
`Resource Cost`). Classified silent-wrong-data.
