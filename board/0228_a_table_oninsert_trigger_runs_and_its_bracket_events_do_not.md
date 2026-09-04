Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/table/devenv-oninsert-table-trigger.md, developer/triggers-auto/tableextension/devenv-oninsert-tableextension-trigger.md
Verdict:  implementiert
Class:    activation

# A table's `OnInsert` runs on `Insert(true)`, and the two events that bracket it do not

`OnInsert` runs when a record is inserted AND the caller asked for triggers: `Insert()` does not run
it, `Insert(true)` does. The same trigger may be declared on the table or on a `tableextension`, and
the two pages describe one trigger -- board:0033 merges the declarations at translation time, so
this is one task.

## The IST-state -- this one is implemented, and the check is named

`include/runtime/Table.h:353`:

```cpp
Boolean Insert(Boolean RunTrigger) {
  if (RunTrigger) {
    if constexpr (requires(Derived &record) { record.OnInsert(); }) {
      static_cast<Derived *>(this)->OnInsert();
    }
  }
  return Insert();
}
```

The `requires` is the right shape: the generated class declares `OnInsert` exactly when its `.al`
does, so an object without the trigger costs nothing and no registry has to be kept in step.
`Insert()` without the argument goes straight to `RuntimeInsert` and runs nothing, which is AL's
rule.

## What this task still owes

- **The two bracket events are not raised.** board:0029's order is `OnBeforeInsertEvent`, then
  `OnInsert`, then `OnDatabaseInsert`, then the row, then `OnAfterInsertEvent`. Only the middle
  step exists. The events are their own items (0244, 0245) and this one is where the CALL SITE for
  them lives.
- **`Insert(RunTrigger, InsertWithSystemId)` does not exist**, and `Table.h:318` says so in its own
  `\warning`: the two-argument overload is the one that honours a caller-assigned `SystemId`, and
  it needs trigger dispatch. Until it exists, `Rec.SystemId := X; Rec.Insert(true, true)` silently
  gets a platform-generated id.
- **The trigger runs BEFORE the platform's existence check**, which board:0029 records as the
  non-obvious order. `Insert(true)` here calls the trigger and then `Insert()`, which is that order
  -- and it is worth a gate case because the reverse looks equally plausible.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnInsert()` on a table or tableextension: **1 261 declarations.**

## Ordering

The bracket events (0244, 0245) hang off this call site, and board:0057's dispatcher is what they
need. Nothing else blocks.

## Gate, and its negative control

`Insert(true)` on a table whose `OnInsert` writes a field: the written value reaches the row.
`Insert()` on the same table: it does not. A table with no `OnInsert` at all inserts under both.

**The negative control is `Insert()`** -- a runtime that runs the trigger unconditionally passes the
first and third cases and breaks every BaseApp call site that passes `false` to avoid side effects.
