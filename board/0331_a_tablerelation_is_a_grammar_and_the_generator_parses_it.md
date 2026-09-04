Type:     task
Status:   open
Parent:   0043
Area:     al, gen, rt
Source:   developer/properties/devenv-tablerelation-property.md
Verdict:  deklariert
Class:    activation

# A `TableRelation` is a grammar, and the generator parses it into `constexpr` data

The property is not a table name. It is a small language, and the page gives its grammar:

```
TableRelation = <TableName>[.<FieldName>] [WHERE(<TableFilters>)] |
[IF(<Conditions>) <TableName>[.<FieldName>] [WHERE(<TableFilters>)] ELSE <TableRelation>]
<Conditions>   ::= <TableFilters>
<TableFilters> ::= <TableFilter> {,<TableFilter>}
<TableFilter>  ::= <DestinationFieldName>=CONST(<FieldConst>) | FIELD(<SourceFieldName>)
```

Four things it does, and the page names all four:

1. **A lookup into another table** -- the dropdown on the field.
2. **Where to look to validate entries** -- the check board:0043 owns.
3. **What to test** when the relations between primary and secondary indexes are tested
   (board:0333).
4. **A conditional target**: `if (Type = const(Customer)) Customer else if (Type = const(Item)) Item`.
   The relation's TABLE depends on another field's value, which is why this cannot be a `TableId` on
   `FieldDef`.

And a fifth from the extension half:

> The `TableRelation` property can be modified through a table extension. **Modifications to the
> `TableRelation` are additive and evaluated after the existing value.** The primary use case is
> conditional table relations based on conditional enums.

So an extension APPENDS an `else` branch. That is a merge rule and it lands in board:0033, which
merges extensions at translation time -- but only this item knows what "additive" means for this
property.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TableRelation =`: **40 221 declarations.** The largest population measured in this sweep, by a
factor of four over `DecimalPlaces`. It is on almost every Code field in the BaseApp.

## The IST-state

- `src/al/Ast.h:67` -- `FieldDecl::properties` holds the property as its raw text, like every other.
  **Nothing parses it.**
- `src/gen/TableWriter.cpp` -- the generator consumes `Caption`, `OptionCaption`, `OptionMembers`
  and `InitValue` on a field and nothing else, so the relation is dropped.
- `include/meta/TableDef.h:67` -- `FieldDef` has no relation member.
- `include/runtime/Table.h:116` and `src/rt/Table.cpp:350` -- `CheckRelation` is DECLARED, is called
  from `Validate` at the documented point (`Table.h:1380`), and its body is three `static_cast<void>`
  discards. **The call site is right and there is nothing behind it.**

That is why the verdict is `deklariert` and not `fehlt`: the hard half -- calling it before the
trigger, which cost the predecessor four rounds -- is already correct.

## The choice

**The grammar is parsed in the GENERATOR and never at run time.** `agiru::Declare` already resolves
an `InitValue` member name to its ordinal where the enumeration is in scope
(`include/meta/TableDef.h:86`); a relation resolves the same way, into `constexpr` data:

- the target `TableId` and target `FieldNo`, both strong types;
- a span of filter terms, each `{ destination FieldNo, CONST value | FIELD source FieldNo }`;
- for the conditional form, a span of `{ condition terms, target }` alternatives read in order.

**Not the alternative**: keeping the string and parsing it in `CheckRelation`. 40 221 declarations
parsed per validate is the predecessor's descriptor dictionary again, and this tree left Python to
stop doing that. A relation that names a table nobody declared is then a translation error rather
than a run-time lookup that finds nothing.

**Where the grammar goes**: `src/al`, beside the filter language board:0018 already needs a parser
for. `WHERE(...)`'s `CONST` and `FIELD` terms are the same shape as a filter's, and two parsers for
one syntax is one too many.

## Ordering

Before board:0043's check, because the check has nothing to check without it. Behind board:0018 only
if the two parsers are shared; if not, this one is independent and first on population.

## Gate, and its negative control

A field with `TableRelation = Customer."No."` refuses a value no `Customer` carries and accepts one
it does. A field with the conditional form follows the discriminating field: the same value is
accepted when `Type = Customer` and refused when `Type = Item`.

**The negative control is the conditional half** -- an implementation that reads the first branch and
ignores the `else if` passes the simple gate and silently validates against the wrong table.
