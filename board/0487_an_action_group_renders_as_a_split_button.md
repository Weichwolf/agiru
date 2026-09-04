Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-showas-property.md
Verdict:  fehlt
Class:    activation

# An action group renders as a split button

> **Version**: runtime 10.0. Applies to: **Page Action Group.**
>
> `Standard` -- a standard group. `SplitButton` -- **"a combination of a button and a menu ... fast
> one-click access to the FIRST action, which is set to `Visible` and to `Enabled` in a menu via the
> left button part, and access to other related actions via the right dropdown part."**
>
> **"Re-ordering the actions in a split button group from a page extension or page customization can
> CHANGE the action used for the split button."**
>
> - **"Split buttons aren't supported in CONTEXT MENUS."** And with `ModernActionBar` off, not on a
>   promoted action category group.
> - **"Mobile clients don't support split buttons and the property will be IGNORED."**
> - **"Any tooltip, caption, or image property set on a group ISN'T RENDERED if the group is defined
>   as a split button. It's still a best practice to set these properties, because they're used if
>   the group is rendered as a regular group, for example on mobile clients."**

**The default action is the first VISIBLE and ENABLED one**, not the first declared -- so it is
resolved at render time from board:0401's and board:0402's bits, not by the generator. And a page
extension reordering the group silently changes which action a one-click press runs, which the
documentation flags as a hazard rather than a feature.

**And the group's own caption, tooltip and image are dead in this mode** -- board:0382's, board:0385's
and board:0416's values are carried and not rendered. That is the third declaration in this sweep that
is deliberately ignored, after board:0374's card-page caption fields and board:0422's `RowSpan`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ShowAs =`: **882 declarations**, all necessarily `SplitButton` since `Standard` is the default.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

A two-valued enumerator on the action group. The renderer emits a split button and picks the default
action by walking the group for the first visible and enabled one **at render time**, because both
bits may be AL expressions.

The context-menu restriction is a `static_assert` where the group's placement is a declaration.

## Ordering

With board:0030's action rendering, board:0401 and board:0402.

## Gate, and its negative control

A split-button group whose first action is invisible offers the SECOND action on the button.

**The negative control is the invisible first action** -- an implementation that takes the first
declared action gets the common case right and this one wrong; the group's caption not being rendered
is the second assertion.
