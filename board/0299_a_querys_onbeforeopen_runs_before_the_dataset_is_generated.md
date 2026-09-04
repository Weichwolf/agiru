Type:     task
Status:   open
Parent:   0064
Area:     rt, gen
Source:   developer/triggers-auto/query/devenv-onbeforeopen-query-trigger.md
Verdict:  fehlt
Class:    activation

# A query's `OnBeforeOpen` runs before the dataset is generated, and it is the only query trigger

```al
trigger OnBeforeOpen()
```

"Runs before a query is run and the dataset is generated." It is where a query sets the filters it
was not given -- `SetFilter` and `SetRange` on its columns and filter rows (board:0064) -- and it is
the ONLY trigger a query object has.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeOpen()`: **35 declarations**, against 457 `.Query.al` files -- so seven queries in
eight have no trigger at all and are pure declarations.

## The IST-state

Query has no generator: board:0034's object-kind table lists it among the kinds with none, and
board:0064 is the item. So neither the object nor its trigger exists.

## The choice

The generated query class carries the trigger as a member and the runtime calls it in `Open()`,
before the SQL is built -- because the filters it sets have to reach the `WHERE` clause.

**Before the SQL is built, not before it runs.** board:0064 records that a `DataItemTableFilter`
cannot be overwritten from AL while a `ColumnFilter` can; this trigger is AL, so what it sets belongs
in the overwritable layer and the emitted `WHERE` has to be assembled after it.

## Ordering

Blocked on board:0064, which owns the query generator. It is the smallest piece of that item and the
last one -- a query with no trigger works without it.

## Gate, and its negative control

A query whose `OnBeforeOpen` sets a range: the dataset honours it.

**The negative control is a `DataItemTableFilter` on the same field** -- the trigger must NOT be able
to widen it, which is board:0064's precedence rule and the one a single filter layer collapses.
