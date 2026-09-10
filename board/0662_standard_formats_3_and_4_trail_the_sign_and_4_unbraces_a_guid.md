# Standard formats 3 and 4 trail the sign, and 4 renders a GUID without braces

**Finding (2026-09-10).** `Format(Company.Id, 0, 4)` (OData, workflow webhooks) was refused with
"standard format 4 is not one devenv-format-property.md tabulates" -- 6 UT cases of ERM General
Journal UT through the job-queue posting path.

**Reference.** `system-format-joker-integer-integer-method.md`: `<Integer Thousand><Decimals>
<Sign,1>` is format 3 and `<Integer><Decimals><Sign,1>` is format 4. For a GUID the platform's
format 4 is the value without braces, which is what every caller lower-cases into a JSON id.

**Choice.** `Rendered` in `BuiltinsWritten.cpp` puts the sign last for numbers under 3 and 4 and
strips a GUID's braces under 4; the guard accepts both. Thousand separators are not rendered for
3, since no format here renders them yet. Gate `FormatsThreeAndFourTrailTheSignAndFourUnbracesAGuid`.
