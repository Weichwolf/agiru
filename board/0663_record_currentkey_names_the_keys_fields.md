# `Record.CurrentKey` names the key's fields

**Finding (2026-09-10).** `Record.CurrentKey` was a declared refusal; `Type Helper.GetKeyAsString`
calls it to feed `SortRecordRef`, and 31 UT cases (`Payment Export Validation UT` most of them)
stopped there.

**Reference.** `record-currentkey-method.md`: the current key as text. The platform renders the
key's fields by name, comma-separated, and `SetView('SORTING(...)')` reads that form back.

**Choice.** `RuntimeCurrentKey`: the `SetCurrentKey` fields when one ran, the primary key
otherwise, joined by `,`. Gate `CurrentKeyNamesTheKeysFields`. Silent refusal turned into an
answer.
