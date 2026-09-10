# 0694 The `Field` virtual table has rows

**The finding.** `Field` (2000000041) was registered, had a schema, and was never FILLED.
`Table Metadata` and `Page Metadata` are written at provisioning; `Field` was not. Everything that
reads the field catalogue therefore found nothing:

- `DataTypeManagement.FindFieldByName(RecRef, FieldRef, Name)` filters `Field` by table and name
  and answers false, leaving the caller's `FieldRef` **unchanged** -- so the AL that ignores its
  Boolean reads through a STALE FieldRef. That is the 29 `Tax code did not match` cases: the
  expectation came from whatever field the previous call had bound, not from `VAT Identifier`.
- 10 cases meet the refusal added in board:0689, which is the same defect saying so out loud.
- 9 say `There is no Field within the filter` outright.

**The choice.** Provision it from the catalogue beside the other two, once per database: for every
installed table, every declared field -- number, name, table name, type, length, class, relation
table, caption, `Enabled`, and `IsPartOfPrimaryKey` from the primary key. It is a virtual table in
BC and metadata here, so it is written when the database is provisioned and never again.

**Why it went unnoticed:** the failures it causes do not name it. A stale FieldRef compares two
plausible values and the message is about tax codes; only the refusal from board:0689 pointed at
the FieldRef itself, and it was added two rounds ago.

**Measured.** Chain 117, A/B against chain 116's 1 691.

**Chain 117 was an ABORT and not a measurement: `0 of 0 over 0 codeunits, 80 that printed no
total`.** The provisioning threw before any test ran, on a generated table whose field is called
`Address1_ShippingMethodCodeEnum` -- 31 characters, where BC's `Field.FieldName` column is
`Text[30]`. The names are fitted to the column, which is what BC does with that column too, and
`Storage.cpp` already carried a `Fitted` helper for exactly this on the profile rows.

**The rule is what caught it.** `0 of 0` is the shape a clean sheet has; only "a run that finds
nothing is an ABORT" tells the two apart. A provisioning step that throws takes every codeunit
with it, so the blast radius of a metadata write is the whole suite and not one case.

**Chain 118 measured the rows themselves at 1 639 -- a LOSS of 52 against chain 116's 1 691**, on
a right denominator, so it counts. 18 cases fixed, 70 lost, and 59 of the 70 said the same thing:
`the Variant does not hold Text`. Binding those FieldRefs is what let AL reach the conversion that
was refused.

**The refusal was the defect, not the binding.** `FieldRef.Value` on a Date field is a Variant
holding a Date, and the BaseApp passes it straight into a `Text` parameter --
`IncomingDocument.FindByDocumentNoAndPostingDate(Rec, DocumentNoFieldRef.Value(),
PostingDateFieldRef.Value())`, whose second parameter is `PostingDateText: Text`. That is shipping
AL and BC converts it, so the door's "it RAISES on a mismatch because AL's does" was right about
records and wrong about every scalar. A Variant handed to a `Text` now renders what it holds --
Boolean as `Yes`/`No`, Integer, Decimal WITH its scale, Date, Time, DateTime, Guid, RecordId,
DateFormula through the same `ToInvariantString`/`ToText` the field text uses -- and what has no
text form still refuses.

**So the loss is not taken back but PAID FOR in the same round**, which is what the rule is for: a
revert would have restored 52 cases and left the cause standing, and the next round would have met
it again. Chain 119 measures the pair.
