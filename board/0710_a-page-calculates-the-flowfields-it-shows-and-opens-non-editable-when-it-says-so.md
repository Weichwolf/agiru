Type:     task
Status:   active
Area:     rt
Source:   developer/properties/devenv-calcformula-property.md
Verdict:  teilweise
Class:    silent-wrong-data

# 0710 A page calculates the FlowFields it shows, and opens non-editable when it says so

**Three findings of one sweep (2026-09-12, run 119).**

1. `-Exist` reversed nothing. `[-]Exist(...)` is in the syntax block of
   `devenv-calcformula-property.md`; `Sales Invoice Header.Closed` is
   `-exist("Cust. Ledger Entry" where("Entry No." = field(...), Open = filter(true)))`, true when
   NO open entry is left, and `src/rt/Table.cpp` rendered it as a plain `EXISTS` -- so every
   posted invoice with its open entry read as paid (`ERM Sales Invoice Aggregate UT`
   `TestUpdateAggregateTable` and four more: `Expected:<Open> Actual:<Paid>`). Five BaseApp
   formulas carry the sign. Fixed in both the calculation and the correlated filter clause, with
   a `FilterGate` case and its negative control.
2. A page's FlowField controls were never calculated. `record-calcfields-method.md`: a FlowField
   is zero until calculated, "or a page control whose source is the field itself" -- the platform
   calculates what the page shows before `OnAfterGetRecord`. `detail::AfterGetRecord` now walks
   the page's layout and calculates each FlowField control (`CalcShownFlowFields`); a formula
   over a table outside this build is left alone (`CalcFieldIfCarried`) rather than refusing the
   landing, because the page shows the field whether or not a test reads it.
3. `Editable = false` on the page was ignored: `VAT Return Period Card` declares it, and
   `TestPage.Editable()`, every control's `Editable()` and `CurrPage.Editable` answered true after
   `OpenEdit` (5 cases, VAT Return Period UT `UI_FieldsAndActionsVisibility_*`).
   `devenv-editable-property.md`: the page cannot be edited. `detail::OpenPage` folds the
   property into what `OpenedAs` records.

**Class: silent-wrong-data** for all three: a value read wrong, no path that was dead. Measured
in the same run as board:0708, whose activation is the one that needs the A/B.
