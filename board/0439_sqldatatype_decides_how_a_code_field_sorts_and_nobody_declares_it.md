Type:     task
Status:   open
Parent:   0080
Area:     gen, db
Source:   developer/properties/devenv-sqldatatype-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `SqlDataType` decides how a Code field sorts, and nobody declares it

> Sets the data type that you want to allow in a **code field**. Applies to: **Table field.**
>
> `Varchar` (**default**), `Integer`, `BigInteger`, `Variant`.
>
> Leaving the value undefined means you accept the default, which is `Varchar`.
>
> **NOTE: The `Variant` option is represented by the `SQL_VARIANT` SQL data type introduced in SQL
> Server 2000** and not supported by SQL Server 7.0.

**board:0080 is already this item's root** -- "a Code field sorts the way its `SqlDataType` says" --
and it exists because a Code field holding `'10'` and `'9'` sorts differently as text and as a number.
That is not cosmetic in an ERP: document numbers are Code fields.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SqlDataType =`: **0 declarations.**

**Not once in 2.56 million lines.** So every Code field in the BaseApp is `Varchar`, and board:0080's
question has one answer in practice: text ordering, always.

That is a useful negative result and it does not close board:0080 -- what board:0080 owns is that
`Varchar` ordering must match SQL Server's collation, which is a separate question from which type is
declared.

## The IST-state

Not among the nine properties the generator consumes (board:0067). `src/rt/Storage.cpp:85` maps AL
field types onto SQL types with no per-field override, so every Code field is one column type.

## The choice

**Refuse it**, on the zero -- the sweep's standing arithmetic. And record the negative result where
board:0080 can use it: **no Code field in the BaseApp declares a non-`Varchar` SQL type**, so a
correct `Varchar` ordering is sufficient and no numeric-ordering path is needed.

`SQL_VARIANT` has no PostgreSQL equivalent, so `Variant` would be a divergence to name if it were ever
declared -- which the refusal makes visible.

## Ordering

With board:0067's census. It removes a question from board:0080 rather than adding work.

## Gate, and its negative control

A field declaring `SqlDataType` fails to transpile.

**The negative control is the whole BaseApp transpiling with the refusal in place** -- which is what
proves the zero, and this item's entire value is that zero.
