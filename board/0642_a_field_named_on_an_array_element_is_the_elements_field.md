# A field named on an array element is the element's field

**Finding (2026-09-09).** `CurrencyExchRate2[CacheNo].SetRange("Currency Code", CurrencyCode)` in
table 330 is generated as `At(Var_Block->CurrencyExchRate2, CacheNo).SetRange(this->CurrencyCode,
CurrencyCode)`, and the next line's `"Starting Date"` as a bare `StartingDate`: the field
designator is taken from the enclosing record (`this`) while the receiver is an array element.
The runtime measures a member's field number by its offset from the receiver, so the offset is
the distance between two records and the call refuses with "declares no field at byte
18446744073709281496". Every UT case that reaches an exchange rate through `FindCurrency` stops
there (Currency UT, ERM General Journal UT; 10 cases). 19 sites in the tree have the shape:
table 330, `SalesLine[i].Validate(Quantity, ...)`, `Contacts[i].Validate(Name, ...)`,
`GLBudgetEntry[i].Validate(Amount, ...)`.

**Reference.** AL resolves a bare field name in a field-designating argument against the
RECEIVER of the call (`record-setrange-method.md`, `record-validate-method.md`: "the field of the
record"), never against the enclosing table, and a parameter of the same name does not shadow it
there.

**Choice.** `src/gen/BodyWriter.cpp`: when the receiver is an `Index` expression whose base is a
record variable that has the named field, the designator is spelled on the receiver, as it already
is for a receiver that is a plain name. Gate case in `GenCodeunitGate`. Silent-wrong-data: the
old spelling never ran a right answer.
