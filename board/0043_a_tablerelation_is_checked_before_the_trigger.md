Type: leaf
State: open
Area: gen

# A field's `TableRelation` is in the metadata, and `Validate` checks it

`Validate` calls `detail::CheckRelation` in the documented place -- before the `OnValidate` trigger --
and the function does nothing, because a field's `TableRelation` is nowhere in `FieldDef`. So a
value that names no row of the related table is accepted.

## Reference

`devenv-tablerelation-property.md`. The relation is a property of the FIELD and it can carry a
condition (`TableRelation = "Item"."No." where(Type = const(Inventory))`), a `ValidateTableRelation`
switch, and an alternative through `if`/`else`.

**THE ORDER IS ALREADY DECIDED AND MEASURED** (openerp, `test_validate_relation_before_trigger`):
the relation check runs BEFORE the trigger. `Service Item Line."Variant Code"` has an `OnValidate`
that raises the moment `"Service Item No."` is set, and BC's own test expects the RELATION message
for a blocked variant on exactly such a line. With the trigger first its error wins and the relation
message can never appear. The call site is in `Table<Derived>::Validate` in that order today.

**`ValidateTableRelation = false` turns the CHECK off and keeps the LOOKUP** -- openerp filed that
as its own item after switching both off together.

## The choice

`FieldDef` gains the relation: the target table's id, the target field's number, and the `where`
conditions as a small `constexpr` array of {field, kind, value} -- `const` and `filter` are the two
kinds the BaseApp uses. All of it is knowable at translation time, so all of it is `constexpr` data
beside the field table, like everything else the transpiler emits.

A relation whose target this run does not have is a HOLE WITH A COUNT and not a silent zero: the
generator counts it the way it counts an absent object.

## Gate

A field with a relation refuses a value no row carries and accepts one that exists; the message is
BC's own. A conditional relation accepts under the condition and refuses outside it. The negative
control removes the check and the first case must go red.
