# A conditional or filtered `TableRelation` is evaluated at run time

**Finding (2026-09-10).** `Price List Line."Unit of Measure Code Lookup"` declares
`TableRelation = if ("Asset Type" = const(Item)) "Item Unit of Measure".Code where("Item No." =
field("Asset No.")) else if (...) "Resource Unit of Measure".Code where(...) else "Unit of
Measure"`. The generator keeps `relationTable` / `relationField` only for a SIMPLE relation
(`TableWriter.cpp`, "simple" = no `where`, `if`, `else`), so a conditional one carries nothing:
`CheckRelation` skips it, and `TestField.Lookup()` (board:0657) cannot open the related page.

**Reference.** `devenv-tablerelation-property.md`: the condition is evaluated on the record's
current values, the `where` clause filters the related table by `const`, `field` and `filter`
terms -- the same three `SubPageLink` uses, which `src/rt/SubPageLink.cpp` already parses.

**Choice.** The generator carries the whole property text on the `FieldDef` (`relation`,
whitespace collapsed), and `detail::ResolveRelation` (`src/rt/Relation.cpp`, door
`runtime/Relation.h`) reads it against the record: branches `if (conditions) target else ...` in
order, a condition `Field = const(V)` / `filter(F)` matched with the filter language, a target
`Table[.Field] [where(...)]` whose `field(X)` / `const(V)` / `filter(F)` terms become filter texts
for the related table. `CheckRelation` and the test page's `Lookup` share it; a filtered check
narrows a probe record of the related table and asks `IsEmpty`. Gate `RelationGate` over the
target image, whose `Code` carries the real conditional declaration. board:0043 asked for a
`constexpr` parsed form; the string is what that form would be parsed from, and the filter
language inside it is parsed at run time everywhere else in this tree, so the shape is not a
new one. A relation to a table this build does not carry still passes silently (board:0043).
