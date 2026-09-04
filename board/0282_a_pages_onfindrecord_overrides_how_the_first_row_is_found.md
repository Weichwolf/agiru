Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onfindrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-onfindrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnFindRecord` OVERRIDES how the page finds its rows

```al
trigger OnFindRecord(Which: Text): Ok
```

"**Overrides the default page behavior** and enables you to specify which record you want to display
when the page opens." `Which` carries AL's own find selectors:

| `Which` | |
|---|---|
| `-` | the first record |
| `+` | the last record |
| `=<>` | "Record defined in the Rec variable or the closest match" |

Declaring it replaces the page's read entirely -- the page no longer calls `Find` itself, it calls
this. With `OnNextRecord` (0283) it is how a page presents rows that are not a plain table read:
a buffer, a computed set, a merged list.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnFindRecord(` on a page or pageextension: **317 declarations.**

## The IST-state

No page runtime, so nothing reads rows for a page at all.

## The choice

The page's row source consults the trigger when the generated class declares it -- the same
`if constexpr (requires ...)` shape `Table.h:353` uses for the table triggers -- and falls back to
`Find(Which)` on the record otherwise.

**The `Which` strings are AL's, not the runtime's**: `-`, `+`, `=<>` are the same selectors
`Record.Find(Which)` takes (board:0056), so the trigger receives what the page would have passed to
`Find` and the page cannot tell which of the two answered.

## Ordering

Blocked on board:0030, and paired with 0283 -- a page that overrides the find and not the step
presents a first row it cannot move from.

## Gate, and its negative control

A page whose `OnFindRecord` returns rows from a temporary buffer: the list shows the buffer's rows
and never touches the source table.

**The negative control is the source table** -- a runtime that calls the trigger and then reads
normally shows both sets, and the test that only counts the buffer's rows passes.
