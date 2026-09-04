Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   dev-itpro/security/security-auditing.md
Verdict:  fehlt
Class:    activation

# The change log is always on for nine system tables, and it hangs off the global trigger

> The changelog on the above tables **is always turned on and can't be turned off** by using
> **Change Log Setup** page in the client.
>
> The changelog code is called from the **`OnDatabaseInsert` method in the system codeunit 49
> GlobalTriggerManagement**.

The nine: `Access Control` (2000000053), `Permission` (2000000005), `Permission Set` (2000000004),
`User` (2000000120), `User Property` (2000000121), `Tenant Permission Set Rel.` (2000000253), and
the three `Change Log Setup` tables (402, 403, 404).

**This is the concrete reason board:0029's step 3 exists.** The five-step order around a database
operation is: trigger event, the table trigger, **the global table trigger in a codeunit**, the
operation, the after event. `OnDatabaseInsert` is that third step, and a runtime that skips it has
an audit trail with holes in exactly the tables an auditor asks about.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`OnDatabaseInsert`, `OnDatabaseModify`, `OnDatabaseDelete`, `OnDatabaseRename` subscriptions: **28**,
concentrated in codeunit 49 and the change-log module. Small, and every one of them is on the write
path of every table.

## The IST-state

`include/runtime/Table.h:353`, `:381` and `:406` call the table's own trigger and then the operation.
There is no global trigger step, and codeunit 49's subscriptions are among board:0057's 3 753 that
are never called.

## The choice

The write path gains step 3: after the table's own trigger and before the operation, the runtime
raises the global trigger with the table id and the record. **It is raised for EVERY table**, which
is what "global" means -- codeunit 49 decides whether the table is one of the nine, and that
decision is AL, not runtime (CLAUDE.md: the runtime knows no AL object).

## Ordering

After board:0057's dispatcher. With board:0228-0231, which own the call sites this step is inserted
into.

## Gate, and its negative control

Insert a row into a table with a global-trigger subscriber: the subscriber runs, after the table's
own `OnInsert` and before the row exists.

**The negative control is the ORDER** -- a step placed after the operation gives the subscriber a
row it was meant to inspect beforehand, and any assertion that only checks it ran passes.
