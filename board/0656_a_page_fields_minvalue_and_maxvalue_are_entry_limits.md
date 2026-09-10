# A page field's `MinValue` / `MaxValue` are entry limits the test page enforces

**Finding (2026-09-10).** Once a control over a page variable could take a value (chain 81),
`asserterror DocumentSearch.AmountTolerance.SetValue(150)` observed nothing: the control
declares `MinValue = 0; MaxValue = 100;`, the generator carries both on the `ControlDef` and the
`FieldDef`, and nothing read them (2 cases of `Payment Registration UT`, green before only
because the refusal it observed was "shows no field to set").

**Reference.** `devenv-minvalue-property.md`: checked "only if the field or control value is
updated through the UI ... If a field is updated through application code, then the MinValue
property is not validated." The predecessor learnt it the expensive way (openerp WI-801: the
check ran on every `Rec.Validate`, and `CashFlowSetup.Validate(Horizon, RandInt(10))` with
`MinValue = 3` would have made Microsoft's own suite red one run in five); its wording is the
platform's, "The value must be greater than or equal to 0. Value: -1."

**Choice.** `detail::CheckEntryRange` (`src/rt/Table.cpp`, gate `EntryRangeGate`) is called from
the test page's `SetValue` -- for a field control against the field's declaration and the
control's, for a variable control against the control's -- and never from `Validate`. Coded
`TestValidation`. The runtime does not decide between the control's bound and the field's when
both are declared: each is checked and the tighter one refuses first.
