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

## THE GRAMMAR, from `devenv-set-relationships-between-tables.md` (read 2026-09-04, board:0071)

The page gives it formally, and it is a small language rather than a pair of ids:

```
<TableRelation> = <TableName>[.<FieldName>] [WHERE(<TableFilters>)]
                | if (<Conditions>) <TableName>[.<FieldName>] [WHERE(<TableFilters>)]
                  else <TableRelation>
<TableFilter>  = <DstFieldName>=CONST(<FieldConst>)
               | <DstFieldName>=FILTER(<Filter>)
```

Three things follow that this item's `{field, kind, value}` array has to carry:

- **`else` CHAINS.** The `if ... else <TableRelation>` is recursive, so a field can carry a list of
  conditional relations rather than one with a condition. The `constexpr` shape is a sequence of
  {condition, target, filters} tried in order, not a single entry with an optional `where`.
- **`FILTER(<Filter>)` is the FILTER LANGUAGE** (board:0018), inside table metadata. So the relation
  check parses a filter expression, which means the two items meet: a `constexpr` relation table
  that stores a filter STRING needs the parser at run time, and one that stores a parsed form needs
  the parser at translation time. The second is this tree's own answer everywhere else.
- **"You can define a relationship only to a field that is a member of the primary key group."**
  That bounds the check: the target is always a primary-key field, so the lookup is a `Get` and
  never a scan.

`FieldDef` gains the relation: the target table's id, the target field's number, and the `where`
conditions as a small `constexpr` array of {field, kind, value} -- `const` and `filter` are the two
kinds the BaseApp uses. All of it is knowable at translation time, so all of it is `constexpr` data
beside the field table, like everything else the transpiler emits.

A relation whose target this run does not have is a HOLE WITH A COUNT and not a silent zero: the
generator counts it the way it counts an absent object.

## The neighbours at the same call site

`Validate` is where four other declared properties are checked, and they arrive together or the call
site is rewritten four times: `NotBlank` (949), `MinValue` (897), `MaxValue` (346) and
`AutoIncrement` (151) are board:0068, and `ValidateTableRelation` (457 declarations, measured
2026-09-04) is this item's own switch. The relation is 9 275 declarations, so it is the large one and
sets the shape.

## Gate

A field with a relation refuses a value no row carries and accepts one that exists; the message is
BC's own. A conditional relation accepts under the condition and refuses outside it. The negative
control removes the check and the first case must go red.
