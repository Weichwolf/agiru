Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-scope-action-property.md
Verdict:  fehlt
Class:    activation

# A `Repeater`-scoped action appears on the row, not on the page

> Specifies whether an action applies to the page or to a repeater control. The property has the
> values `Page` and `Repeater`. **The default is `Page`.**
>
> Use `Repeater` on pages that include a repeater control. **The action then appears in the shortcut
> menu for each row in the list.**

Two placements from one declaration: an action in the page's own action bar, or an action on every
row of the list. It is how a user runs something on the line they are standing on rather than on the
document.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Scope =`: 1 131 declarations, of which **`Repeater` 1 096** and **`Page` 29**. Since `Page` is the
default, those 29 are redundant, and essentially every declaration of this property in the BaseApp is
a row action.

**1 096 makes this the largest of the three `Scope` uses by three orders of magnitude** (board:0361
measures 0 for its own, board:0363 the same), so a reader who meets `Scope` in AL is almost certainly
looking at this one.

## The IST-state

Pages carry no control or action metadata beyond `SourceTable` (`src/gen/PageWriter.cpp`).

## The choice

An enumerator on the action, defaulting to `Page`, read by board:0030's renderer to decide where the
action is emitted -- the action bar or the row's own menu. The action's `OnAction` is the same code in
both cases; only the placement and the record it runs against differ, and for a row action that record
is the row rather than the page's current one.

**That last clause is the part that is not decoration.** A repeater action runs on the row it was
invoked from, which for a list page under board:0056's cursor is not necessarily the record the page
considers current.

## Ordering

Behind board:0030's action rendering. board:0361's kind dispatch has to exist first, or the property
is read as the wrong one.

## Gate, and its negative control

A `Repeater` action appears in each row's menu and runs against that row; a `Page` action appears in
the action bar.

**The negative control is which record it ran on** -- an implementation that places the action
correctly and runs it against the page's current record passes the placement half and posts against
the wrong line.
