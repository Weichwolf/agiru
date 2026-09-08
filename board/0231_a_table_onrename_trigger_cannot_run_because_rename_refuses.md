Type:     task
Status:   open
Parent:   0029
Area:     rt
Source:   developer/triggers-auto/table/devenv-onrename-table-trigger.md, developer/triggers-auto/tableextension/devenv-onrename-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A table's `OnRename` runs on `Rename`, and `Rename` itself refuses today

`OnRename` runs when a record's PRIMARY KEY changes. It is the only one of the four table triggers
whose operation is not a plain write: renaming moves a row to a new key and the platform must
carry every relation that pointed at the old one.

## The IST-state -- the trigger cannot run because the operation does not exist

`include/runtime/Table.h:1141`:

```cpp
template <typename... Arguments> Boolean Rename(Arguments &&...arguments) const {
  ...
  throw Error("Record.Rename is declared and not implemented yet (board:0035)");
}
```

So unlike `Insert`, `Modify` and `Delete` -- which fire their triggers at `Table.h:353`, `:381` and
`:406` -- there is no `Rename(RunTrigger)` overload and no call site for `OnRename`. **Verdict
`fehlt` rather than `deklariert`**: the variadic refusal accepts any argument list, so even the
signature is not distinguished.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnRename()`: **181 declarations**, and `.Rename(` at **1 003 call sites**.

## The choice

`Rename(Keys...)` and `Rename(Keys..., RunTrigger)`, shaped like `Insert`: the trigger under
`if constexpr (requires ...)`, then the operation. The operation is an `UPDATE` of the primary key
columns -- **not a delete plus an insert**, because the row keeps its `SystemId` and its rowversion
must advance once (board:0013), and a delete/insert pair would break both and fire the wrong
triggers.

**What makes this harder than the other three**: AL's `Rename` also updates every table whose
`TableRelation` points at the renamed key (board:0043). That cascade is the real work and it is why
this item is ranked behind the other three triggers rather than beside them.

## Ordering

Behind board:0043, which owns the relation metadata the cascade reads. The trigger itself is four
lines once `Rename` exists.

## Gate, and its negative control

`Rename` on a record whose `OnRename` writes a field: the new row carries it, and the old key is
gone. A second table with a `TableRelation` to the renamed key must follow.

**The negative control is the related table.** A `Rename` that moves the row and leaves the relation
pointing at a key that no longer exists passes every single-table assertion.

## Standing (2026-09-08): the rename runs; the cascade does not

`Record.Rename` runs `OnBeforeRenameEvent`, the table's `OnRename` (with `xRec` the row as it
was), the row operation and `OnAfterRenameEvent`. The row operation is `RenameRow`: an UPDATE of
every stored column addressed by the OLD primary key, so `SystemId` and the version survive; a
temporary row is deleted under its old key and inserted under the new one.

**Not done: the cascade.** BC rewrites every field related to the renamed key through
`TableRelation` -- the customer number in every entry, line and setup that names it. Here those
rows keep the old value. The population is the `relationTable`/`relationField` pairs the
metadata already carries, so the cascade is a walk over the catalogue's tables for relations
pointing at this table, one UPDATE each; it needs the relation's WHERE filters honoured, and
that is the part with a measurement to take first.

