Type:     task
Status:   open
Parent:   0381
Area:     gen
Source:   developer/properties/devenv-type-property.md, developer/properties/devenv-type-entitlement-property.md, developer/properties/devenv-roletype-property.md, developer/properties/devenv-type-report-property.md
Verdict:  fehlt
Class:    activation

# An entitlement declares its type, and `RoleType` qualifies one of them

**Four pages, one item**: the `Type` property's overview, its entitlement-specific page, the
`RoleType` that refines one of its values, and its report-layout page -- one property name across
object kinds, the shape `Scope` (board:0361) and `SubType` (board:0472) also have.

> **Type on entitlements**: the entitlement kind. When it is `Role`, **`RoleType`** distinguishes
> `Local` -- "the user is either a native user or a guest user in the company's Microsoft Entra
> tenant" -- from `Delegated` -- "via a **Delegated Admin relationship** with a partner's Microsoft
> Entra tenant."
>
> When the type is `ConcurrentUserServicePlan`, **`GroupName`** (board:0478) names the Entra group.
>
> **Type on report layouts**: the format of a layout -- board:0452's subject from the other side.

**Both qualifying properties point at Microsoft Entra**, an external identity service agiru does not
talk to. So the entitlement's type is carried, its two qualifiers are carried, and none of them gates
anything -- board:0381 already takes that position for the object and this item extends it to the
type.

**The conditional structure is the checkable part**: `RoleType` only with `Type = Role`, `GroupName`
only with `Type = ConcurrentUserServicePlan`. Both are declarations, so both are `static_assert`s, and
they are the only thing this item delivers that a carried string does not.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Type =` **1 555** across all object kinds; `RoleType =` **56**.

**1 555 is not this property's population** -- `Type` is declared on report layouts, DotNet types and
more, and no statement-boundary pattern separates them.

**COUNTED since, by board:0559, over the 206 `entitlement` declarations themselves**: `Type` appears
206 times in those files, so every entitlement declares one and the property is effectively mandatory.
`PerUserServicePlan` **101**, `Role` 56, `ConcurrentUserServicePlan` 16, `ApplicationScope` 14,
`Implicit` 10, `Application` 9 -- and `FlatRateServicePlan`, `PerUserOfferPlan`, `Unlicensed` and
`Group` **zero each**. `RoleType` splits `Delegated` 32 / `Local` 24, summing to exactly the 56 `Role`
entitlements, so every one of them qualifies itself and the conditional below is never idle.

## The IST-state

`Entitlement` has no generator (board:0034); board:0381 records the state.

## The choice

Enumerators on the entitlement, carried, plus the two conditional `static_assert`s. The report-layout
`Type` belongs to board:0452 and is named here only because it shares the page family.

## Ordering

Inside board:0381 and board:0034's entitlement generator.

## Gate, and its negative control

An entitlement declaring `RoleType` without `Type = Role` fails to transpile; one declaring both
transpiles and carries the value.

**The negative control is `GroupName` without `ConcurrentUserServicePlan`** -- the second conditional,
which an implementation that checks only the first accepts.
