# 0030 — One page lifecycle will serve TestPage and both clients

Status: open | Priority: P1 | Stage: UT lifecycle; Clients adapter extraction | Reviewed: 2026-10-04
Depends on: 0034 platform metadata; 0043 validation; 0018 filters; 0019 totals; 0718 images; 0039 activation proof; 0061 var-Record error-state identity.

## Evidence

- Current original Incoming Document replay: 12/32, unchanged pass identities.
  Error Message 26 is saved before rollback al_621, restored afterward as row 27
  and selected by Status drilldown; the first Error Messages trap now succeeds.
  `/tmp/agiru-incoming-scoped-20261004.{log,jsonl}`; debugger
  `/tmp/agiru-incoming-scoped-page-gdb-20261004.log` fails in
  ErrorMessagesPart.FindFirstField, the second assertion after system Edit.
  This supersedes the earlier first-trap diagnosis (0061).
- ModeAction_ ignored the list's emitted CardPageId and only changed its mode.
  Development Edit routing now uses the declared card and selected record through
  the common Page.Run catalogue, retaining ordinary opening triggers. Explicit
  Edit actions take precedence; standalone mode switching remains. Missing/wrong
  targets, false card ModifyAllowed and absent current rows refuse.
  The authored probe also exposed an unbound AsInteger null dereference; it now
  raises the existing unbound-control error. PageSource 15 and generated AL 15
  checks pass; no-card/wrong-row/no-policy/unbound-integer compiled controls reject.
  Complete `make test JOBS=2`: 127 cases/223 tooling tests green
  (`/tmp/agiru-page-navigation-all-local.log`, proof `/tmp/agiru-page-navigation.IFy49w`).
  Final include-only runner cleanup repeats all focused checks/controls and passes
  targeted lint (`/tmp/agiru-page-navigation-includes-final.log`, proof
  `/tmp/agiru-page-navigation.48JyYP`). Runtime/gate have zero own findings;
  44/49 unchanged header findings remain, without suppression/baseline increase.
  Frozen activation `20261004T064830Z-1661382` completed:
  slice-check/all/test=0, ut=2, six jobs, source
  `76f14f5fc6f16069f0bb4619fdc4adcf91257edb46341db981d3efafc28bfdd8`.
  2,162/2,314, 152 failed, zero incomplete; one gain and no losses/missing identities
  against 044804. Incoming Document reaches 13/32: TestProcessAskUserPermission
  passes; three formerly unopened-page failures now reach Table Metadata's provider
  refusal. `/tmp/agiru-page-metadata-ut-comparison.json`; legacy unsealed seed,
  diagnostic repeatability rather than causal A/B.
- Predecessor 1143 distinguishes first-list and second/card-part assertions; its
  first extra-trigger activation lost six cases. Current OpenPage and
  RereadAfterAction_ already invoke AfterGetRecord; do not add blanket refreshes.
  Original diagnostic clone was owned/marked/non-template and idle when dropped;
  seed OID 4638229 remains unchanged. Null/unsealed seed is not causal A/B proof.
