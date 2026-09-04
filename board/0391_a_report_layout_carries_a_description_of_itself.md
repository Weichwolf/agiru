Type:     task
Status:   open
Parent:   0063
Area:     gen
Source:   developer/properties/devenv-summary-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A report layout carries a description of itself

> **Version**: runtime 9.0. Applies to: **Report Layout.**
>
> Sets the string that is used to provide a **detailed description of this layout**.

A `report` may declare several `rendering` layouts -- an RDLC for print, a Word for letters, an Excel
for analysis -- and the user picks one at run time. `Summary` is the sentence shown beside the
layout's name in that picker, so it is the only thing distinguishing two layouts whose captions are
both "Sales Invoice".

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Summary =` **855** · `SummaryML =` **0** (board:0386).

855 layouts carry a description, which places it beside the 668 reports CLAUDE.md counts as in scope.

## The IST-state

Reports have no generator (board:0063, board:0034), so no layout object exists to carry it.

## The choice

A `string_view` on the layout descriptor, `constexpr`, beside its caption and its file name. It is
the cheapest possible item and it is filed because a layout descriptor built without it would have to
be widened later across 668 reports.

## Ordering

Inside board:0063's report generator, with the layout descriptor.

## Gate, and its negative control

A report with two layouts shows each layout's own summary in the picker.

**The negative control is the second layout** -- a picker that shows one summary for the report rather
than one per layout looks correct with a single-layout report and is wrong on every multi-layout one.
