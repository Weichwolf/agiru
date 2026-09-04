Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-changetrackingallowed-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `ChangeTrackingAllowed` is accepted and recorded as having no effect

> **Version**: runtime 2.0. Applies to: **Page**.
>
> Sets a value that indicates whether the entity exposed through the **OData API** supports change
> tracking. When true, **an annotation is written in the OData metadata document**. The default is
> false.
>
> The property can only be set if the `PageType` property is set to **API**.
>
> **From Wave 1 2024 setting this property has no effect, as delta links are no longer supported.**

**Microsoft has already declared it a no-op**, which is a different situation from every other
property in this sweep: it is not unimplemented here, it is unimplemented THERE. So the decision is
not whether to build it but how to record that there is nothing to build.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ChangeTrackingAllowed =`: **63 declarations**, all of them necessarily `true`.

63 API pages still declare it after it stopped doing anything, which is what a deprecated property
looks like in a tree this size.

## The IST-state

Not among the nine properties the generator consumes (board:0067); no OData surface exists.

## The choice

**Accept and ignore, with this item as the citation** -- the same handling board:0333's first version
proposed and the opposite of what board:0333 ended up with, and the difference is the reason:
`TestTableRelation` has 0 declarations so a refusal is free, this has 63 so a refusal would stop 63
pages translating over a property that does nothing in BC either.

board:0067's census lists it as KNOWN-AND-IGNORED with the documentation's own sentence as the
reason, so the decision is provable from the census rather than from a commit message.

The `PageType = API` precondition is still a `static_assert`: it is decidable and BC's own compiler
enforces it, so accepting the property on a non-API page would be accepting what AL refuses.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

63 pages declaring it transpile; one declaring it on a non-API page does not.

**The negative control is the non-API page** -- an implementation that ignores the property entirely
passes the first half and accepts what the AL compiler rejects.
