Type:     task
Status:   active
Area:     rt
Source:   business-central/teams-faq.md
Verdict:  fehlt
Class:    activation

# 0708 A TestPage `SetValue` and `Invoke` are client round trips that save and re-read

**The finding (2026-09-12, run 119 at 1 883 of 2 296).** Three ranked shapes share one root: the
page keeps its copy of the record across what the client would treat as a round trip.

- `Control 'VAT Return No.'` expected `GL…`, actual blank (6 cases, VAT Return Period UT): the
  card's "Create VAT Return" action writes the period's return number through
  `VATReportMgt.CreateVATReturnFromVATPeriod`'s own record variable and `Modify`; the page's
  `Rec` still shows the value it read before the action.
- `Control 'Enabled'` expected false, actual `Yes` (6 cases, Workflow Engine UT): `asserterror
  WorkflowCard.Enabled.SetValue(true)` fails validation, and the record keeps the value the failed
  validation wrote -- the client discards a failed edit and shows the last valid value.
- The API aggregate discount cases (12 + 5 + 3 + 4, ERM Sales Invoice Aggregate / Cr. Memo Aggr.
  / Purch. Cr. Memo Aggr. UT): the header's discount fields are read by the subform through a
  record variable of its own, so a header change that is not SAVED is invisible to the lines.

**Reference.** The user documentation states the client's rule: "automatically saves changes you
make to any field as soon as you leave the field" (`business-central/teams-faq.md`, also
`admin-teams-troubleshooting.md`). `triggers-auto/page/devenv-onmodifyrecord-page-trigger.md`:
the page's `OnModifyRecord` runs before the platform modifies, and `false` means the trigger did
the write itself (the BaseApp's 73 bodies say `exit(false)` after `CODEUNIT.Run("G/L Entry-Edit",
Rec)` and `exit(true)` where the platform is to write). The predecessor's WI-1113 measured the
same round trip at +17 (542 -> 559 over 811 ids, LOST 0): re-read before the input, parts
refreshed after it; it left the save to the leave of the row.

**The shape.** `TestPage<P>::SetControlText` on a positioned, existing record: validate as before,
then `OnModifyRecord` and `Modify(true)`; on a validation error the record is put back to the
copy taken before the input (`Record_() = before`) and the error is rethrown coded
`TestValidation`. `RunControlTrigger(Action)`: after the trigger, `Find("=")` on the page's record
and the after-get triggers again. Parts already re-read and re-run their after-get triggers on
every use (`Relink_`), which is the predecessor's second half.

**Class: activation.** `Modify(true)` after every `SetValue` on an existing record runs every
table's `OnModify` where nothing ran before; the A/B is over the whole suite (run 121 against run
120), and on a loss the list names the deeper roots and the save is split from the re-read.

**Measured (run 121 against run 120, 2026-09-12):** 1 909 -> 1 926 of 2 296, GAINED 29, LOST 12.
The round trip, the page events (`OnOpenPageEvent`, `OnAfterGetCurrRecordEvent` and the rest),
the caption, `-Exist`, `Editable = false` and the FlowField calculation landed together. Of the
12 lost, 9 are the FlowField calculation at landing reaching two formula gaps it had never reached
-- `const(Database::X)` bound as words, and a term over the target's own FlowField read as a
column -- and 1 (`TestDeletingAllLinesUpdatesTotalsDiscountPct`) passes when the codeunit runs
alone (a "Shipment Date before work date" message keyed on the date the run crossed); both gaps
are fixed in the round that follows, with gate cases. Kept.
