Type:     task
Status:   open
Parent:   0047
Area:     al, gen, rt, db
Source:   developer/properties/devenv-calcformula-property.md
Verdict:  fehlt
Class:    activation

# A `CalcFormula` is a grammar with seven verbs, and the generator parses it

```
CalcFormula =
[-]Exist(<DestinationTable> [WHERE (<TableFilters>)]) |
Count(<DestinationTable> [WHERE (<TableFilters>)]) |
[-]Sum(<DestinationTable>.<DestinationFieldName> [WHERE(<TableFilters>)]) |
[-]Average(<DestinationTable>.<DestinationFieldName> [WHERE(<TableFilters>)]) |
Min(...) | Max(...) | Lookup(...)
<TableFilter> ::= <DestinationFieldName> =
  CONST(<FieldConst>) | FILTER(<Filter>) | FIELD(<SourceFieldName>) |
  FIELD(UPPERLIMIT(<SourceFieldName>)) | FIELD(FILTER(<SourceFieldName>)) |
  FIELD(UPPERLIMIT(FILTER(<SourceFieldName>)))
```

**Seven verbs and six filter term shapes**, and the leading `-` that negates `Exist`, `Sum` and
`Average` is part of the grammar rather than an operator.

The term shapes are where the work is, and they are not interchangeable:

| shape | reads |
|---|---|
| `CONST(x)` | a literal |
| `FILTER(expr)` | a filter expression -- board:0018's language, verbatim |
| `FIELD(f)` | this record's value of `f` |
| `FIELD(FILTER(f))` | this record's FLOWFILTER `f`, as a filter |
| `FIELD(UPPERLIMIT(f))` | the upper bound of `f`'s range only |
| `FIELD(UPPERLIMIT(FILTER(f)))` | the upper bound of the flowfilter's range |

`UPPERLIMIT` is what turns a "balance in period" flowfilter into a "balance at date" -- it takes the
end of the range and drops the start. A FlowField that ignored it would compute the period's movement
where BC computes the running balance, which is the single most common wrong number in an ERP.

And one constraint on the target, stated in the page's own table: **"A key for the other table must
exist and include the fields used in the filters."** That is checkable at translation time against
the target's declared keys, and it is board:0045's "a `SetCurrentKey` onto a key with no index is a
sort of the table" applied to a FlowField.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CalcFormula =`: **8 761 declarations**, and the verb distribution decides what is built first:

| verb | count |
|---|---:|
| `Sum` | 3 967 |
| `Lookup` | 2 125 |
| `Count` | 1 378 |
| `Exist` | 1 026 |
| `Max` | 143 |
| `Min` | 111 |
| `Average` | 9 |

`Sum`, `Lookup`, `Count` and `Exist` are 8 496 of 8 761 -- **97 %**. `Average` is nine call sites in
the whole BaseApp.

## The IST-state

Nothing. `FieldDef` has no formula member, the generator drops the property, and board:0047 has no
input.

## The choice

**Parsed in the generator into `constexpr` data**, like board:0331's relation and for the same
reason: 8 761 formulas re-parsed per calculation is the predecessor's run-time descriptor tree.
The emitted shape is `{ verb, target TableId, target FieldNo, span of terms }`, each term
`{ destination FieldNo, kind, source FieldNo or literal }`.

**One SQL statement per formula, and the aggregate is SQL's.** `Sum` is `SELECT SUM(col) WHERE ...`
and never a read loop -- at board:0045's 100 million rows a loop is the process.

**Build the four verbs that are 97 % first**, and refuse the other three loudly until they are built.
A `Max` that silently returned 0 would be a wrong number on a document; a refusal is a message.

**The filter half is board:0018's parser and not a second one.** `FILTER(10|20..30)` inside a
`CalcFormula` is the same language as a `SetFilter` argument.

## Ordering

Behind board:0339, which says which fields have a formula. Behind board:0018 for `FILTER` terms.
Ahead of board:0335, whose drill-down opens the rows this computed.

## Gate, and its negative control

A `Sum` FlowField over a filtered set equals the same `SUM` from `psql`; a `Lookup` returns the
target field of the one matching row; `FIELD(UPPERLIMIT(FILTER(f)))` with `f` set to `01012026..
31012026` computes over everything up to 31 January and not over January alone.

**The negative control is `UPPERLIMIT`** -- an implementation that treats it as a plain `FIELD(FILTER)`
passes every gate whose flowfilter has no lower bound, which is most of them.
