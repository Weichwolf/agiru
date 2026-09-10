# A text position in a Variant is its Char, and a Char formats as itself

**Finding (2026-09-10).** `Format(GLNValue[ExpectedSize])` in the GLN check digit handed a
`CharAt` position to `Format`'s `Any`, and the Variant's catch-all constructor held nothing:
"the Variant does not hold that type (alternative 0)" on every company-information setup that
validates a GLN (18 UT cases of Incoming Doc. To Data Exch.UT). Beside it, `Variant(Char)` held
the character's NUMBER, so `Format` of a Char would have rendered `55` for `7`.

**Reference.** `char-data-type.md`: a Char formats as the character; `Format(Any)`.

**Choice.** A `CharAt` (tagged `IsATextPosition`) constructs a Variant through its `Char`, and a
Char in a Variant is held as its text -- what `Format`, `StrSubstNo` and a comparison with a
formatted number read. A Char handed to an Integer through a Variant reads the text's number,
which no BaseApp call site does (a Char to Integer is a direct conversion, not a Variant).
Gate `ATextPositionHoldsItsChar`.
