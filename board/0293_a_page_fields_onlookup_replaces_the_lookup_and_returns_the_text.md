Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/pagefield/devenv-onlookup-pagefield-trigger.md, developer/triggers-auto/pagefieldextension/devenv-onlookup-pagefieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page field's `OnLookup` replaces the lookup and hands the chosen text back through `var`

```al
trigger OnLookup(var Text: Text): Ok
```

"Runs **in place of** the normal lookup features for the current page." Two things follow from the
signature: the chosen value comes back through `var Text`, and the Boolean says whether the user
picked anything -- `false` means cancelled and the field keeps its value.

It is the page-level counterpart of the TABLE field's `OnLookup` (board:0233), and the two can both
exist: the page's wins.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnLookup(` on a page field or extension: **1 355 declarations**, against 1 197 on table
fields -- so the two mechanisms are used about equally, and a runtime that implements only one gets
half the lookups.

## The IST-state

No page runtime. `TestField::Lookup` exists in `src/rt/TestPage.cpp` and reaches `Unopened()`.

## The choice

`TestField::Lookup` and the page's own lookup path consult the page trigger first and fall back to
the table field's (board:0233), then to the `TableRelation` default.

**Three levels, most specific first** -- the same resolution shape board:0063 found for report
layouts and board:0066 for `AutoFormat`.

## Ordering

Blocked on board:0030, and paired with board:0233 so the fallback chain is built once.

## Gate, and its negative control

A page field with an `OnLookup` that writes a value and returns `true`: the field holds it. One
returning `false`: the field is unchanged.

**The negative control is the `false` case** -- a runtime that assigns whatever `var Text` holds
regardless of the return writes an empty string on every cancelled lookup.
