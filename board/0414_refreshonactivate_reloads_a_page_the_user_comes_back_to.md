Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-refreshonactivate-property.md
Verdict:  fehlt
Class:    activation

# `RefreshOnActivate` reloads a page the user comes back to

> Set this property on pages where you want to **refresh the data when the user navigates back from
> another page**. Applies to: **Page.**
>
> **On RoleCenters, modifying data in one part will automatically refresh data in any other parts
> which have the `RefreshOnActivate` property set to true.**

**Two triggers for one property**: returning to a page, and -- on a role centre -- another part
writing. The second is the one that matters, because a role centre's cues are FlowFields
(board:0047) and a cue that does not refresh after a posting shows yesterday's count.

**And it collides with a rule this tree already holds.** CLAUDE.md: "nothing in a process is
authoritative ... anything cached across a transaction without the rowversion is stale by design."
A page that does NOT refresh is showing cached rows; this property is BC's own admission of that and
its per-page opt-in. So the honest question this item asks is whether agiru should refresh
unconditionally -- an htmx renderer re-requests the fragment anyway -- and treat the property as a
no-op, or reproduce BC's selective staleness.

**Take the deviation deliberately, not by omission.** If refreshing always is chosen, that is a
documented difference from BC and it belongs in the item, measured: 771 pages ask for it and the rest
do not.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`RefreshOnActivate =`: **771 declarations**, all necessarily `true`.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no page lifecycle to activate.

## The choice

One bit on the page descriptor, plus the role-centre part notification. **The refresh itself is what a
fragment request already is** in an htmx renderer, so the mechanism is cheap and the DECISION is what
this item is about.

## Ordering

Behind board:0030's page lifecycle and board:0047's FlowFields, which is what a cue shows.

## Gate, and its negative control

Posting from one role-centre part changes the cue in another part declaring the property.

**The negative control is a part NOT declaring it** -- under BC's rule it must NOT refresh, and if
agiru refreshes everything the control goes green and the deviation is invisible. So the gate has to
assert the chosen behaviour explicitly rather than assert BC's.
