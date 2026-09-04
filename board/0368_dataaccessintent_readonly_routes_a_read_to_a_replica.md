Type:     task
Status:   open
Parent:   0045
Area:     rt, db
Source:   developer/properties/devenv-dataaccessintent-property.md
Verdict:  fehlt
Class:    activation

# `DataAccessIntent = ReadOnly` routes a read to a replica

> **Version**: runtime 5.0. Applies to: **Page, Report, Query.**
>
> `ReadOnly` -- intent to access records, but not to modify them. **Read-only pages are run against a
> replica of the database**, leading to improved performance but preventing modifications.
>
> **ReadOnly works as a hint** for the server to route the connection to a secondary (read-only)
> replica, **if one is available**. When a workload is executed against the replica, insert/delete/
> modify operations aren't possible. **If any of these operations are executed against the replica,
> an exception is thrown at runtime.**

**This is CLAUDE.md's "reads can go to a replica; a write cannot", declared per object**, and the
documentation's scope is narrower than the name suggests:

| kind | when it applies |
|---|---|
| **Page** | only `PageType = API`, and `Editable` must be `false`; only the OData FETCH uses the replica -- the `OpenCompany` triggers run against the primary |
| **Query** | only queries exposed through OData. **"It has no effect in normal code paths."** |
| **Report** | also in UI sessions: the data items' iteration uses the replica, the reads before and after the data set do not |

And two overrides that are not the AL declaration at all: page **9880 Database Access Intent List**
changes it at run time, and an HTTP request header `Data-Access-Intent` overrides it per call.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataAccessIntent =`: **367 declarations, every one of them `ReadOnly`.** `ReadWrite` is the default
and is never written.

## The IST-state

A session borrows a connection for its transaction (board:0012) and there is one connection kind. No
replica, no routing, and neither reports nor queries are generated.

## The choice

**The property lands as metadata and the routing waits for a replica to exist**, which is a
deployment question this tree has not answered. What does NOT wait is the REFUSAL: a `ReadOnly`
object that writes must raise, and that is checkable without any replica at all -- the intent marks
the session's transaction read-only and `RuntimeInsert`, `RuntimeModify` and `RuntimeDelete` refuse
inside it.

**That half is worth building alone.** It is the documented behaviour, it needs no infrastructure, and
it turns 367 declarations from decoration into a checked contract.

The per-kind narrowing is a `static_assert` on the page half: `DataAccessIntent` on a page whose
`PageType` is not `API`, or whose `Editable` is not `false`, is a translation error.

## Ordering

Behind board:0012's connection handling for the routing; the refusal half is independent and can go
with the transaction.

## Gate, and its negative control

A report declaring `ReadOnly` that inserts raises; the same report without the property does not.

**The negative control is the query** -- the documentation says the property has NO effect in normal
code paths, so a query declaring `ReadOnly` called from AL must still be able to write. An
implementation that applies the intent everywhere passes the report gate and breaks that.