- `include/runtime/test/TestPage.h` owns validation, save/reread, actions, parts and mode handling; `Page.h` owns a second portion of lifecycle.
- `Page::Update(false)` returns immediately. `TestPage::SetControlText`/`RunControlTrigger` expose operations without a general editability/enabled check.
- UT failures include stale batch totals/document numbers, unbound controls, missing traps and lookup results; none proves a single common cause.
- Previously unreadable computed sources include `PurchaseJournal.GetVendorName()` and `BankAccReconciliationLines.TotalBalance + Rec."Statement Amount"`; generated getters now lower their original expressions. Refresh/lifecycle effects remain open.
- Integrated source getters reuse AL statement/name lowering; declarations and constexpr callbacks remain in headers, bodies in sources. One variable lookup replaces four loops; scalar/record/array binding stays separate from computed evaluation. Quoted variables gain typed writeback. GenPage 25/25 and PageSource 14/14 under Clang 19/GCC 14; old generator eight red, old TestPage reader refuses the computed control. Generated AL execution fixture 13/13 under both compilers; final local suite 85 cases green. Global lint remains red; focused repairs remove the variable-resolution complexity finding and new gate findings. Main regeneration exits 0; main gates 25/25 and 14/14. Full UT effects pending.
- Regeneration exits 0; generated file paths and all 9,872 header identities match the frozen baseline. Independent census remains 80 codeunits / 2,310 UT methods. Logs and the executable fixture inputs: `/home/cosmo/Git/agiru-worktrees/goal-20260928/build/page-source-*.log`, `build/development-page-source/fixture/`. Text display uses normal `Format`; this is not typed client-parity or refresh-timing proof.
- Documentation-corrected numeric fixtures stay within AL's Format/literal range: reader 14/14 and generated AL execution 13/13 on both compilers; local suite 85/85. `build/page-source-al-range-*.log`. Initial larger literals were not valid AL conformance evidence; Decimal contract gaps are 0066.
- Latest completed frozen run `20260930T210740Z-797012` passes User Personalization/Feature Key compile boundaries but stops on AllObjWithCaption.ALNamespace; 0/2,310, 80 incomplete. Platform declaration/value closure is 0034; never hide controls or getters to restore slice-only green.
- Generated `UserSettingsList.def.cpp` omits PageDef.sourceTable and all seven ControlDef.field IDs for its platform source. CalcShownFlowFields ignores zero IDs. Compiling/displaying assigned values therefore does not prove automatic lookup calculation; 0019 owns missing-provider/dependency semantics.
- FeatureManagement.def.cpp has the same missing source/field IDs. Its repaired declaration permits three source readers, not a functional page: Enabled comparisons still emit RefusedOption (0073). Both consumers must resolve the ordinary platform-symbol index supplied by 0034, not independent page-specific maps.
- Own 0034 native-contract prototype repairs the previously wrong Caption 20/GUID and missing Namespace 63 reads; shared `Table::FieldFormat` assigned-value checks pass under Clang/GCC. Eleven of fourteen native declarations still fail the generic shape contract; row providers/provenance and full properties remain open. Preserved `build/native-contract-20261001/`; production source is unchanged. Assigned values and syntax compilation are not functional-page proof.
- Current own OData binding emits original three layout fields in `ODataEDMDefinitionCard.def.cpp`, but nine header TestPage fields: six extra raw Blob/system-field aliases come from `PageWriter.cpp::ControlIdentifiers(object, objects)`. `TestPage::SetControlText` refuses absent layout definitions; this is not evidence of six added browser controls. Separate table-field lookup aliases from actual command/control eligibility; both clients must consume declared PageDef layout. `build/odata-edm-20261001/source/apps/base/system/integration/page/ODataEDMDefinitionCard.{h,def.cpp}`; original three declared controls must not disappear.

## Implementation

0. Extend list View/New/Delete with explicit modes and linked-card properties;
   do not change ordinary Page.Run defaults through ambient mutable context.
   Prove missing/wrong targets, empty/deleted rows, inherited policy and permissions;
   retain explicit action precedence and standalone mode switching.
1. Extract a production `PageSession`/`PageDispatcher` below the TestPage adapter. Move the `PageCore` contract out of `runtime/test/`; keep test handlers/assertions outside production behavior.
2. Represent open mode, current row/key/version, dirty/new state, parent/part handles and pending dialog explicitly. Generated factories and typed control accessors supply immutable PageDef/ControlDef metadata.
   Keep submitted control text/validation errors separate from accepted typed record values; an invalid Decimal must remain editable without corrupting the AL record. Emit revisioned refresh results with stable row/control identities.
   Resolve platform SourceTable/control IDs through the same declaration index as AL tables (0034/0033), not page-specific maps. Gate direct displayed FlowFields and visibility against their guarantees; computed expressions retain explicit calculation semantics.
3. Trace open → validate → save → row leave → action → close against AL; distinguish row fetch from current-row change. Preserve delayed insertion, header save before part entry, SetRecords and SubPageLink.
4. Implement control refresh and UpdatePropagation with explicit invalidation. First reproduce save/discount recalculation order; do not blindly replay both after-get triggers on Update. SourceTable determines the omitted SaveRecord default.
5. Enforce mode, inherited Editable/Enabled and permissions at command execution; preserve documented TestPage handler exceptions explicitly. Unknown controls/actions refuse. Client delivery and equivalence are 0720.

## Acceptance

- Lifecycle traces cover new/edited/temporary rows, canceled close, header/part save, lookup writeback and update(false) without writing.
- Computed control reads preserve arguments, exact Decimal cents within the documented AL Format range, owning-instance state, empty values and AL errors. Direct fields/variable writeback remain green; unknown/unbound/unsupported sources refuse. Complex source expressions do not implicitly calculate FlowFields. Decimal domain/scale conformance remains 0066.
- Full UT population green before client construction. Any refresh activation compares every method and investigates losses in invoice-discount and physical-inventory families.

## References

