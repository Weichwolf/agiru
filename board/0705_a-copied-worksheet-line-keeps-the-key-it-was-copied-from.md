# 0705 A copied worksheet line keeps the key it was copied from

**The finding (2026-09-11, `Suggest Price Lines UT` T020-T022, 6 cases).**
`The Price Worksheet Line already exists. Identification fields and values: No, 1`. Traced with
`AGIRU_TRACE_SQL=1`:

- the source `Price List Line` is inserted as `("GU00000006", 1)` -- `Line No.` from `MAX+1`,
  because the library inserts it as 0 and the field is `AutoIncrement`;
- `PriceListManagement.CopyToWorksheetLine` then does `PriceWorksheetLine.TransferFields(ToPriceListLine)`
  and inserts, and BOTH worksheet rows arrive as `("", 1, ...)`: a BLANK `Price List Code`, the
  first `Line No.` from `MAX+1`, the second already 1 before its insert (no `MAX` query ran).

So `TransferFields` between the two tables did not carry the primary-key fields 1 and 2 the way
AL's default (`InitPrimaryKeyFields = true`) says it does -- or the record it was handed had them
blank. Which of the two is the next measurement: print `ToPriceListLine`'s key at the call, and the
worksheet record's key after `TransferFields`, in one probe. `Price Worksheet Line.OnInsert` fills a
blank `Price List Code` from `GetDefaultPriceListCode`, which is why the blank did not refuse.

**Reference:** `record-transferfields-method.md` -- "copies by field NUMBER", primary key included
unless told otherwise; `devenv-autoincrement-property.md` -- an explicit non-zero value is inserted
as it is.
