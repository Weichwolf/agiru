Type:     task
Status:   open
Parent:   0082
Area:     rt, db
Source:   the refusal that `DateFormula::FromText` gained, 2026-09-08
Class:    silent-wrong-data

# A `DateFormula` column holds BC's PACKED form, and not its text

**MAKING THE GRAMMAR REFUSE FOUND IT IN ONE RUN: 293 failures with one shape.**

```
the column of Default Safety Lead Time holds 1, and a date formula is a sign, a number and a
unit, and this character is none of them
```

`1` is not a date formula, and the column holds it because **SQL SERVER STORES A `DateFormula` IN
BC'S OWN PACKED FORM** rather than as the text a user types. The predecessor filed exactly this and
never closed it: `~/Git/openerp/board/1077_dateformula-bcs-gepackte-sql-form-wird-nicht-entpackt.md`.

**THE SEEDED CRONUS ROWS CARRY THAT FORM**, so every DateFormula field that came from the demo data
reads back as something the grammar refuses -- and read as TEXT it silently produced an empty
formula, which is a date calculation that moves nothing and says nothing (board:0082's own class).

## What is standing

**THE GRAMMAR REFUSES AND THE COLUMN DOES NOT, and the split is deliberate.** `FromText` answers
`std::expected<DateFormula, Refusal>` -- so AL's `Evaluate` answers `false` with a reason, and
`CalcDate` on a text that is not a formula is an error naming it. But `SetFieldText` falls back to
an EMPTY formula for a column it cannot read, because the alternative is 293 failures on data this
tree did not write.

**THAT FALLBACK IS THE HALF THAT IS WRONG, and it is this item.** Measured: taking the refusal all
the way to the column cost 325 UT passes -> 317.

## What has to happen

The packed form has to be decoded -- and ENCODED, because a formula this runtime writes has to be
readable by the same reader. What it is has to be established from the data rather than guessed:
`cronus` holds 1 864 tables with the 28.4 schema, and every `DateFormula` column in it is a sample
with a known display text beside it in the BaseApp's own demo setup.

## What proves it

`Default Safety Lead Time` reads back as the formula the demo data means, and a formula written by
`Evaluate` and read back through the column is the same formula. The negative control is the text
form: a column holding `<1D>` must still read, because a runtime that only understands the packed
form cannot read what it wrote before this item.
