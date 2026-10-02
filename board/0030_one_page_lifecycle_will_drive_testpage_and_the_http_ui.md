# 0030 — One page lifecycle will drive TestPage and the HTTP UI

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Page generation, navigation and TestPage exist in src/gen/PageWriter.cpp and src/rt/{TestPage,PageRecord,Navigate}. No test/ui directory or HTTP server is currently present. Metadata carriage does not implement visibility, editing or save behaviour.

## Implementation for Sol

1. Finish one explicit page state machine for open/new/edit/validate/row-leave/close. Respect source views, temporary sources, default Boolean trigger results, page events and table-before-control validation.
2. Implement header save before part entry, SetRecords sharing, delayed insert, Update(SaveRecord) and subpage links against the same record state. Keep OnAfterGetRecord separate from OnAfterGetCurrRecord.
3. Build the htmx HTTP renderer over ControlDef/PageDef with server-held session state. Cover cards, lists, documents, role centres/cues, lookup/drilldown, Tell Me, dialogs and page actions. Enforce permissions and editability on the server.
4. Evaluate dynamic properties and inherited container restrictions once per relevant lifecycle event. Include captions/translations, views, actionrefs, customizations, FactBoxes, keyboard navigation and supported page kinds. Add API/control-add-in adapters after the shared lifecycle works.

## Acceptance

Every UT TestPage case runs a second time through actual HTTP and yields the same messages, rows and values. Add lifecycle-order traces, denied-edit requests, two independent sessions and header/part save controls. Activation requires a full UT A/B.

## References

Platform: devenv-testing-pages.md, page/field/action triggers and page property pages. AL: page declarations plus UT TestPage users. Predecessor: WI-1169/1170, WI-1235 (part SetRecords), WI-1401 (Update), WI-1411 (discount recalculation).

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `abouttext`, `abouttitle`, `additionalsearchterms`, `allowedfileextensions`, `allowincustomizations`, `allowmultiplefiles`, `analysismodeenabled`, `applicationarea`, `assistedit`, `cardpageid`, `clearviews`, `columnspan`, `contextsensitivehelppage`, `cuegrouplayout`, `customizations`, `datacaptionexpression`, `datacaptionfields`, `delayedinsert`, `deleteallowed`, `drilldown`, `drilldownpageid`, `editable`, `ellipsis`, `enabled`, `enabled-profile`, `entitycaption`, `entityname`, `entitysetcaption`, `entitysetname`, `extendeddatatype`, `fileuploadaction`, `fileuploadrowaction`, `filters`, `freezecolumn`, `gesture`, `gridlayout`, `groupname`, `helplink`, `hidevalue`, `image`, `images`, `importance`, `indentationcolumn`, `indentationcontrols`, `infooterbar`, `insertallowed`, `instructionaltext`, `isheader`, `linksallowed`, `lookup`, `lookuppageid`, `masktype`, `modifyallowed`, `multiline`, `multiplicity`, `pagetype`, `pasteisvalid`, `populateallfields`, `profiledescription`, `promoted`, `promoted-action`, `promoted-profile`, `promotedactioncategories`, `promotedcategory`, `promotedisbig`, `promotedonly`, `quickentry`, `refreshonactivate`, `rolecenter`, `rowspan`, `runobject`, `runpagelink`, `runpagemode`, `runpageonrec`, `runpageview`, `savevalues`, `scope-action`, `shortcutkey`, `showas`, `showastree`, `showcaption`, `showfilter`, `showmandatory`, `sourcetable`, `sourcetabletemporary`, `style`, `styleexpr`, `subpagelink`, `subpageview`, `tooltip`, `treeinitialstate`, `updatepropagation`, `usagecategory`, `visible`, `width`.
