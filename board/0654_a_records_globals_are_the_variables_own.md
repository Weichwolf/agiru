# A record's globals are the variable's own, and `Rec := Other` never copies them

**Finding (2026-09-10).** `Currency Exchange Rate.FindCurrency` loads a rate into an element of
its global `CurrencyExchRate2` array and then does `Rec := CurrencyExchRate2[CacheNo]`. The
generated assignment copied every member, the `Var_Block` handle among them, so the variable's
own globals -- the array holding the rate it had just found -- were replaced by the element's
empty block. The cache then said "use it" and `TestField("Exchange Rate Amount")` failed with a
blank key: 8 cases of `Inc Doc Attachment Overview UT`, and the same shape wherever a table's
procedure assigns `Rec` from one of its own record globals. The temporary store's `load` had
already worked around it by hand (board history: the VAT rounding precision of zero, 44 cases).

**Reference.** `Instance` in `runtime/Codeunit.h` promised it: "AL copies a record's FIELDS; its
object variables are the copy's own and are made on ITS first use." The predecessor kept a
record's globals on the Python object and never copied them at all.

**Choice.** `Var_Block` is a `Globals<Variables>`: assignment keeps what this variable holds, a
copy starts unmade (`InstanceGate::AssigningKeepsTheVariablesOwnGlobals`), and the temporary
store's special cases are gone. Silent-wrong-data.
