# A record variable carries its InitValues before Init

**Finding (2026-09-10).** With `Item Jnl.-Post Line` in the slice, item posting stops at
`Round(UnitCost / Qty, Currency."Unit-Amount Rounding Precision")` with "Round: precision is
zero" in 86 UT cases: the `Currency` global is never loaded when there is no additional
reporting currency, and a generated member is `Decimal UnitAmountRoundingPrecision{}`, zero.
The BaseApp relies on the field's `InitValue = 0.00001` being there on a fresh variable.

**Reference.** `devenv-initvalue-property.md`: the value a field holds when a record is created;
the platform gives a declared record variable its InitValues without `Init` -- which is what the
shipped posting code assumes. 960 fields declare one (Option 286, Boolean 245, Decimal 164,
Enum 93, Code 70, Integer 57, Text 29, Date 9, DateFormula 4, Time 3).

**Choice.** `src/gen/TableWriter.cpp` emits a constructor for a table with any `InitValue`,
defined after its traits, calling `detail::RuntimeInitValues` (the init-value half of `Init`)
so the member values need no per-type spelling. Gate case in `GenTableGate`. Silent-wrong-data.
