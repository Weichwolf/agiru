Type:     task
Status:   open
Parent:   0062
Area:     gen
Source:   developer/attributes/devenv-requiredpermissions-attribute.md
Verdict:  fehlt
Class:    activation

# A `[RequiredPermissions]` attribute gates who may subscribe to an external event

`[RequiredPermissions(PermissionObjectType: PermissionObjectType, ObjectId: Integer, Permissions: Text)]`
-- "Specifies the permissions required by the SUBSCRIBER of an external event."

It is the mirror of board:0202: `[InherentPermissions]` gives a code path a right it would not have;
this one demands a right of whoever subscribes. Both sit on a method, both name (object type, id,
mask), and they point in opposite directions.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**0 occurrences** (2026-09-04). Runtime version 11.0 introduced it and the read roots do not use it
yet -- which is stated rather than rounded, and it sets the ordering: last.

## The IST-state

The attribute parses into the raw list and is dropped.

## The choice

`constexpr` metadata beside the method, alongside board:0197's external-event declaration -- the two
belong to the same feature and the same object. The check happens where an external subscription is
registered, which is the transport agiru does not build.

**So this item carries the DECLARATION and explicitly not the enforcement**, and it closes when the
metadata is emitted. Anything more would be enforcing a rule against a subscriber that cannot exist.

## Ordering

Last of the attribute family: zero call sites and a consumer that `scope.json` excludes. It is filed
because board:0190's rule is that every attribute is acted on or refused, and "carried, inert, zero
population" is the third state that has to be recorded to reach 41 of 41.

## Gate, and its negative control

An annotated method carries the three values in its metadata. **The negative control is a method
with a malformed argument list -- it must FAIL the build**, because an attribute whose arguments are
not parsed is indistinguishable from one that was dropped.
