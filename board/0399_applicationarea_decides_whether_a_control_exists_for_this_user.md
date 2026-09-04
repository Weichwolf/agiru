Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-applicationarea-property.md
Verdict:  fehlt
Class:    activation

# `ApplicationArea` decides whether a control exists for this user

> Sets the application areas that apply to the control. **Standard values are All, Basic, Suite and
> Advanced.**
>
> A text string that contains a **comma-separated list** of application area tags. An application
> area tag must have the format *name*, where *name* can be any combination of letters and numbers
> without spaces. **If the control applies to all application areas, you can set the property to
> `All`. This means that the control will always appear in the user interface.**
>
> Applies to: Page Label, Page Field, Page Part, Page System Part, Page Chart Part, Page Action, Page
> Custom Action, Page File Upload Action, Page User Control, Page, **Report**.

**The tag set is open**: `All`, `Basic`, `Suite` and `Advanced` are "standard values", not the values.
An app may declare `FixedAssets` and there is no enumeration to close. So this is a `string_view`
list and not an enum, which is unusual in this sweep and follows from the documentation rather than
from convenience.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ApplicationArea =`: **186 502 declarations.**

**The third-largest population in the sweep**, behind `Caption`'s 288 491 and `ToolTip`'s 159 993 --
and ahead of `ToolTip` in fact. Essentially every control in the BaseApp declares one, because AL's
own analyzer requires it.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone. Nothing carries an application area, so every
control would show for every user.

## The choice

A `constexpr` span of `string_view` per control, **split at translation time**, and the session
carries the set of enabled areas. The check is a set intersection per control at render time -- which
at 186 502 controls has to be cheap, so the tags are interned to small integers by the generator and
the comparison is over integers rather than strings.

**`All` is not a tag but a wildcard**, and collapsing it into the tag list would make it a tag nobody
enables.

**Not the alternative** -- a run-time string split of a comma-separated list per control per render.
186 502 declarations is exactly the size at which that becomes the page's cost.

## Ordering

With board:0030's control metadata. The session's enabled-area set is an experience-setup question
that has no board item yet and is named here.

## Gate, and its negative control

A control tagged `Basic` renders for a session with `Basic` enabled and not for one without; a control
tagged `All` renders for both.

**The negative control is the `All` control in a session with NO areas enabled** -- it must still
render, and an implementation that treats `All` as an ordinary tag hides everything.
