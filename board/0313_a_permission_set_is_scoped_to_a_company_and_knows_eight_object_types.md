Type:     task
Status:   open
Parent:   0062
Area:     rt, gen
Source:   dev-itpro/security/Data-Security.md
Verdict:  fehlt
Class:    activation

# A permission set may be scoped to ONE COMPANY, and the security model knows eight object types

`Data-Security.md` describes four layers -- database, company, object, record -- and two of them are
this item.

**The company layer**: "When you assign a permission set to a user, you can specify a company to
restrict the user's access for that permission set to that specific company." So the effective set
is assembled per (user, COMPANY), not per user -- and it is one more thing board:0060's company
switch invalidates.

**The object layer knows EIGHT object types**, not the twelve AL has:

| | |
|---|---|
| `Table Data` | the rows |
| `Table` | the table itself |
| `Page`, `Report`, `Codeunit`, `XMLport`, `Query` | the executable objects |
| `System` | "the system tables in the database that allow the user to make backups, change license files, and so on" |

Enum, Interface, Profile, PermissionSet, ControlAddIn and Entitlement carry no permissions at all.
The five verbs are Read, Insert, Modify, Delete, Execute.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`permissionset` objects: **555**; `Permissions =` declarations: **3 897**.

## The IST-state

PermissionSet has no generator: board:0034's object-kind table lists it among the kinds with none.
No permission is declared, assembled or checked.

## The choice

The generated permission entry is `constexpr` data with an **8-way object-kind enum** and a
`static_assert` that nothing else reaches it -- the type system saying the thing the specification
says, at zero cost.

The company scope is a field on the assignment rather than on the set, because the same set may be
granted globally to one user and per company to another.

## Ordering

First of board:0062: the generator comes before the check, because a check has nothing to read until
the sets are emitted. It closes a row of board:0034 with no runtime behaviour attached.

## Gate, and its negative control

A permission set granted to a user for one company only: the user reads the table in that company
and not in another.

**The negative control is the second company** -- an effective set assembled per user rather than per
(user, company) grants both and passes every single-company test.
