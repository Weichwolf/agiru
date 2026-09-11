Type:     task
Status:   active
Area:     gen, rt
Source:   developer/properties/devenv-datacaptionfields-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# 0709 A TestPage `Caption` is the page caption and its data caption

**The finding (2026-09-12, run 119).** `TestPage.Caption()` answered the `Caption` property and
nothing else. `ERM General Journal UT`'s four `GenJnlApply*` cases read the caption of `Apply
Customer Entries` and expect the applying customer's number and name in it; `Payment Registration
UT`'s two caption cases expect the balancing account type and number from the page's
`DataCaptionExpression = BalAccCaption`. Both are the title bar's data caption.

**Reference.** `devenv-datacaptionfields-property.md`: a CARD takes the table's
`DataCaptionFields` (the primary key when the table declares none) from the current record; a
TABULAR page shows a data caption only when a filter on a listed field fixes a single value,
through the field's `TableRelation` to the related table's `DataCaptionFields`, or the value
itself without one. `devenv-datacaptionexpression-property.md`: an expression evaluated at run
time replaces that. The shape of the whole caption is read off the tests that compare it whole:
`ERM Sales/Purchase Application` expects `'%1 - %2-%3'` of `Sales Journals`, batch name and
description -- `<Caption> - <data caption>`. `ServiceItemTracking` expects `'View - Posted
Service Shipments'` of a list a factbox drill-down RAN, which is the classic client's mode prefix
on a page opened by code; no UT case asks for it and it is not composed here (the tests outside
the milestone that compare it whole are eight, all in that one codeunit).

**The shape.** The generator synthesises `OnDataCaptionExpression(): Text` from the property
(`SynthesizeDataCaption`, 326 pages carry one, 84 distinct expression shapes); the runtime's
`PageDataCaption` composes the field form from the page's `PageType`, the table's
`dataCaptionFields` and the record's filters (`SingleFilterValue`, `ResolveRelation`, a
`RecordRef` over the related table). `TestPage::Caption()` joins caption and data caption with
` - `; `CurrPage.Caption(...)` set at run time replaces the caption half.

**The population.** 306 pages declare `DataCaptionExpression`, 385 `DataCaptionFields`, 231
tables `DataCaptionFields`; 6 UT cases read the result today.
