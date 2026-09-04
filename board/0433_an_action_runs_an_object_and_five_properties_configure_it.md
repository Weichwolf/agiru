Type:     task
Status:   open
Parent:   0030
Area:     al, gen, rt
Source:   developer/properties/devenv-runobject-property.md, developer/properties/devenv-runpagelink-property.md, developer/properties/devenv-runpageview-property.md, developer/properties/devenv-runpagemode-property.md, developer/properties/devenv-runpageonrec-property.md
Verdict:  fehlt
Class:    activation

# An action runs an object, and four properties configure it

**Five pages, one item.** `RunObject`'s own page names `RunPageView` and `RunPageLink` as its
parameters, and the other three exist only to configure it. An action with `RunPageLink` and no
`RunObject` does nothing.

> **RunObject** (Page Action): the object to run when the action is activated. **Pages, reports,
> codeunits, and from version 23.0 also queries.**
>
> **RunPageLink**: a link -- the same six-shape term grammar as `TableRelation`, `CalcFormula` and
> `SubPageLink`. **"The filters defined by this property are VISIBLE in the UI and can be MODIFIED by
> end-users."**
>
> **RunPageView**: a table view -- `SORTING`, `ORDER`, `WHERE`. **"The filters defined by this
> property are NOT visible in the UI and CANNOT be modified by end-users."**
>
> **RunPageMode**: `View`, `Edit` or `Create`.
>
> **RunPageOnRec** (default false): show the same record on the launched page as on the current one.

**The visible/hidden distinction between the two filter properties is the finding.** They apply the
same restriction and differ only in whether the user can lift it -- so an implementation that mapped
both onto one filter set would let a user widen a filter BC hides, which on a permission-adjacent
page is a data leak and not a layout bug.

And the documentation adds a performance rule that is checkable:

> **IMPORTANT: For performance reasons, always set the `RunPageView` property if `RunPageLink` is
> also set. In fact, the sort order chosen in `RunPageView` MUST CONTAIN THE FIELDS listed in
> `RunPageLink` or else the performance is decreased.**

Both properties are declarations, so **"the view's sort contains the link's fields" is decidable at
translation time** -- a `static_assert`, or at minimum a counted diagnostic. That is board:0045's
index rule appearing as a property relation.

One more, from 2025 wave 1: an action with `RunObject` and no `Caption`, `ToolTip`, `AboutText` or
`AboutTitle` **inherits them from the targeted object**. So board:0382's and board:0385's caption
resolution has a fallback that crosses objects, and the generator can fold it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`RunObject =` **33 486** · `RunPageLink =` **6 294** · `RunPageView =` **3 193** ·
`RunPageMode =` **1 232** · `RunPageOnRec =` **387**.

**`RunObject` at 33 486 is the fifth-largest population in the sweep** -- most actions in the BaseApp
run an object rather than calling code. And **6 294 links against 3 193 views** means roughly half the
links violate the documentation's own performance rule.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One `constexpr` action descriptor: `{ ObjectKind, object id, mode, on-rec, visible link terms, hidden
view }`, all resolved by the generator. The two filter sets stay SEPARATE in the descriptor -- the
separation IS the visible/hidden rule, and merging them at translation time throws the distinction
away before the renderer can honour it.

The caption fallback is folded by the generator, since both objects are known.

## Ordering

Behind board:0018's filter parser and board:0430's term parser. With board:0429, which is what makes
a page runnable.

## Gate, and its negative control

An action with `RunPageLink` opens the page with a filter the user can see and clear; the same
restriction expressed as `RunPageView` is not shown and cannot be cleared.

**The negative control is clearing the filter** -- an implementation that merges the two lets the user
clear a `RunPageView` restriction, which every "the right rows appear" gate passes.
