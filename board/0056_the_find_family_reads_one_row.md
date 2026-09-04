Type: root
State: open
Area: rt

# `Find`, `FindFirst` and `FindLast` read one row, and a discarded one raises

`FindSet` works and the whole rest of the find family refuses. Counted 2026-09-04 -- the sites are
`.Method(` over `Layers/W1`, the UT column over the 78 codeunits of the milestone:

| refuses | UT codeunits | UT sites | BaseApp sites |
|---|---|---|---|
| **`Init`** | 53 of 78 | 454 | 3 623 |
| **`FindFirst`** | 52 | 378 | 4 078 |
| **`Find`** | 28 | 164 | 4 559 |
| **`DeleteAll(Boolean)`** | 30 | 206 | 3 296 |
| **`FindLast`** | 33 | 123 | 1 168 |
| `TableCaption` | 10 | 29 | 2 403 |
| `CalcFields` | 14 | 49 | 1 597 (board:0047) |
| `GetFilter` | 4 | 8 | 2 131 |
| `Rename` | 16 | 72 | 120 |

58 `Record` methods refuse in `include/runtime/Table.h` in all. **These five are the ones the
milestone stands on**, and three of them are one mechanism.

## What the platform documents

`record-find-method.md` gives `Which` completely, and there is nothing to infer:

| `Which` | finds |
|---|---|
| `=` (the default) | the row equal to the key values |
| `>` | the first row larger than them |
| `<` | the last row smaller than them |
| `-` | the FIRST row of the filtered set -- alone only |
| `+` | the LAST row -- alone only |

`=`, `<` and `>` may be combined (`>=`), each character at most once, and with any of them "you must
assign value to all fields of the current and primary keys before you call FIND". The search path is
the current key, and where that key does not separate two rows the primary key decides -- so the
ORDER BY is the current key followed by the primary key, always, and that is not an optimisation.

**THE OMITTED RETURN VALUE RAISES, AND THE PAGE SAYS SO:** "If you omit this optional return value
and the operation does not execute successfully, a runtime error will occur." That is CLAUDE.md's
`value context` trap in the platform's own words, and it decides the shape below.

## The wording, from BC's own tests

| when | text |
|---|---|
| a discarded `Find`/`FindFirst`/`FindLast` finds nothing | `There is no <Table Caption> within the filter.`, and with filters set `There is no <Table> within the filter: <filters>` |
| a discarded `Get` finds nothing | `The <Table Caption> does not exist. Identification fields and values: <Field>=<Value>,...` |
| `Insert` over an existing key | `The <Table Caption> already exists. Identification fields and values: ...` |

`ERMCashFlowForecast` and `PurchaseLine` assert the first verbatim; `Dimension` and `Lot No.
Information` the second. The codes beside them are board:0055.

## The choice

- **One entry point.** `Find(Which)` is the whole family: `FindFirst()` is `Find('-')` and
  `FindLast()` is `Find('+')`, which is what the platform's own three pages describe. Writing three
  readers would be three places for the ORDER BY to disagree.
- **It reads ONE row and opens no cursor.** `FindSet` opens the set because `Next` steps it;
  `FindFirst` does not, and a runtime that made them the same would hold a cursor open for every
  single-row read in the BaseApp -- 4 078 of them.
- **`+` reverses the ORDER BY rather than reading the set.** `ORDER BY <key> DESC LIMIT 1` is one
  index seek; reading forward to the end is the table.
- **The raising form is separate from the answering one**, because the generator already knows
  which context it emits into. `bool Find(...)` answers; the discarded form calls the same reader
  and raises on false. What must NOT happen is one function guessing from a `[[nodiscard]]`.

`Init` is the other big one and it is not this item -- it is fully specified by
`record-init-method.md` (every field to its type default, EXCEPT the primary key and the timestamp)
plus `InitValue` (815 declarations under `Layers/W1`, which `FieldDef` does not carry), and
`devenv-initvalue-property.md` says `Clear` and `ClearAll` apply it too.

## The gate

A table with a two-field key and rows that straddle it: `-` finds the first in KEY order and not in
insertion order, `+` the last, `>` the row after a key that exists and after one that does not, `<`
the mirror. The negative control is a second key: setting `SetCurrentKey` to it must change which
row `-` returns, and a reader that ordered by the primary key regardless passes every other check
in this gate.
