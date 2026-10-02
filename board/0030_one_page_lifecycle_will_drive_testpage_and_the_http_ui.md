# 0030 — One page lifecycle will serve TestPage and both clients

Status: open | Priority: P1 | Stage: UT lifecycle; Clients adapter extraction | Reviewed: 2026-09-28
Depends on: 0043 validation; 0018 filters; 0019 totals; 0718 images; 0039 activation proof.

## Evidence

- `include/runtime/test/TestPage.h` owns validation, save/reread, actions, parts and mode handling; `Page.h` owns a second portion of lifecycle.
- `Page::Update(false)` returns immediately. `TestPage::SetControlText`/`RunControlTrigger` expose operations without a general editability/enabled check.
- UT failures include stale batch totals/document numbers, unbound controls, missing traps and lookup results; none proves a single common cause.

## Implementation

1. Extract a production `PageSession`/`PageDispatcher` below the TestPage adapter. Move the `PageCore` contract out of `runtime/test/`; keep test handlers/assertions outside production behavior.
2. Represent open mode, current row/key/version, dirty/new state, parent/part handles and pending dialog explicitly. Generated factories and typed control accessors supply immutable PageDef/ControlDef metadata.
   Keep submitted control text/validation errors separate from accepted typed record values; an invalid Decimal must remain editable without corrupting the AL record. Emit revisioned refresh results with stable row/control identities.
3. Trace open → validate → save → row leave → action → close against AL; distinguish row fetch from current-row change. Preserve delayed insertion, header save before part entry, SetRecords and SubPageLink.
4. Implement control refresh and UpdatePropagation with explicit invalidation. First reproduce save/discount recalculation order; do not blindly replay both after-get triggers on Update. SourceTable determines the omitted SaveRecord default.
5. Enforce mode, inherited Editable/Enabled and permissions at command execution; preserve documented TestPage handler exceptions explicitly. Unknown controls/actions refuse. Client delivery and equivalence are 0720.

## Acceptance

- Lifecycle traces cover new/edited/temporary rows, canceled close, header/part save, lookup writeback and update(false) without writing.
- Full UT population green before client construction. Any refresh activation compares every method and investigates losses in invoice-discount and physical-inventory families.

## References

Code: `include/runtime/{Page.h,test/PageCore.h,test/TestPage.h}`, `src/rt/{TestPage,PageRecord,SubPageLink}.cpp`, `src/gen/PageWriter.cpp`, `include/meta/PageDef.h`. Platform: `devenv-testing-pages.md`, `methods-auto/page/page-update-method.md`, page/field/action triggers. AL: `VATReport.Page.al`, `DocumentTotals.Codeunit.al`, `ERMGeneralJournalUT.Codeunit.al`. Predecessor: WI-1113/1235/1330/1401/1411; 1401's repeated refresh attempts lost discount cases.

Property scope: `abouttext`, `abouttitle`, `additionalsearchterms`, `allowedfileextensions`, `allowincustomizations`, `allowmultiplefiles`, `analysismodeenabled`, `applicationarea`, `assistedit`, `cardpageid`, `clearviews`, `columnspan`, `contextsensitivehelppage`, `cuegrouplayout`, `customizations`, `datacaptionexpression`, `datacaptionfields`, `delayedinsert`, `deleteallowed`, `drilldown`, `drilldownpageid`, `editable`, `ellipsis`, `enabled`, `enabled-profile`, `entitycaption`, `entityname`, `entitysetcaption`, `entitysetname`, `extendeddatatype`, `fileuploadaction`, `fileuploadrowaction`, `filters`, `freezecolumn`, `gesture`, `gridlayout`, `groupname`, `helplink`, `hidevalue`, `image`, `images`, `importance`, `indentationcolumn`, `indentationcontrols`, `infooterbar`, `insertallowed`, `instructionaltext`, `isheader`, `linksallowed`, `lookup`, `lookuppageid`, `masktype`, `modifyallowed`, `multiline`, `multiplicity`, `pagetype`, `pasteisvalid`, `populateallfields`, `profiledescription`, `promoted`, `promoted-action`, `promoted-profile`, `promotedactioncategories`, `promotedcategory`, `promotedisbig`, `promotedonly`, `quickentry`, `refreshonactivate`, `rolecenter`, `rowspan`, `runobject`, `runpagelink`, `runpagemode`, `runpageonrec`, `runpageview`, `savevalues`, `scope-action`, `shortcutkey`, `showas`, `showastree`, `showcaption`, `showfilter`, `showmandatory`, `sourcetable`, `sourcetabletemporary`, `style`, `styleexpr`, `subpagelink`, `subpageview`, `tooltip`, `treeinitialstate`, `updatepropagation`, `usagecategory`, `visible`, `width`.
