Type:     task
Status:   open
Parent:   0055
Area:     rt, gen
Source:   developer/properties/devenv-captionclass-property.md
Verdict:  fehlt
Class:    activation

# A `CaptionClass` is resolved by an AL codeunit, and the runtime only calls it

> Controls the caption that is used in the label of a field in a database table or in the label of a
> control on a page. Applies to: **Table field, Page Label, Page Field.**
>
> The property must be expressed in the format `'<Caption Area>, <Caption Expression>'`.
>
> **The `Caption Class` (codeunit 42) in the system application layer then translates the
> `CaptionClass` property into actual captions.**
>
> `<Caption Area>` can be `'50000'`, `'50140'` and so on, **or a code among 1, 2 and 3, which are
> handled by the base application layer and have a special meaning**: 1 is a **Dimension** as
> caption; 2 is captions of fields that can include or exclude **VAT** -- `'2,0,Invoice Amount'`
> renders **Invoice Amount Excl. VAT** and `'2,1,Invoice Amount'` renders **Invoice Amount Incl.
> VAT**; 3 renders the expression as-is.

**The resolution is AL, not platform**, and that is the item's whole shape. The runtime does not
interpret areas 1, 2 or 3 -- codeunit 42 does, and it is BaseApp code the transpiler translates like
any other. So what the runtime owes is: recognise that a field or control carries the property, and
call the resolver with the string.

**And the resolver is reached by EVENT, which board:0066 already measured.** The `Auto Format`
codeunit's `OnResolveAutoFormat` pattern is the same shape, and codeunit 42's own extension point is
where a local layer adds an area. So this is board:0057's event dispatch with a caption on the end of
it, not a new mechanism.

**A hardcoded area 1, 2 or 3 in `src/` would violate the invariant** that the runtime knows no AL
object. The areas are BaseApp semantics; the runtime carries the string.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CaptionClass =`: **9 524 declarations.**

## The IST-state

Not among the nine properties the generator consumes (board:0067). `src/rt/Builtins.cpp:69` --
`CaptionClassTranslate(Text)` refuses the door.

## The choice

The string lands on `FieldDef` and on the control as a `string_view`, unparsed -- the transpiler must
NOT split it on the comma, because the area's meaning decides how many parts there are and only the
resolver knows that.

The caption path asks the resolver when the property is present and uses the declared `Caption`
(board:0382) when it is not.

## Ordering

Behind board:0057's event dispatch and board:0034's codeunit generator, which is what makes codeunit
42 exist. The metadata half can go with board:0382.

## Gate, and its negative control

A field declaring `CaptionClass = '2,1,Invoice Amount'` renders **Invoice Amount Incl. VAT**, with
the string produced by the transpiled codeunit 42 and not by `src/`.

**The negative control is a grep**: removing every AL app from the build must leave `src/` unable to
produce that caption. If `src/` can still render it, an area was hardcoded and the invariant is
broken.
