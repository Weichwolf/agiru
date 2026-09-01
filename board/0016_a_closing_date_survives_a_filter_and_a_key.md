Type: leaf
State: open
Area: rt, db
Tags: navision, semantics

# A closing date survives a filter, a key and a SIFT bucket, and not only a comparison

`type/Date.h` holds the closing date correctly AS A VALUE: normal(d) < closing(d) < normal(d+1),
one integer, ordering for free. That is the easy half and it is done. The half that is not done is
every place the date is not simply compared.

## What is open

**A filter range.** AL's `SETRANGE("Posting Date", 0D, 20251231D)` -- does it include the closing
date of 31 December? In NAV it does NOT, because the closing date sorts after the normal one and
the range ends at the normal one. `SETFILTER('..%1', ClosingDate(20251231D))` is how the
year-end close is actually selected, and BaseApp code is full of both spellings. Get this wrong and
a year-end close either double-counts or vanishes: the two most expensive wrong answers an ERP has.

**`CLOSINGDATE` and `NORMALDATE` inside filter expressions.** The filter language accepts them, so
the filter parser must too. `Date.Closing()` exists; the filter grammar does not.

**The date in a KEY.** `G/L Entry` is keyed on posting date. Sorting has to put the closing date
between the two normal dates, which the storage form already does -- `23:59:59` against `00:00:00`
of the next day -- but only as long as the column stays `timestamp` and the ORDER BY is on the
column and not on a `::date` cast. Nothing checks that today.

**Reading one back.** `DateFromStorageText()` maps the WHOLE of 1753-01-01 to the undefined date,
because a date column spends both of that day's instants on the 0D sentinel. That is right for a
date column and wrong for anything that reuses the function; `DateTimeFromStorageText()` already
had to be split away from it once for exactly this reason.

## The benchmark

A gate that inserts three rows -- 31 December normal, 31 December closing, 1 January normal -- and
then reads them back through a filter, in key order, and through a `SETRANGE` that ends on
31 December. The closing row must be present or absent in each exactly as NAV has it, and the
answers must be quoted from the documentation rather than from what the code happens to do.

## Closed when

The gate passes against PostgreSQL, and a `..` filter that ends on a normal date is shown to
exclude that date's closing row while one that ends on the closing date includes it.
