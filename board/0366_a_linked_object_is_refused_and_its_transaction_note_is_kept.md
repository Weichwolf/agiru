Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-linkedobject-property.md, developer/properties/devenv-linkedintransaction-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A linked object is refused, and the reason it exists is kept

**Two pages, one item**: `LinkedInTransaction` "is available when the `LinkedObject` property is set
to true", so the second cannot be declared without the first.

> **LinkedObject** (Table): **True** if a link to SQL Server objects is provided. **The default is
> false.**
>
> **LinkedInTransaction** (Table, deprecated in the runtime version that introduced it): Gets and
> sets data from **linked server data sources, such as Microsoft Office Excel, Access, or another SQL
> Server**.
>
> **The access to linked server data sources is not under Business Central transaction control. This
> means that if a Business Central transaction is aborted, then any changes that were made to a
> linked object ... will remain in effect.**

**That last paragraph is why this item exists at all**, and it is worth keeping even though the
answer is a refusal. It describes a table whose writes escape the transaction -- and CLAUDE.md's first
invariant is that a posting is all or nothing, that a boundary rolls back everything inside it. A
linked object is a documented hole in exactly that guarantee.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`LinkedObject =` **0** · `LinkedInTransaction =` **0**. Neither appears anywhere.

## The IST-state

Unknown to the generator's consumers.

## The choice

**Refuse both**, joining board:0327, board:0333, board:0346, board:0347 and board:0361. Zero
declarations makes the refusal free, and here it is more than free: accepting `LinkedObject` would be
accepting a table whose writes are outside the transaction, against the invariant that outranks every
other goal in this tree.

**And the reason is recorded rather than left implicit.** If a linked object is ever wanted, the
argument has to start from the rollback paragraph above, not from "the property exists".

## Ordering

With board:0067's census.

## Gate, and its negative control

A table declaring `LinkedObject = true` fails to transpile with a message naming the property.

**The negative control is `LinkedObject = false`** -- the default, explicitly written. It must be
ACCEPTED, because it declares the absence of the feature; a refusal that matches on the property name
rather than the value refuses a table that asked for nothing.
