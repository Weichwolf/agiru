Type:     task
Status:   open
Parent:   0062
Area:     rt, gen
Source:   developer/properties/devenv-permissions-property.md
Verdict:  fehlt
Class:    activation

# The `Permissions` property grants an object more than its caller has

> Sets permissions required to perform operations on one or more objects. Applies to: **Codeunit,
> Table, Page, Request Page, Report, Query, Xml Port, Permission Set, Permission Set Extension.**
>
> ```AL
> Permissions = tabledata MyTable = RIMD, codeunit MyCodeUnit = X, page MyPage = X;
> ```
>
> | value | means |
> |---|---|
> | `R` / `r` | direct / **indirect** read |
> | `I` / `i` | direct / indirect insert |
> | `M` / `m` | direct / indirect modify |
> | `D` / `d` | direct / indirect delete |
> | `X` / `x` | direct / indirect execute |

**The case of the letter is the semantics**, which is the trap in this property: `R` and `r` are two
different grants and a case-insensitive read of the value collapses them. That is the same class of
defect as board:0349, arriving in a property where the collapse would be silent and would GRANT
rather than withhold.

> A user who has **Indirect Read** permission **cannot open a page** that displays data from a table
> unless the page has been given permission to read data from the table for the user. That means the
> user can perform an action on the data **only through another object**.
>
> By default, objects do not have the permission property defined and **use the same permissions as
> the user**. However for special cases, such as various **Ledger Entries** tables, users do not have
> write or modify permissions. Instead, users should run **posting routines that create ledger
> entries on behalf of the user**.

**That last paragraph is the whole reason this property exists in an ERP.** A user may not insert
into `G/L Entry`; the posting codeunit may. Without it, either every user can write ledger entries or
no posting runs -- and board:0062 already records that the runtime has no permission check at all, so
today it is the first.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Permissions =`: **4 020 declarations.**

## The IST-state

board:0062 measures the surface: `RecordRef.ReadPermission()`, `WritePermission()` and
`SetPermissionFilter()` all throw at `include/runtime/RecordRef.h:966`, `:1155` and `:1048`;
`Record.ReadPermission` at `include/runtime/Table.h:1099` is a variadic refusal. The property is not
among the nine the generator consumes (board:0067).

## The choice

The value is a small grammar -- `<objectkind> <identifier> = <letters>` repeated -- parsed by the
GENERATOR into `constexpr` data on the object, like board:0331's relation. Each entry is
`{ ObjectKind, object id, mask }` and **the mask distinguishes direct from indirect per operation**,
so it is two bits per operation and not one.

`Permissions = codeunit * = X` is a wildcard (board:0062 records it), so the identifier is either an
id or `*`.

**The grant is a scope, not a flag.** While the object runs, its permissions are added to the
session's; when it returns they are gone. That is a stack discipline and it belongs with board:0061's
`TryFunction` scope and board:0077's `Codeunit.Run`, not beside the record.

## Ordering

Behind board:0062, which has to have a permission check for a grant to widen.

## Gate, and its negative control

A user without insert permission on a table cannot insert directly and CAN insert through a codeunit
declaring `Permissions = tabledata T = I`.

**The negative control is after the codeunit returns** -- the same direct insert must fail again. An
implementation that adds the grant to the session and never removes it passes the positive half and
gives every user every permission any codeunit ever declared.
