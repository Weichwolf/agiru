Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/properties/devenv-includedpermissionsets-property.md, developer/properties/devenv-excludedpermissionsets-property.md
Verdict:  fehlt
Class:    activation

# A permission set composes from other permission sets

**Two pages, one item**: each names the other as its counterpart and they are the two operators of one
composition.

> **IncludedPermissionSets** (runtime 7.0, Permission Set and Permission Set Extension): Sets the
> lists of other permission sets that are **included** in this permission set.
>
> **ExcludedPermissionSets** (runtime 10.0, Permission Set only): Sets the lists of other permission
> sets that are **excluded** in this permission set.

**The asymmetry is declared and it matters**: a permission set EXTENSION may include and may not
exclude. So an extension can only ever widen, which is the same direction rule board:0033 gives the
app boundary -- an extension may not take away what the base app granted.

**And exclusion makes composition order-dependent**, which is where an implementation goes wrong: a
set that includes `BASIC` and excludes `BASIC` is not empty by construction, it depends on whether
exclusion is applied to the transitive closure or to the direct includes. The page does not say, and
`devenv-permissionset-composing.md` is the reference it points at -- read before this is built, not
after.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`IncludedPermissionSets =` **968** · `ExcludedPermissionSets =` **1**.

**Inclusion is the mechanism and exclusion is an escape hatch used once** in the entire BaseApp. That
ratio decides the build order: the transitive closure of includes is the feature, and exclusion is a
single case that must not be allowed to complicate it.

## The IST-state

board:0062: no permission check. PermissionSet is one of the object kinds with no generator
(board:0034), so the sets do not exist to compose.

## The choice

The composition is resolved by the GENERATOR into a flat mask per permission set -- a set's effective
permissions are known at translation time, since every input is a declaration. So the runtime looks up
one flat table and never walks a graph.

**A cycle in the includes is a translation error**, and it is only findable at translation time.

## Ordering

Behind board:0034's PermissionSet generator. Ahead of board:0062's check, which needs the flat masks.

## Gate, and its negative control

A set including two others grants the union of their permissions; a set that includes and excludes
the same one grants what `devenv-permissionset-composing.md` says.

**The negative control is the cycle** -- A includes B includes A must fail to transpile, and a
resolver that memoises without detecting the cycle either loops or silently returns a partial union.
