Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/properties/devenv-assignable-property.md
Verdict:  fehlt
Class:    activation

# An `Assignable` permission set can be given to a user

> **Version**: runtime 7.0. Applies to: **Permission Set.**
>
> Sets whether the permission set **can be assigned to a user**. Assignable permission sets are
> permissions that an admin can assign to users, using the **Permission Sets** page.

So a permission set has two roles and this property picks which: a set a user is given, or a
BUILDING BLOCK that only other sets include (board:0379). BC's own sets are layered that way -- a
non-assignable `... - Objects` set that several assignable ones include.

**The page does not state the default**, and that is not a detail: whether an undeclared set is
assignable decides what an admin sees on the Permission Sets page. `devenv-permissionset-object.md`
is where to look, and 1 116 declarations against however many sets exist will show which way the
BaseApp leans.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Assignable =`: **1 116 declarations**, against 968 `IncludedPermissionSets`. The two populations are
the same order, which fits the layering: roughly as many sets are assigned as are composed.

## The IST-state

board:0062: no permission check anywhere. PermissionSet has no generator (board:0034), so no set
exists to be assignable.

## The choice

One bit on the generated permission set, and **it is not a permission check** -- it gates
ASSIGNMENT, which is an administrative operation, not a read. So it belongs with whatever holds the
user-to-set mapping and never on the path of a record read, where board:0062's cost is measured.

## Ordering

Behind board:0034's PermissionSet generator. Independent of board:0062's check.

## Gate, and its negative control

Assigning a non-assignable set to a user is refused; assigning an assignable one succeeds; a
non-assignable set still contributes its permissions when another set includes it.

**The negative control is the third assertion** -- an implementation that treats non-assignable as
"inactive" passes the first two and empties every set that composes from a building block.
