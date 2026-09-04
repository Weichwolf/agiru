Type:     task
Status:   open
Parent:   0047
Area:     rt, db
Source:   developer/devenv-calcfields-calcsums-fielderror-fieldname-init-testfield-and-validate-methods.md
Verdict:  fehlt
Class:    activation

# `CalcSums` needs its key to be current, and both use the filters

**The second half of a page split in two** -- board:0506 takes the error methods. These two are
board:0047's and they are one subject: computing an aggregate from the record's current filter state.

> **CalcFields** "updates FlowFields. **FlowFields are AUTOMATICALLY updated when they're the DIRECT
> SOURCE EXPRESSIONS of controls, but they must be EXPLICITLY CALCULATED when they're part of a more
> complex expression.**"
>
> ```AL
> Customer.Get('01454545');
> Customer.SetRange("Date Filter", 0D, TODAY);
> Customer.CalcFields(Balance, "Balance Due");
> ```
> -- "calculates the fields **by using the current filter** and performing the calculations defined as
> the `CalcFormula` properties."
>
> **CalcSums** "calculates the sum of one or more fields that are **SumIndexFields** in the record.
> **For `CalcSums`, a key that contains the SumIndexFields MUST BE SELECTED AS THE CURRENT KEY.**
> Similar to `CalcFields`, `CalcSums` uses the current filter settings."
>
> ```AL
> custledgerentry.SetCurrentKey("Customer No.");
> custledgerentry.SetRange("Customer No.", '10000', '50000');
> custledgerentry.CalcSums("Sales (LCY)");
> ```

**Three facts, and the second is a precondition an implementation would not invent.**

**A FlowField on a control calculates itself; a FlowField in an expression does not.** So board:0030's
renderer calls `CalcFields` implicitly for bound controls and AL code must call it explicitly. Two
call sites, one mechanism -- and a runtime that calculated on every read would be correct and would
pay board:0045's row counts on every field access.

**`CalcSums` REQUIRES the SumIndexFields key to be current.** That is board:0343's property appearing
as a runtime precondition rather than a schema hint: the method is not "sum this field over the
filter", it is "read the aggregate the current key maintains". A `CalcSums` whose current key lacks the
field is an error, not a slow path.

**If agiru computes instead of maintaining an aggregate** -- board:0343's open measurement -- the
precondition still has to be ENFORCED, or code that would fail in BC silently succeeds here and the
divergence is invisible until a customer's key changes. Refusing where BC refuses is the conservative
choice and it is this item's.

**Both use the current filters**, FlowFilters included: `SetRange("Date Filter", ...)` in the example
sets a `FieldClass = FlowFilter` field (board:0339), which the `CalcFormula`'s `FIELD(FILTER(...))`
terms read (board:0340). Three items, one chain -- the filter is set on a flowfilter field, the
formula reads it, the aggregate runs.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0339: `FieldClass = FlowField` **8 772**, `FlowFilter` **1 510**. board:0340: `CalcFormula`
**8 761**. board:0343: `SumIndexFields` **762** across 3 272 keys. The method call counts belong to
board:0028's census -- **stated rather than guessed.**

## The IST-state

board:0047 records it: FlowFields are not computed. `CalcFields` and `CalcSums` are part of
board:0035's declared-and-refusing surface.

## The choice

`CalcFields` walks the named fields, runs each `CalcFormula` (board:0340) against the record's current
filter state, and writes the result into the field's storage -- which for a FlowField is a value on the
record and never a column (board:0019).

`CalcSums` **checks the current key first** and raises when it does not carry the field, then computes.
Whether the computation reads a maintained aggregate or sums the base table is board:0343's
measurement; the precondition holds either way.

## Ordering

Behind board:0340's formula parser and board:0339's `FieldClass`. With board:0343 for the key
precondition.

## Gate, and its negative control

`CalcFields(Balance)` under a `Date Filter` returns the sum over that range; changing the filter and
recalculating returns a different number; `CalcSums` without the matching current key raises.

**The negative control is the second `CalcFields`** -- a cached FlowField returns the first answer
again, and a single-call gate cannot see it. The `CalcSums` refusal is the second control, and it must
fire even where the implementation could have computed the answer anyway.
