Type:     task
Status:   open
Parent:   0034
Area:     gen
Source:   developer/properties/devenv-objectentitlements-property.md
Verdict:  fehlt
Class:    activation

# An entitlement names the object permissions a licence permits

> **Version**: runtime 7.0. Applies to: **Entitlement.**
>
> Determines the object permissions that this **entitlement object** permits a user or application to
> use.
>
> ```al
> entitlement MyEntitlement
> {
>     ObjectEntitlements = "D365 BUS PREMIUM - BaseApp";
> }
> ```

**`Entitlement` is one of the twelve AL object kinds** and CLAUDE.md's scope sentence says every one
of them is transpiled and represented. It is the licence layer: an entitlement maps a purchased plan
onto the permission sets it grants, and it is the gate `InherentEntitlements` (board:0378) exists to
bypass.

This property is the entitlement's payload -- everything else about the object is which plan it
describes.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ObjectEntitlements =`: **203 declarations.**

## The IST-state

`Entitlement` has no generator (board:0034), so neither the object nor the property exists. board:0062
records that no licence gate exists either, so there would be nothing for an entitlement to gate.

## The choice

The list resolves at translation time to the permission sets it names -- the same flattening
board:0379 does for a permission set's includes, and reusing it rather than a second resolver.

**Whether agiru HAS a licence is the open question and it is not this item's to answer.** An ERP with
no licence gate is a defensible position for a standalone system; the object still transpiles,
because CLAUDE.md's scope sentence says a kind with no generator is a hole with a count and never a
decision. So this item builds the object and the property, and board:0062 decides whether anything
consults them.

**ANSWERED by board:0559, and by citation rather than by preference.** Both concept pages state it
without qualification: *"entitlements are ONLY USED IN THE ONLINE VERSION of Business Central."*
agiru is one process on the user's own machine, so **nothing consults them** -- and the platform's own
composition rule makes that fall out rather than be a special case: actual permissions are the
INTERSECTION of entitled and assigned, and with no entitlement layer the entitled operand is the
universe. An implementation that gated here would be more restrictive than BC.

## Ordering

Inside board:0034's entitlement generator, which does not exist. Behind board:0379 for the
flattening.

## Gate, and its negative control

An entitlement declaring `ObjectEntitlements` transpiles and its named permission sets resolve to
their flat masks.

**The negative control is a name no permission set carries** -- it must fail to transpile, or the
entitlement silently grants nothing and the licence gate that reads it lets everyone through.
