Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-insertallowed-property.md, developer/properties/devenv-modifyallowed-property.md, developer/properties/devenv-deleteallowed-property.md
Verdict:  fehlt
Class:    activation

# A page says which of insert, modify and delete a user may do on it

**Three pages, one item.** They are the same property three times -- same two applicable kinds (Page,
Request Page), same `bool`, same default `true`, same consumer -- and they name one thing: which of
BC's three write operations this page offers. Splitting them would be three files differing in one
verb.

> **InsertAllowed**: whether users can add records while using a page. **The default is true.**
> **ModifyAllowed**: whether users can modify records while using this page. **The default is true.**
> **DeleteAllowed**: whether users can delete records while using the page. **The default is true.**

**These are not permissions** (board:0376) and not `Editable` (board:0400). A permission says what the
USER may do anywhere; `Editable` says whether a control accepts typing; these say what this PAGE
offers. A list page with `DeleteAllowed = false` shows no Delete action to a user who has the delete
permission.

**So the page's own actions are what changes**, and the check is at the page's insert/modify/delete
paths and not at `Table<Derived>`'s -- an AL `Rec.Delete()` from the page's own code is unaffected.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`InsertAllowed =` **2 101** · `ModifyAllowed =` **1 020** · `DeleteAllowed =` **1 863**.

All necessarily `false`, since `true` is the default. The ordering is what an ERP looks like: pages
forbid inserting most often and modifying least -- a posted document is read-only, and a list is
usually openable but not extendable.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

Three bits on the page descriptor. board:0030's page insert, modify and delete paths consult them, and
the renderer omits the corresponding actions -- omits, not disables, matching `AccessByPermission`
(board:0377).

## Ordering

With board:0030's page paths. Ahead of board:0354's `AutoSplitKey`, which is an insert.

## Gate, and its negative control

A page with `DeleteAllowed = false` offers no Delete action and refuses a delete request; the same
page's own AL code can still call `Rec.Delete()`.

**The negative control is the AL call** -- an implementation that puts the check in `RuntimeDelete`
passes the UI half and breaks the page's own code, which is a posting routine on a document page.
