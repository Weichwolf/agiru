Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/table/devenv-onmodify-table-trigger.md, developer/triggers-auto/tableextension/devenv-onmodify-tableextension-trigger.md
Verdict:  implementiert
Class:    activation

# A table's `OnModify` runs on `Modify(true)`, and `xRec` is what it reads the old values from

`OnModify` runs when a record is updated and the caller asked for triggers. Inside it, `Rec` holds
the new values and **`xRec` holds the ones the row had** -- which is the trigger where that pair
matters most, because the BaseApp's `OnModify` bodies compare them to decide what to cascade.

## The IST-state -- implemented, and the check is named

`include/runtime/Table.h:381`:

```cpp
Boolean Modify(Boolean RunTrigger) {
  if (RunTrigger) {
    if constexpr (requires(Derived &record) { record.OnModify(); }) {
      static_cast<Derived *>(this)->OnModify();
    }
  }
  return Modify();
}
```

Same shape as `Insert` (0228), same `requires` guard, and `Modify()` without the argument runs
nothing.

## What this task still owes

- **`xRec` is not established here.** `Validate` builds a `detail::BeforeImage` (`Table.h:1376`)
  and `Modify` does not, so an `OnModify` body reading `xRec` reads whatever the last validate left
  -- or nothing. board:0042 owns `xRec`; this is the call site that has to open its scope, and it is
  the difference between the two triggers' current state.
- **The bracket events are not raised** (0246, 0247), like 0228.
- **`Modify` does not detect that the row changed.** AL's `Modify` returns `false` when the record
  does not exist; whether it also compares the before-image is not stated on this page and is not
  assumed here.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnModify()` on a table or tableextension: **793 declarations.**

## Ordering

Needs board:0042's `xRec` scope, which `Validate` already builds and this path does not -- so the
mechanism exists and the call site is missing. Before the bracket events.

## Gate, and its negative control

`Modify(true)` on a record whose `OnModify` compares `Rec.Field` with `xRec.Field` and raises when
they differ: changing the field must raise, leaving it alone must not.

**The negative control is the unchanged case.** A runtime with no before-image gives `xRec` the NEW
value, both comparisons come out equal, and the trigger never raises -- which looks like a passing
test.