Restore path: developer `methods-auto/codeunit/codeunit-run-integer-table-method.md`
names var Record; BCApps main original eServices/EDocument/IncomingDocument.Table.al::
CreateWithDataExchange/SaveErrorMessages and System/IO/IncomingDocWithDataExch.Codeunit.al::
RollbackIfErrors. Current generated runtime: `include/runtime/Codeunit.h::{Globals,
TakeIn_,GiveBack_}`, `include/runtime/Table.h::Copy`; fix runtime/generator, not apps.
Predecessor 1204 warns that post-rollback table dumps cannot prove absent logging;
its final root cause is different and its FindSet workaround lost 49 tests.

Code: `include/runtime/{Page.h,test/PageCore.h,test/TestPage.h}`, `src/rt/{TestPage,PageRecord,SubPageLink}.cpp`, `src/gen/{PageWriter,BodyWriter}.cpp`, `include/meta/PageDef.h`, `test/gate/{GenPageGate,PageSourceGate}.cpp`. Platform: `devenv-testing-pages.md`, `methods-auto/{page/page-update-method,testfield/testfield-value-method}.md`, `devenv-deprecating-with-statements-overview.md` (page source scope), `devenv-calcfields-calcsums-fielderror-fieldname-init-testfield-and-validate-methods.md` (complex FlowField expressions), page/field/action triggers. AL on current BCApps main (revision in README): `Layers/W1/BaseApp/Finance/GeneralLedger/Journal/PurchaseJournal.Page.al`, `Layers/W1/BaseApp/Bank/Reconciliation/BankAccReconciliationLines.Page.al`, `Layers/W1/Tests/{General Journal/ERMGeneralJournalUT,Bank/MatchBankReconciliationUT}.Codeunit.al`, `VATReport.Page.al`, `DocumentTotals.Codeunit.al`. User-doc search adds no separate source-expression guarantee. Predecessor: WI-775/776 (normal expression lowering, arguments and arithmetic), WI-1113/1235/1330/1401/1411; 1401's repeated refresh attempts lost discount cases.

Edit navigation: developer `ff5939a46e`, `properties/devenv-cardpageid-property.md`
and `methods-auto/{testpage/testpage-edit,testfield/testfield-asinteger}-method.md`;
BCApps `bb7111877f`, original IncomingDocuments.Page.al CardPageId and
IncomingDocToDataExchUT::AssertExpectedError. User `0ff62b2266`,
`business-central/across-income-documents.md` supplies workflow intent, not trigger
ordering. Earlier 1142/1143 separates first-list and second/card-part assertions;
no extra refresh is added. Fixtures: `test/runtime/page-navigation/`,
`test/runtime/page-navigation.sh`; `make page-navigation JOBS=2`.

Property scope: `abouttext`, `abouttitle`, `additionalsearchterms`, `allowedfileextensions`, `allowincustomizations`, `allowmultiplefiles`, `analysismodeenabled`, `applicationarea`, `assistedit`, `cardpageid`, `clearviews`, `columnspan`, `contextsensitivehelppage`, `cuegrouplayout`, `customizations`, `datacaptionexpression`, `datacaptionfields`, `delayedinsert`, `deleteallowed`, `drilldown`, `drilldownpageid`, `editable`, `ellipsis`, `enabled`, `enabled-profile`, `entitycaption`, `entityname`, `entitysetcaption`, `entitysetname`, `extendeddatatype`, `fileuploadaction`, `fileuploadrowaction`, `filters`, `freezecolumn`, `gesture`, `gridlayout`, `groupname`, `helplink`, `hidevalue`, `image`, `images`, `importance`, `indentationcolumn`, `indentationcontrols`, `infooterbar`, `insertallowed`, `instructionaltext`, `isheader`, `linksallowed`, `lookup`, `lookuppageid`, `masktype`, `modifyallowed`, `multiline`, `multiplicity`, `pagetype`, `pasteisvalid`, `populateallfields`, `profiledescription`, `promoted`, `promoted-action`, `promoted-profile`, `promotedactioncategories`, `promotedcategory`, `promotedisbig`, `promotedonly`, `quickentry`, `refreshonactivate`, `rolecenter`, `rowspan`, `runobject`, `runpagelink`, `runpagemode`, `runpageonrec`, `runpageview`, `savevalues`, `scope-action`, `shortcutkey`, `showas`, `showastree`, `showcaption`, `showfilter`, `showmandatory`, `sourcetable`, `sourcetabletemporary`, `style`, `styleexpr`, `subpagelink`, `subpageview`, `tooltip`, `treeinitialstate`, `updatepropagation`, `usagecategory`, `visible`, `width`.
