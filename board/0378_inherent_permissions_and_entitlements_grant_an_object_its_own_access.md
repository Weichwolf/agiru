Type:     task
Status:   open
Parent:   0062
Area:     rt, gen
Source:   developer/properties/devenv-inherentpermissions-property.md, developer/properties/devenv-inherententitlements-property.md
Verdict:  fehlt
Class:    activation

# Inherent permissions and entitlements grant an object its own access

**Two pages, one item.** They apply to the same six object kinds, take the same permission letters,
and differ in which of BC's two gates they open: `InherentEntitlements` (runtime 10.0) satisfies the
LICENCE, `InherentPermissions` (runtime 11.0) satisfies the PERMISSION SET. Neither is meaningful
without the other -- an object that passes one gate and fails the other is unreachable.

> **InherentPermissions**: Specifies the permissions that are inherently assigned to the given
> object. Developers can define inherent entitlements for their objects so that **all users have
> enough access to carry out essential tasks without any halt, and regardless of what access their
> present license or entitlement grant them.**
>
> **InherentEntitlements**: The inherent entitlements permission values *Read*, *Insert*, *Modify*,
> *Delete* and *Execute* **are set according to the object type**. For example, a Report object can
> have an execute `X` permission, but the same permission is not valid for data in a Table object.
>
> Applies to (both): **Query, Report, Xml Port, Table, Codeunit, Page.**

**This is `Permissions` (board:0376) from the other end.** `Permissions` widens what an object may do
while it runs; these two say what a user may do WITH the object regardless of their licence. The
first is a grant the object makes, the second is a grant the object carries.

The per-kind validity is decidable from the declaration -- `X` on a table's data is not valid, `R` on
a report is not -- so it is a `static_assert`, the same one board:0377 needs.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`InherentPermissions =` **3 199** · `InherentEntitlements =` **3 170**.

**They track each other within 1 %**, which is the pairing measured: an object that declares one
declares the other, because passing one gate alone leaves it unreachable.

board:0062 counts the ATTRIBUTE form separately -- `[InherentPermissions(...)]` on a procedure, 93 in
57 files under `Layers/W1`. That is a third spelling of the same idea and it lands the same way.

## The IST-state

board:0062: no licence gate, no permission gate. `InherentPermissionsScope` exists as a door header
with values and nothing that reads it.

## The choice

A mask on the object, `constexpr`, parsed by the same parser board:0376 and board:0377 share -- three
properties and one attribute reading one value syntax, which is why a second parser for any of them
would be a mistake.

**agiru has no licence**, so `InherentEntitlements` has no gate to open. It is carried and recorded as
such rather than refused: 3 170 declarations, and the day a licence gate exists it is the input.
`InherentPermissions` has a gate as soon as board:0062 does.

## Ordering

Behind board:0062. With board:0376 and board:0377 for the shared value parser.

## Gate, and its negative control

A user whose permission set grants nothing can still run a codeunit declaring
`InherentPermissions = X`.

**The negative control is a codeunit declaring nothing** -- it must be refused for that same user, or
the gate is measuring an absent check rather than an inherent grant.
