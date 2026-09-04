Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/table/devenv-ondelete-table-trigger.md, developer/triggers-auto/tableextension/devenv-ondelete-tableextension-trigger.md
Verdict:  implementiert
Class:    activation

# A table's `OnDelete` runs BEFORE the row goes, which is what lets it stop the delete

`OnDelete` runs when a record is deleted and the caller asked for triggers. It runs BEFORE the row
is removed, and that order is what makes it useful: the BaseApp's `OnDelete` bodies check for
dependent rows and RAISE, which cancels the delete because the row is still there.

## The IST-state -- implemented, and the check is named

`include/runtime/Table.h:406`:

```cpp
Boolean Delete(Boolean RunTrigger) {
  if (RunTrigger) {
    if constexpr (requires(Derived &record) { record.OnDelete(); }) {
      static_cast<Derived *>(this)->OnDelete();
    }
  }
  return Delete();
}
```

The trigger is called and only then is `Delete()` reached, so a raising trigger leaves the row --
correct, and worth a gate case because the reverse order would also compile.

## What this task still owes

- **The bracket events are not raised** (0248, 0249).
- **`DeleteAll(true)` does NOT reach this path**, and must not: it passes a COPY of the record with
  its INITIAL values (board:0044), so an `OnDelete` reading a caller-mutated global sees something
  different. `Table.h`'s `DeleteAll` is a variadic refusal today, so the divergence is not yet
  reachable -- and it is recorded here because the two look like the same trigger and are not the
  same call.
- **`OnDelete` on a table with a `tableextension` that also declares one**: both run, base first,
  which is board:0033's merge order and is not expressed anywhere yet.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnDelete()` on a table or tableextension: **655 declarations.**

## Ordering

Nothing blocks it. The bracket events and `DeleteAll(true)` are separate items that reference this
call site.

## Gate, and its negative control

`Delete(true)` on a record whose `OnDelete` raises: the call raises AND THE ROW IS STILL THERE.
`Delete()` on the same record removes it without running anything.

**The negative control is the surviving row.** A runtime that deletes first and then runs the
trigger passes the "it raised" assertion and loses the row, which is the failure that matters.
