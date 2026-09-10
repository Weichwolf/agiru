Type: root
State: open
Area: rt
Tags: semantics

# A filter on a FlowField is a correlated subquery

**Finding (2026-09-10, chain 93).** Five UT cases end in PostgreSQL's `column "Template Type" does
not exist` (Phys. Invt. Order Subform 3, Non-Deductible UT, SCM - Planning UT): `Item Journal
Batch."Template Type"` is a FlowField (`lookup("Item Journal Template".Type where(Name =
field("Journal Template Name")))`), and `SetRange("Template Type", ...)` reaches the SELECT as a
plain column. `SetRange(Recurring, ...)` -- the same shape on the journal batches -- has 36
BaseApp call sites, so the population is far larger than five cases.

**Reference.** `devenv-flowfields.md`: a FlowField can be filtered like any field; the platform
evaluates the CalcFormula per row. The runtime already has the pieces: `FormulaReader` and
`PredicateOf` (`src/rt/Table.cpp`) build the lookup/aggregate SQL for `CalcFields`, and
`detail::Where(def, expr, first, column)` (`src/rt/Where.cpp`) takes a column EXPRESSION.

**Choice.** In `Selection::Narrow`, a FlowField filter becomes `Where(def, expr, first, column)`
with `column` a scalar subquery over the target table, aliased, whose `field(...)` terms are
correlated to the outer row (`"Item Journal Template"."Name" = "Item Journal Batch"."Journal
Template Name"`) rather than bound from a record. Lookup: `(SELECT col FROM target t WHERE ...
ORDER BY pk LIMIT 1)`; Sum/Count/Min/Max/Average: the aggregate; Exist: `EXISTS(...)`.
`FlowFilter` terms of the outer record stay binds, as `CalcFields` binds them.

**Gate.** A hand-built table pair with a lookup FlowField: the rendered WHERE text, and a
negative control on a FlowField whose formula names a table the catalogue lacks (a refusal
naming the field, never a silent drop).
