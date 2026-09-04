Type:     task
Status:   open
Parent:   0062
Area:     rt, gen
Source:   developer/devenv-inherent-permissions.md
Verdict:  fehlt
Class:    activation

# An inherent permission is granted for the duration of the call and then revoked

board:0378 filed `InherentPermissions` and `InherentEntitlements` as object properties. **This page is
the METHOD-scoped form** -- the attribute -- and it states the lifetime, which no property page does.

> "Developers can grant permissions to a method or event **while code executes. AS SOON AS THE CODE
> EXECUTION IS COMPLETED, PERMISSIONS ARE REVOKED.**"
>
> ```AL
> [InherentPermissions(PermissionObjectType::TableData, Database::<MyTable>, 'r',
>                      InherentPermissionsScope::Both)]
> ```
>
> **"You can use inherent permissions ONLY FOR OBJECTS WITHIN THE SAME EXTENSION."**
>
> **"After you apply inherent permissions, SECURITY ADMINISTRATORS CAN NO LONGER CONTROL the
> permission."**
>
> `InherentPermissionsScope` is **optional and defaults to `Both`**, covering permissions and
> entitlements.

**A scope with a revoke is exactly board:0376's shape** -- that item says the `Permissions` property's
grant "is a scope, not a flag ... a stack discipline" -- and this page confirms it for the attribute
form. So there is one mechanism: a permission scope pushed on entry and popped on exit, and three
declarations feed it (`Permissions` on an object, `InherentPermissions` on an object,
`[InherentPermissions]` on a procedure).

**The same-extension rule is a `static_assert`**: the object id and the declaring object's app are
both known at translation time, and board:0033 owns the app boundary. A grant that crossed it would
be an extension elevating its own privileges against the base app, which is precisely what
board:0492's additive-extension warning describes.

**And the administrator-cannot-control clause is a design consequence worth recording**: an inherent
permission is not in any permission set, so board:0492's flat masks do not contain it and no "view all
permissions" listing shows it. It is a second, invisible source of authority.

## The documentation's own guidance, which is the population's shape

> - **General Ledger** -- contains business data, **do not** apply inherent permissions.
> - **Math module** in the System Application -- no business data, **may**.
> - **indirect Read on the General Ledger table when people sign in**, because it is only used to get
>   the work date.
> - **Install and Upgrade codeunits** -- inherent Execute.
> - **Buffer tables** -- indirect RIMD, "because they're only used to hold data in memory".

So the intended population is system tasks, buffers and the install/upgrade path -- board:0270-0277's
territory -- and not business logic.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0062 measured the attribute form: **`[InherentPermissions(...)]` 93 in 57 files under
`Layers/W1`**, parsed nowhere. board:0378 measured the properties: `InherentPermissions` **3 199**,
`InherentEntitlements` **3 170**.

## The IST-state

board:0062: no permission check; `InherentPermissionsScope` is "a door header with values and nothing
that reads it". `src/al/Parser.cpp:545` reads every attribute into `ProcedureDecl::attributes` as raw
text, so the declaration survives parsing and is dropped by the generator (board:0190's census: four
attributes acted on of 41).

## The choice

**One permission scope, three declarations.** The scope is pushed on entry to the procedure or object
and popped on exit -- RAII, which is what C++ has and AL describes -- and board:0376's `Permissions`,
board:0378's two properties and this attribute all push onto it.

`InherentPermissionsScope::Both` is the default and covers the licence gate as well, which is why
board:0378 groups the two properties.

## Ordering

Behind board:0062's permission check. With board:0376, which is the same scope from the object side.

## Gate, and its negative control

A user without read permission on a table can run a procedure declaring inherent read on it, and the
SAME user cannot read that table directly after the procedure returns.

**The negative control is the read after the return** -- an implementation that pushes and never pops
passes the positive half and grants every user every permission any procedure ever declared, which is
board:0376's negative control arriving through the attribute.
