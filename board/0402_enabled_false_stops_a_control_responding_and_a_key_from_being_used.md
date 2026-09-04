Type:     task
Status:   open
Parent:   0030
Area:     gen, rt, db
Source:   developer/properties/devenv-enabled-property.md
Verdict:  fehlt
Class:    activation

# `Enabled = false` stops a control responding -- and stops a KEY being used

> Sets a value that indicates whether a **field or key** is enabled or disabled, or whether a control
> can respond to user-generated events. **The default is true.**
>
> Applies to: **Table field, TABLE KEY**, Page Label, Page Field, Page Group, Page Part, Page System
> Part, Page Chart Part, Page Action, Page Action Group, Page Custom Action, Page System Action, Page
> File Upload Action, **Profile**.

**"Table key" in that list is the half nobody expects**, and it is not a UI concern at all: a disabled
key is a key the runtime may not sort by and the database need not index. `include/Builtins.h:551`
already records the neighbouring rule -- "Disabling clustered or unique keys is also not supported and
will fail at runtime" -- so the tree has met this property from the other side without naming it.

Against board:0345's finding that `src/rt/Storage.cpp:112` builds an index for every key
unconditionally, a disabled key is **another index BC does not create**, on top of the 158
`MaintainSqlIndex = false` ones.

The Profile half has its own page (`devenv-enabled-profile-property.md`) and its own item.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Enabled =`: **9 514 declarations**, controls and keys together. They are not separable by `grep`; the
key half is measured by counting declarations inside a `keys` block when the item is pulled.

**That is a limit of the measurement and it is said rather than rounded.**

## The IST-state

`include/meta/TableDef.h:98` -- `KeyDef` carries `name`, `fields`, `clustered`, so a key cannot be
disabled. `src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One bit on `KeyDef` -- read by the schema writer, which skips the index, and by `SetCurrentKey`, which
refuses the key -- and one bit per control, disabling rather than removing it: a disabled control is
rendered and does not accept input, which is what "cannot respond to user-generated events" means and
is the opposite of board:0401's removal.

## Ordering

The key half with board:0345, which is the same `CREATE INDEX` decision. The control half with
board:0400.

## Gate, and its negative control

A disabled key produces no index and `SetCurrentKey` onto it raises; a disabled control renders and
ignores input.

**The negative control is the control's presence** -- an implementation that reuses board:0401's
removal for `Enabled` passes an input-ignoring gate and deletes 9 514 controls from their pages.
