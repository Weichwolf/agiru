Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-showcaption-property.md
Verdict:  fehlt
Class:    activation

# `ShowCaption = false` renders a control without its label

> **Version**: runtime 3.2. Applies to: **Page Label, Page Field, Page Group.**
>
> Sets whether the text specified by the `Caption` property is displayed for the control. **True** if
> the caption is displayed; otherwise false. **The default is true.**

The control keeps its caption -- the tooltip still uses it, the API still names the field by it -- and
only the LABEL disappears. That distinction is the item: a renderer that dropped the caption rather
than the label would take the tooltip with it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ShowCaption =`: **8 636 declarations**, all necessarily `false` since `true` is the default.

**8 636 controls in the BaseApp render without a label**, which is a large fraction of anything on a
document page -- the totals under a subform, the fields in a cue group, a repeater's own columns
whose header is the caption.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no control metadata and no caption on
a control (board:0382).

## The choice

One bit on the control descriptor beside its caption. The renderer emits the label element or omits
it; the caption `string_view` is unaffected either way.

## Ordering

With board:0382's control captions -- the bit is meaningless before the caption exists.

## Gate, and its negative control

A control declaring `ShowCaption = false` renders no label and still reports its caption to a tooltip
and to a `FieldCaption` call.

**The negative control is `FieldCaption`** -- an implementation that clears the caption instead of
hiding the label renders correctly and returns an empty string to AL code, which is a silently wrong
error message on 8 636 controls.
