# A temporary row carries no variable block

**Finding (2026-09-09).** `Sales Line` initialises its global `Currency` in `CalcVATAmountLines`
and then walks its own temporary rows; each `Next` loaded the row by assigning the WHOLE object,
so the record's `Var_Block` (its AL globals, held by `Instance`) was replaced by the stored
row's empty one, and `VAT Amount Line.UpdateLines` rounded to `Currency."Amount Rounding
Precision"` = 0: "Round: precision is zero", 44 UT cases across ERM Document Totals UT and the
three aggregate codeunits. The database load copies fields only and never had the defect; a
probe over the generated tree showed the temporary load losing the block and the database load
keeping it. `Insert` also cloned the record's whole block INTO every stored row.

**Reference.** A table's global variables belong to the record VARIABLE (`devenv-table-object.md`:
variables declared in the table are per instance), and a row is fields only.

**Choice.** `kTempOps` in `include/runtime/Table.h`: `load` keeps the variable's block the way it
keeps its state, `insert` and `replace` store a copy with an empty block, through a
`HasVariableBlock` concept over the generated member. A gate case needs a target table with a
variable block, which `test/target` does not carry yet; the proof is the milestone and the probe.
