Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-quickentry-property.md
Verdict:  fehlt
Class:    activation

# `QuickEntry` defines the path the Enter key takes

> Specifies if the page control should have input focus. **The default is true.** To specify that a
> control can be skipped, change this value to false. **Specifying an expression as the value of the
> property is not supported.**
>
> The property is respected when users select **Enter**. **This behavior differs from using Tab,
> which will sequentially give input focus to all page controls.**
>
> Applies to: **Page Field.**

**Two different traversals over one control list**, and that is the whole item: Tab visits every
control, Enter visits only the quick-entry ones. A renderer that produced one tab order would get one
of the two right.

The "expression is not supported" line is a `static_assert`: a non-literal value is a translation
error, and the page says so.

The note about a Boolean variable applies to the old Windows client and is explicitly "not supported
in the Web client", so it is a documented non-target.

**And the user may change it**: personalisation moves a control in or out of quick entry, so the
declared value is a default and not the effective one -- which means the two traversals are computed
from per-user state layered over the declaration.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`QuickEntry =`: **2 957 declarations**, all necessarily `false` since `true` is the default.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no control list, no tab order and no
personalisation.

## The choice

One bit per control, and the Enter traversal is a filtered walk of the same control list the Tab
traversal walks -- not a second list, which would drift.

Personalisation is a separate mechanism with no board item; it is named here because it makes the
declared bit a default rather than the answer.

## Ordering

With board:0030's control metadata and its keyboard handling.

## Gate, and its negative control

Enter skips a control declaring `QuickEntry = false`; Tab reaches it.

**The negative control is the Tab traversal** -- an implementation that removes the control from the
list entirely satisfies the Enter half and makes 2 957 controls unreachable by keyboard.
