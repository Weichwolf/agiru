Type:     epic
Status:   open
Area:     rt, gen
Source:   developer/triggers-auto/ (151 pages)
Tags:     surface, counted

# Every documented trigger is fired, declared or refused, and the count may only rise

`triggers-auto/` is one page per trigger per object kind. `test/triggers.py` counts the triggers
the tree NAMES anywhere in `src/rt`, `src/gen` and `include` -- fired, declared or refused -- and
`test/trigger-baseline` holds the count: **27 of 151 on 2026-09-06**, and it may only rise.

## Measured 2026-09-06

    page                          4 named / 23 absent   OnAfterActionEvent, OnAfterGetCurrRecordEvent, OnAfterGetRecordEvent, OnBeforeActionEvent, OnClosePageEvent, OnDeleteRecordEvent, OnInsertRecordEvent, OnModifyRecordEvent, OnNewRecordEvent, OnOpenPageEvent, OnQueryClosePageEvent, OnAfterGetCurrRecord, OnAfterGetRecord, OnClosePage, OnDeleteRecord, OnFindRecord, OnInit, OnInsertRecord, OnModifyRecord, OnNewRecord, OnNextRecord, OnOpenPage, OnQueryClosePage
    table                        11 named /  3 absent   OnAfterRenameEvent, OnBeforeRenameEvent, OnRename
    requestpage                   0 named / 12 absent   OnAfterGetCurrRecord, OnAfterGetRecord, OnClosePage, OnDeleteRecord, OnFindRecord, OnInit, OnInsertRecord, OnModifyRecord, OnNewRecord, OnNextRecord, OnOpenPage, OnQueryClosePage
    tableextension                3 named /  9 absent   OnAfterDelete, OnAfterInsert, OnAfterModify, OnAfterRename, OnBeforeDelete, OnBeforeInsert, OnBeforeModify, OnBeforeRename, OnRename
    codeunit                      3 named /  8 absent   OnCheckPreconditionsPerCompany, OnCheckPreconditionsPerDatabase, OnInstallAppPerCompany, OnInstallAppPerDatabase, OnUpgradePerCompany, OnUpgradePerDatabase, OnValidateUpgradePerCompany, OnValidateUpgradePerDatabase
    pageextension                 2 named /  9 absent   OnAfterGetCurrRecord, OnAfterGetRecord, OnClosePage, OnDeleteRecord, OnInsertRecord, OnModifyRecord, OnNewRecord, OnOpenPage, OnQueryClosePage
    requestpageextension          0 named /  9 absent   OnAfterGetCurrRecord, OnAfterGetRecord, OnClosePage, OnDeleteRecord, OnInsertRecord, OnModifyRecord, OnNewRecord, OnOpenPage, OnQueryClosePage
    xmlporttableelement           0 named /  7 absent   OnAfterGetRecord, OnAfterInitRecord, OnAfterInsertRecord, OnAfterModifyRecord, OnBeforeInsertRecord, OnBeforeModifyRecord, OnPreXmlItem
    pagefield                     1 named /  5 absent   OnAfterLookup, OnAssistEdit, OnControlAddIn, OnDrillDown, OnLookup
    pagefieldextension            0 named /  6 absent   OnAfterAfterLookup, OnAfterValidate, OnAssistEdit, OnBeforeValidate, OnDrillDown, OnLookup
    reportextensiondatasetmodify  0 named /  6 absent   OnAfterAfterGetRecord, OnAfterPostDataItem, OnAfterPreDataItem, OnBeforeAfterGetRecord, OnBeforePostDataItem, OnBeforePreDataItem
    report                        0 named /  4 absent   OnInitReport, OnPostReport, OnPreRendering, OnPreReport
    reportdataitem                0 named /  3 absent   OnAfterGetRecord, OnPostDataItem, OnPreDataItem
    reportextension               0 named /  3 absent   OnPostReport, OnPreRendering, OnPreReport
    xmlport                       0 named /  3 absent   OnInitXmlPort, OnPostXmlPort, OnPreXmlPort
    actionextension               0 named /  2 absent   OnAfterAction, OnBeforeAction
    field                         1 named /  1 absent   OnLookup
    fieldextension                0 named /  2 absent   OnAfterValidate, OnBeforeValidate
    xmlportfieldattribute         0 named /  2 absent   OnAfterAssignField, OnBeforePassField
    xmlportfieldelement           0 named /  2 absent   OnAfterAssignField, OnBeforePassField
    xmlporttextattribute          0 named /  2 absent   OnAfterAssignVariable, OnBeforePassVariable
    xmlporttextelement            0 named /  2 absent   OnAfterAssignVariable, OnBeforePassVariable
    action                        1 named /  0 absent
    fileuploadaction              1 named /  0 absent
    query                         0 named /  1 absent   OnBeforeOpen
    triggers named 27 of 151 (baseline 27)

## What the table says

- **Table**: 11 of 14; the three absent are `OnRename`, `OnBeforeRenameEvent`,
  `OnAfterRenameEvent` -- board:0231's, and board:0057 phase 2 for the events.
- **Page**: 4 of 27 -- the 23 absent are the page EVENTS (`OnAfterGetRecordEvent`,
  `OnBeforeActionEvent`, ...): the platform raises them around the page triggers, the same shape
  as the table events, and board:0057 phase 3 is that.
- **Codeunit**: 3 of 11 -- the 8 absent are the install/upgrade triggers (board:0274, 0275).
- **Table/page/field extensions**: an extension's triggers run beside the base object's; the
  merge carries the procedures and not yet the triggers.
- **Request page, report, xmlport, query**: 0 -- no generator (board:0063, 0064, 0065).

## The rule this carries

A kind whose generator exists fires every trigger its pages list, and the count in the baseline
is the proof; a kind without a generator is a hole with a count (board:0034).
