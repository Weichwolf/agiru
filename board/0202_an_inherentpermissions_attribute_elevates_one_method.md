Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/attributes/devenv-inherentpermissions-attribute.md
Verdict:  fehlt
Class:    activation

# An `[InherentPermissions]` attribute elevates ONE method while it runs, and revokes on return

`[InherentPermissions(PermissionObjectType, ObjectId, Permissions [, Scope])]` on a method or an
event. `devenv-inherent-permissions.md`: "developers can grant permissions to a method or event
while code executes. **As soon as the code execution is completed, permissions are revoked.**"

The documented purpose is to move a right from the USER to the CODE PATH: a salesperson runs a
report whose method reads a classified table, and instead of granting the salesperson that table,
the method carries the right.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**397 `[InherentPermissions` declarations.**

## The IST-state

`include/type/InherentPermissionsScope.h` exists as a door header. The attribute parses into the raw
list and is dropped, so all 397 elevations are absent -- which is invisible today because there is
no permission layer to elevate against (board:0062).

## The choice

`constexpr` metadata beside the method -- object type, id, the permission mask, the scope -- and a
scoped guard at the top of the emitted body that pushes it onto the session's permission stack and
pops it on exit. The same shape as board:0193's `CommitBehavior` guard, on a different field.

**The stack is what makes "revoked on completion" exact**, including when the method raises: the
guard's destructor runs on the way out either way, which is the C++ shape of "as soon as the code
execution is completed".

**Why not a permanent grant.** A grant that outlived the call would be a privilege escalation that
no AL author asked for, and the page's whole argument is that the elevation is bounded by the code
path.

## Ordering

Blocked on board:0062: there is nothing to elevate until permissions are checked. Ranked with the
rest of that item rather than ahead of it -- and, like board:0062, not a milestone blocker, because
the milestone's cases run as SUPER.

## Gate, and its negative control

A method annotated for `R` on a table the session may not read: the read inside succeeds, and the
same read from the caller AFTER the method returns fails.

**The negative control is the read after the return.** A guard that pushes and never pops passes the
first half and grants the right for the rest of the session.
