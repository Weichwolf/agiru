Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/pagefield/devenv-onafterlookup-pagefield-trigger.md, developer/triggers-auto/pagefieldextension/devenv-onafterlookup-pagefieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page field's `OnAfterLookup` receives the selected record as a `RecordRef`

```al
trigger OnAfterLookup(Selected: RecordRef)
```

"Runs after a lookup is activated passing the selected record as a RecordRef." Unlike `OnLookup`
(0293) it does not REPLACE the lookup -- it observes the result, and it gets the whole record rather
than a text, which is how a page copies several fields from one selection.

**It runs after the DEFAULT lookup too**, which is what makes it different from `OnLookup`: a field
with a `TableRelation` and no `OnLookup` still reaches this trigger.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterLookup(` on a page field or extension: **94 declarations** -- small, because copying
several fields from a lookup is a specialised need.

## The IST-state

No page runtime, and `RecordRef` is largely absent: board:0059 records that `FieldRef` declares one
`TestField()` against 36 documented pages.

## The choice

The call sits after whichever lookup ran -- the page trigger, the table trigger or the default --
with a `RecordRef` over the selected row.

**It needs a working `RecordRef` more than it needs a page**, which puts it behind board:0059 rather
than only behind board:0030: handing a subscriber a `RecordRef` whose `FieldRef` cannot read a field
gives it nothing.

## Ordering

Blocked on board:0030 and board:0059.

## Gate, and its negative control

A field with a `TableRelation` and no `OnLookup`, plus an `OnAfterLookup` that reads two fields off
the `Selected` record: both arrive.

**The negative control is the default lookup** -- a runtime that calls the trigger only after a
custom `OnLookup` misses every field that relies on the relation, which is most of them.
