Type:     task
Status:   open
Parent:   0064
Area:     gen, rt, db
Source:   developer/properties/devenv-reversesign-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `ReverseSign` flips a query column's sign

> Changes negative values into positive values and positive into negative **in a column of a
> resulting query data set.** Applies to: **Query Column.** **The default is false.**
>
> **NOTE: The property applies only to columns that have a NUMERIC data type. If you set it on a
> column that does not, you will get an ERROR.**
>
> Credits are typically stored as negative values to deduct them from the overall balance due.
> However, for displaying purposes, such as in spreadsheets and charts, you might want these
> quantities to appear as positive numbers.

**An accounting sign convention rendered as a property.** The stored value is unchanged; the query's
output is negated -- so this is a `SELECT -col` and never an update, and an implementation that
negated on write would corrupt a ledger.

The numeric-only rule is a `static_assert`: the column's type is a declaration.

**And it must not be a Decimal round trip through a `double`.** CLAUDE.md's second invariant is that
no binary floating-point type carries an amount, and negation is the cheapest possible place to break
it -- `agiru::Decimal` negates exactly, and the scale is part of the value.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ReverseSign =`: **11 declarations**, all necessarily `true` since `false` is the default.

Eleven, in a BaseApp with 8 761 `CalcFormula`s -- so the aggregate is where signs are handled and this
is the display exception.

## The IST-state

Queries have no generator (board:0064, board:0034).

## The choice

One bit on the query column, emitted into the `SELECT` list as a negation -- so PostgreSQL does the
arithmetic on `numeric` and no value passes through the runtime to be flipped.

**Not a post-read transformation.** A query streams (board:0045), and negating in the read loop is one
more operation per row for something the database does for free.

## Ordering

Inside board:0064's query generator, with the column list.

## Gate, and its negative control

A query column declaring the property returns the negation of the stored value, and the stored value
is unchanged.

**The negative control is the stored value** -- an implementation that negates on write passes every
read-side gate and inverts the ledger.
