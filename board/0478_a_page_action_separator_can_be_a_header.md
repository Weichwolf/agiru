Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-isheader-property.md, developer/properties/devenv-groupname-property.md
Verdict:  fehlt
Class:    activation

# A page action separator can be a header, and an entitlement names an Entra group

**Two pages, one item, and they are unrelated** -- which is the exception this sweep otherwise avoids.
They are grouped because each is a single sentence with a single value on a small object, and the
alternative is two files that say one line. The ledger records the pairing as one of convenience
rather than substance.

> **IsHeader** (Page Action Separator): **true** if the separator is a header; otherwise false.
>
> **GroupName** (Entitlement, runtime 7.0): **"If the entitlement type is
> `ConcurrentUserServicePlan`, the `GroupName` determines which Microsoft Entra GROUP that users with
> this entitlement should be members of."**

**`IsHeader` turns a separator into a labelled section**, so an action menu's separators are of two
kinds: a rule, and a caption above a group. The caption comes from board:0382.

**`GroupName` is board:0381's entitlement with an external directory on the end of it.** Microsoft
Entra is a cloud identity service agiru does not talk to, so the value is carried and not acted on --
the same position board:0381 takes on the entitlement object itself, and for the same reason: the
object transpiles because CLAUDE.md's scope sentence says every AL object kind is represented.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`IsHeader =` **272** (all necessarily `true`) · `GroupName =` **16**.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; `Entitlement` has no generator (board:0034).

## The choice

One bit on the separator descriptor, rendered as a labelled section rather than a rule. One
`string_view` on the entitlement, carried and unused.

**`GroupName`'s condition is a `static_assert`**: the property is documented as meaningful only when
the entitlement type is `ConcurrentUserServicePlan`, and both are declarations.

## Ordering

`IsHeader` with board:0030's action metadata. `GroupName` inside board:0381's entitlement generator.

## Gate, and its negative control

A separator declaring `IsHeader` renders as a labelled section with its caption; one declaring nothing
renders as a rule.

**The negative control is the plain separator** -- an implementation that renders every separator with
its caption produces a labelled section where BC draws a line, and 272 declarations means most
separators are NOT headers.

## The metadata half landed 2026-09-07 (board:0553)

The property this item is about now reaches `constexpr` metadata -- `ControlDef`/`PageDef` in
`include/meta/PageDef.h`, `FieldDef`/`TableDef` in `meta/TableDef.h`, `CodeunitDef` in
`meta/CodeunitDef.h` -- emitted per object into the `.cpp` and reached through the object's traits.
What is open here is what READS it, not what carries it: the property census over the whole BaseApp
went from 46 203 declarations dropped in silence to 813 in the same round.
