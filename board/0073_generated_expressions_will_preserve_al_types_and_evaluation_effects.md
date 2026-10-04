# 0073 — Generated expressions will preserve AL types and evaluation effects

Status: open | Priority: P1 | Stage: UT lowering | Reviewed: 2026-10-04
Depends on: 0033 symbol identity.

## Evidence

- Boolean lowering now emits `LogicalAnd/Or/Xor` with aggregate-initialized owned
  Boolean operands, left before right; no lambdas, allocations or added standard
  includes. Nested grouping and consumed TryFunction errors remain intact; ternary
  branches stay lazy. `make boolean-expressions JOBS=2`: primitive 26/generated AL
  34 checks green; the old compiler fails 15/34, the source-order mutant fails 4/34.
  Receipts: `/tmp/agiru-boolean-before-session.log`,
  `/tmp/agiru-boolean-expressions.VP2FKL`; header frontend 1.0 ms/three no-PCH rounds
  (`/tmp/agiru-include-cost.36nXSC`). Runtime gate/runner targeted analysis passes.
  Binary traversal is separated from member-handle selection; its former complexity
  46 finding is gone, Link decreases 42→38. New helpers have no findings; inherited
  BodyWriter/RuntimeSurface and BodyWriter.h findings still refuse generator lint.
  Authored golden/expectations corrected; 78 GenTable checks pass. Final complete
  `make test JOBS=2`: 130 cases/223 tooling checks green, exit 0
  (`/tmp/agiru-boolean-final-local.log`). Full regeneration exits 1 with the same
  native/refusal/missing counters as `cvtGhD`; all 14,225 slice sources remain.
  `/tmp/agiru-transpile.Hzzsf5`. `make lint` refuses missing specialist receipts;
  native-report qualification exposed its stale absent-header/source-count
  recipe, repaired in 0034. Full AL execution remains pending in snapshot 101600.
  Authority: developer `devenv-al-{operators,boolean-operators}.md` (Boolean types),
  BCApps `bb7111877f` `Inventory/Posting/ItemJnlPostLine.Codeunit.al:3984`
  (right-side lot splitting), predecessor 1057/1712. Eager evaluation is supported
  by source usage/prior findings; this is not a new native BC runtime oracle.
  User intent: `inventory-how-work-item-tracking.md`, user docs `0ff62b2266fd`;
  developer docs `ff5939a46e05`.
- Numeric primitive ready, not emitted: `include/type/AlDecimalArithmetic.h`,
  `src/net/Decimal.cpp`, `test/gate/AlDecimalGate.cpp`; 0066 records 34 gate checks,
  37,532 BC29 reference cases and three compiled controls. Normalize is separate
  from the CLR core, field range, SQL and display policy. Bind conversions by the
  declared AL/.NET type at every literal/assignment/parameter/return/field boundary;
  sequence side-effecting operands left-to-right before passing owned values.
  Do not activate only division or rely on C++ argument evaluation order.
- Current case lowering (2026-10-04): CaseChain binds the selector once in a scoped
  value. Generated Boolean TryFunction and ordinary Integer side-effect selectors
  execute across multiple/range/else branches; the repeated-selector mutant fails.
  The 40-check call-context fixture and controls are recorded in 0061. Call's former
  154-line argument loop is split without new targeted helper findings. Case-label
  conversion/overflow and Code's special comparison rule remain unproved.
- Current generic record-call repair in `src/gen/BodyWriter.cpp::Link`: callable
  members whose C++ spelling aliases a native field retain the runtime method in
  explicit and property-access syntax. Quoted Record ID stays a value. Actual
  CRMNotesSynchJob consumes both identities; GenCodeunitGate passes 41 checks.
  Reference: developer `methods-auto/record/record-recordid-method.md`, BCApps
  `bb7111877f` Integration/D365Sales/CRMNotesSynchJob.Codeunit.al; predecessor 1337.
  Full UT execution remains pending. Targeted Link cognitive complexity is 41;
  simplify declaration-owned call selection without changing user-method priority.
  Existing TableCaption property/quoted-field controls require no unnecessary base
  qualification: the refined distinction passes all 159 binding checks and the full
  120-case local replay. `RecordId` retains all four new controls.
- Current-source AL/compiler counterexamples: all four fixtures pass Microsoft AL 17.0.34.45391, but agiru C++ fails without PCH. Internal-case overloads fail for different arity, same-arity Integer/Text and bounded `var Text`; identical-spelling Guid/Text overloads instead force the Text literal through Guid from the first declaration. Three first-letter-only variants already compile/execute; keep these positive controls. build/overload-binding-20261002/artifacts/{internal-case,checked}/probe.json; each case keeps oracle/generation/compile logs. No emitter fix or BC runtime execution. The first draft's illegal Codeunit.Run is retained in artifacts/arity/oracle.log (AL0440); corrected fixtures use VerifyContract.
- Callee metadata is chosen independently and without actual argument types: CodeunitWriter::Names::{Resolve,ParameterTypes,LentParameters}, shared LentParametersOf and MemberLentParametersOf, BodyWriter::{Callee,Call}, and Main::Procedures' name-only map. The literal counterexample proves that canonicalizing C++ spelling or selecting only arity is insufficient; preserve the selected declaration through argument adaptation and return/handle handling.
- Retained SCMProductionOrdersII.cpp fails identically before/after the runtime filter-group change: calls at 3525/4094 select CreatePutAwayFromPutAwayWorksheet's seven-argument spelling, although AL calls its case-insensitively equal one-argument overload. Both declarations/bodies are emitted. Source: BCApps main src/Layers/W1/Tests/SCM-Manufacturing/SCMProductionOrdersII.Codeunit.al, calls 5642/6746, declarations 10075/10099. Bind the complete overload signature before allocating callee spelling; do not rename source helpers or insert business-specific arguments. Receipts: build/filter-group-integration-20261002/artifacts/consumer-{before,after}-apps-tests-manufacturing-test-codeunit-SCMProductionOrdersII.cpp.log.
- Main PageNames reuses declared fields/options for Rec, xRec and bare fields; explicit record options ignore shadowing local/page variables. Ordinary collision allocation and report dataitem context are retained. Known native options also resolve through SourceTable without a copied AST; unknown tables/options still refuse. No page-specific branch or contract removal.
- Main now derives native Rec/xRec and named-record method/property context from SourceTable and the existing TableRef. One record owner serves fields, methods and native aliases; nullable AST access is guarded. Exact AL field identity precedes methods: "Table Caption" does not capture TableCaption, nor "User ID" UserId. Undeclared bare fields are not guessed from C++ names. No page/builtin-specific exception.
- Current promotion: 159 binding checks/140 unchanged Python identities green, same 96 local cases/26 DB failures. Old compiler fails 41 checks and both AL aliases; the normalized-field mutant fails one new check. Two generated pages execute without PCH, proving getter/setter, selected-group filters, named field-argument ownership and instance isolation. All 58 changed/regression body/definition units compare 54→55 green; IncomingDocumentApprovers gains, UserCard retains only IsWSKeyAllowed. The measured DefaultDimensionsMultiple draft loss is repaired and retained in the comparison. Full Field declarations/providers, complete receiver signatures and AL/SQL execution remain open; current receipts are in README.
- Main Record/Codeunit member calls consume declaration-owned signatures; TableNames retains variable/field types and routes own/record parameter lookup through the same owner. Four generated AL cases compile/execute without PCH; old compiler fails all four. Real PurchaseBatchPostMgt/SalesBatchPostMgt compile; all 23 changed/tooling consumers retain or improve results, no new diagnostics. Non-named receivers, SQL/posting execution and complete page lifecycle remain open; current receipts are in README.
- Main now places TableNames' primitive shortcut after declared Page/TestPage/TestRequestPage/Query receiver resolution. GenReceiverGate retains eight receiver cases plus two include-deduplication controls: 10 green, old BodyWriter three red, old Door one red. Actual CLR Description remains a getter; no global-name exception or copied prototype binding API. All thirteen main-origin changed bodies and eight original consumers compared without PCH; no new compile loss. Targeted BodyWriter findings 22→22, no new findings across six units. build/dictionary-integration-20261002/artifacts/{controls,body-comparison,consumer-comparison,targeted-lint,proof}.json. Complete receiver/signature binding and page execution remain open.
- Own field-enum repair: one `CodeunitWriter::FieldEnumerationOf` lookup for Codeunit/Table/Page/Query; nearest declaration owns shadowing. Native source-record scopes use it before ordinary AST fallback. `build/field-enum-20261001/`: binder 105 green/predecessor seven red; oracle-valid AL eight checks plus four driver assertions. Report root 639 compiles; its ObjectOptions→Clear blocker is superseded by 0034. No new registry/runtime branch or integration.
- Latest own `build/object-options-20261001/`: generic TableWriter allocator reserves the C++ Temporary wrapper name; original AL field identity remains unchanged. Ordinary/native generated reads/writes use source-number suffixes. Binder 125 green (105 retained)/predecessor nineteen red; native declaration/primitive proofs belong to 0034. Stored Boolean Temporary and Record.IsTemporary are separate. No business-object naming branch, second allocator or global spelling substitution.
- Guid prototype reuses Text concatenation, preserves native-storage std::string and constrained Guid operands. Clang/GCC Text 75 green (43 retained), erased-result mutant six red; generated AL sixteen checks plus destination-error assertion. Actual consolidation compiles, whole old headers fail. Compiler 17.0 proves eight Text-result cases (both Guid orders, bounded/unbounded Text, Code+Code, literal joins). Code+Code still erases that result. SecretText accepts a Text variable but refuses a literal AL0122. `build/text-guid-20261001/{source,artifacts/}`; oracle/source identities retained there, not integrated.
- Latest local suites: 92 cases/26 unchanged connection failures; 119 toolchain tests green under both compilers. CodeunitWriter/TableWriter findings 10/34 unchanged; earlier BodyWriter 25 unchanged. Full source-counted UT/sealed-seed activation remains required; current declaration/build receipts belong to 0034.
- Record-call prototype retains quoted-field identity and generic `BodyWriter::CallableSpelling`: four spellings, implicit/explicit calls, own-versus-related RecordId; 28 AL checks. Extend binding beyond named receivers, not global spelling rules. Earlier Text prototype retains literal/label anchors in `BodyWriter::{Added,TextJoin}` and Text results in `StringValue.h`; old generator/runtime independently fail. Receipts and historical counts: README, `build/{record-link,text-result}-20261001/`; neither is integrated.
- The frozen tree also exposes five header failures from undeclared absent interfaces: PowerBIServiceProvider and GraphAuthorization. Their actual AL declarations are excluded by namespace while in-scope callers name them. This remains an identity/dependency-closure gap in 0033/0034, not a working interface implementation or permission to broaden fallback conversions.
- `BodyWriter::Binary` still promotes division through the raw CLR Decimal core;
  declared AL numerical normalization/conversion is not active. Boolean lowering
  is qualified separately above, not proof of arithmetic sequencing.
- Decimal lowering currently uses the CLR core directly. Executed original BC29
  Decimal18 arithmetic rounds to eighteen significant digits (0066), independently
  of SQL scale/display limits. Preserve typed AL versus .NET conversions and
  operator results; do not globally reduce the shared CLR primitive's precision.
  Actual target-version parity and full unchanged-population activation remain due.
- `TableWriter` caches names by AST address/partial identity; shared option generation writes sanitized C++ spelling as AL names.
- Actual ReportResGovernSettings compiles without PCH; whole predecessor fails its reconstructed type under both compilers. Actual FeatureManagement executes all four one-way/reversible × None/All Users editability cases under both compilers. Its preceding own image already executed them with an ordinary option wrapper; do not claim a formerly failing runtime control. Current regeneration uses the native vocabulary. `artifacts/real-consumers.json`; validation/actions, provider and actual-client workflows remain unproved.
- `BodyWriter::ControlTrigger` checks only the base method name against AL procedures, then returns one unchecked `_Control` suffix. Reserve the complete generated member namespace: both that suffix and another control's synthesized method can collide. Cover multiple occupied suffixes and two controls with intersecting trigger/getter spellings; do not rename either AL declaration.
- `Main::IndexXmlPorts` leaves procedure declarations empty; `CodeunitWriter::MemberLentParametersOf` now accepts Codeunit/Record, not XMLport receivers. XMLport naming/export controls do not prove custom-method `var` arguments. Resolve declarations once through typed receiver binding, not a second parameter map.

## Implementation

1. Reuse declaration-owned callee/receiver and value-consumption lookup before printing. Introduce lowered sequencing only where C++ evaluation differs; share that representation with 0061, not a second type catalogue.
   Resolve a call once to its owning ProcedureDecl using the AL name and full ordered argument types; consume that result for spelling, var/Option/Variant borrowing, Guid literal adaptation, publisher arguments and returned handles. Preserve record/enum identity, nested generic arguments and array shape; length and return type do not invent overload identities. Keep source names/metadata separate from C++ allocation. Unknown/ambiguous best matches must refuse, never select the first/last variant.
   Use existing Objects/TableRef declarations, not another method catalogue or an arity-only dispatcher. Cover own/member Codeunit, Record, Page, Report, Query, XMLport and Interface calls, local shadowing, different/same arity, conversion ranking, var writeback and both declaration orders. Retain the four compiler-accepted failing fixtures, three current positive controls and original SCMProductionOrdersII comparison before promotion. Audit fixture legality with AL compiler controls; source-binding Caller.Codeunit.al's custom Run conflicts with the platform method and needs a separately verified fixture repair.
2. Add execution fixtures for integer division yielding Decimal, decimal DIV/MOD,
   overflow and numeric operand order. Retain the eager Boolean value/effect/error
   controls above; derive expectations from AL documentation and source usage.
   Retain single-evaluation case controls while implementing declared-type label
   conversion and its Code exception; do not re-expand the selector per comparison.
3. Activate the option/record-call prototypes only with full sealed-seed UT A/B. Preserve native/ordinary, alias, shadowing, four-spelling/implicit-call and quoted-field controls. Extend the same declaration-owned binding to indexed/chained receivers and implicit Rec; retain user-procedure priority, not a second method catalogue. Runtime record identity/formatting stay separate; never make RecordId fields callable.
   Declared controls/columns/fields must precede primitive or CLR getter fallback; extend the integrated receiver-priority rule to every lowering path and preserve the actual .NET getter.
   Retain custom procedure signatures and argument modes for every receiver kind; use the same declaration binding for member spelling and `LentParametersOf`. Compile and execute an XMLport method updating a caller's bounded Text, with same-app/dependent-app and archived-emitter controls.
   Distinguish valid Enum.AsInteger from invalid Option.AsInteger: the AL compiler refuses the latter, while the C++ wrapper exposes it. Reject C++-only helper names through typed declaration binding; do not invent AL methods from runtime convenience APIs.
   Activate Text/Guid prototypes only after full sealed-seed UT A/B. Runtime primitives own string results; the emitter anchors literals/labels. Extend the same primitive to the oracle-proven Code+Code→Text contract, preserving native storage joins and destination limits; no second dispatcher/type map. Validate SecretText literal conversion separately from supported Text-variable assignment. Add callee binding only where declared expression types are insufficient.
4. Gate arrays with all dimensions, one-based bounds and var views; enum compatibility only when declared; interface inheritance, implementation selection and assignment ownership.
5. Remove the process-static field-name cache or bind it to an explicit generation context with complete ownership. Preserve the shared wrapper-name reservation and generated execution controls during activation. Emit original AL option names separately from sanitized identifiers; gate `Group(Resource)`, `% Extra` and `LCY Extra` through the real shared option writer. Use generated static_asserts for known limits and type relations. Unsupported conversions must fail at generation/compile time rather than fall into permissive Variant conversions.

## Acceptance

- Include 7/2=3.5, a right Boolean operand that changes a var despite a decisive left operand, multidimensional ArrayLen, incompatible enums and a returned interface call. Inspect emitted code and execute it; header compilation alone is insufficient.
- Preserve actual platform-record option predicate execution and named refusal of missing bindings. Prove validation/actions through the production page runtime; page binding IDs belong to 0030.
- Oracle-proven Text/Guid, Code+Code and literal joins retain Text and select declared overloads; SecretText selects SecretText without unwrapping. Keep independent old generator/runtime controls, destination diagnostics, Unicode/case/spaces, native storage joins and owned-buffer transfer. Activation retains the full source-counted UT population and sealed-seed per-method comparison.

## References

Case selectors: `devenv-al-control-statements.md#case-statements`, developer revision
`ff5939a46e05`; predecessor 810 K6. Implementation: BodyWriter::CaseChain;
`test/runtime/test-contexts/{TryScopes.Table.al,Fixture.Codeunit.al,Runner.cpp}`.

Overload binding: methods/devenv-overload-method.md (current official page checked 2026-10-02), devenv-al-type-conversion-expressions.md; BCApps main a9ea4d84534cebba852c44bf0f841c2ea149de4e, W1 Tests/SCM-Manufacturing/SCMProductionOrdersII.Codeunit.al::{CreatePutAwayFromPutAwayWorksheet,CreatePutawayFromPutawayWorksheet}. No distinct end-user procedure-overload contract. Earlier 1090 requires static type identity for same-arity List overloads; 1318 requires preserving all declarations; 943's unconditional last-variant fallback is rejected. Probe AL compiler assembly SHA256 1eabc562b732bd0b2819a9d4ac152c5ab442d236ea228f8fb65461606c37faed/System.app 5b72ba127cb2221722f02544bfb3f3bd5a50402bf92bfab6d7e584e049b9681d; source/hash pairs remain in probe.json. Acceptance by this compiler is not a BC runtime oracle.

Native record context: src/gen/BodyWriter.cpp::PageNames::{SourceSubtype,RecordSubtype,RecordOf,RecordFieldOf,PlatformFieldOf}; same gates/fixtures below. Platform methods-auto/record/record-{filtergroup,count,istemporary,tablecaption,setrange,getfilter}-method.md. BCApps main a9ea4d84534cebba852c44bf0f841c2ea149de4e: W1 BaseApp eServices/EDocument/IncomingDocumentApprovers.Page.al, Modules/System/User/UserCard.Page.al, Finance/Dimension/{DefaultDimension.Table.al,DefaultDimensionsMultiple.Page.al}, Finance/Analysis/BudgetMatrix.Page.al. User business-central/ui-enter-criteria-filters.md. Earlier 1063/1103 require independent active-group storage and full loss investigation, not transplanted Python machinery; 1709 requires exact quoted field identity. Draft regression and new negative control: build/native-record-context-integration-20261002/artifacts/{draft-consumers,consumers,normalized-field-mutant}.json. Runtime SQL/consumed-return proof remains 0044; the getter/bounds/HasFilter repair is indexed in README.

Page record binding: src/gen/BodyWriter.cpp::PageNames::{FieldsOfRecord,SourceEnumeration,FieldEnumeration}; src/gen/CodeunitWriter.cpp::{FieldEnumerationOf,QueryColumnEnumeration}; test/gate/GenSourceBindingGate.cpp; test/transpiler/page-record-binding/; test/tooling/toolchain.py::PageRecordBindingGate. Platform devenv-{system-defined-variables,deprecating-with-statements-overview,al-type-conversion-expressions}.md, properties/devenv-sourcetable-property.md, methods-auto/option/option-data-type.md. BCApps same main: src/Layers/W1/BaseApp/System/ChangeLog/ChangeLogSetupFieldList.Page.al. User business-central/across-inspect-page.md. Earlier 1709 warns against broad UserId/enum substitution; 1040 rejects duplicate registries. Fixture returns use documented Option→Integer conversion, not unsupported Option.AsInteger. Earlier report regression and fixture corrections remain in build/page-record-binding-integration-20261002/artifacts/; no platform-compiler/SQL proof.

Receiver precedence: src/gen/BodyWriter.cpp::TableNames::MemberIsCall, test/gate/GenReceiverGate.cpp::DeclarationsTakePrecedenceOverGetterNames. Platform methods-auto/{testpage/testpage-data-type,testfield/testfield-value-method}.md; same BCApps main a9ea4d84534cebba852c44bf0f841c2ea149de4e, Layers/W1/{Tests/TestLibraries/AsmAvailabilityTestBuffer.Table.al,BaseApp/Assembly/Document/AssemblyAvailability.Page.al}. User business-central/assembly-assemble-items.md. Earlier 965 requires declared control aliases before raw table fields; no Python machinery adopted. Global door callable names still do not replace complete receiver/signature binding.

Text/Guid: platform `methods-auto/{database/database-usersecurityid-method.md,guid/guid-data-type.md,guid/guid-totext--method.md,guid/guid-totext-boolean-method.md}`, `devenv-al-type-conversion-expressions.md`; BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `src/Layers/W1/BaseApp/Finance/Consolidation/ImportConsolidationFromAPI.Codeunit.al:210`; user `business-central/finance-consolidated-company-reporting.md`. Earlier 1449 requires UserSecurityId's Guid return; 902 rejects string heuristics/type-erased Guid reads. AL compiler assembly SHA256 `1eabc562b732bd0b2819a9d4ac152c5ab442d236ea228f8fb65461606c37faed`; System.app SHA256 `5b72ba127cb2221722f02544bfb3f3bd5a50402bf92bfab6d7e584e049b9681d`. Static compiler/type proof, not BC runtime execution.

Native option scope: `src/gen/BodyWriter.cpp::{TableContext,PageContext}`, `src/gen/CodeunitWriter.cpp::{FieldEnumerationOf,QueryColumnEnumeration,PlatformFieldEnums}`. Platform `methods-auto/{option/option-data-type.md,record/record-{setrange,getrangemin}-method.md}`, `properties/devenv-optionmembers-field-property.md`, `devenv-al-variables.md`; pinned `src/Virtual Tables/AllObjWithCaption.Table.al`. BCApps same main, `src/Layers/W1/BaseApp/ReportResGovernSettings.page.al:171,230`, `src/System Application/App/Feature Key/src/FeatureManagement.Page.al`; user `business-central/ui-work-report.md`. Earlier 1374 requires actual emitted-form evidence and one member source, not guessed registries or documentation-order ordinals. `artifacts/al-oracle.json` retains the compiler refusal; filter caption appearance was not adopted from observed output (`filter-caption-diagnostic.json`).

Wrapper/field naming: `src/gen/TableWriter.cpp::FieldIdentifier`, `test/gate/GenTableBindingGate.cpp::TemporaryFieldsDoNotHideBehindTheRecordWrapper`; Object Options references in 0034. Independent C++ control rejects `Temporary<Row>.Temporary`; stored flag and temporary-storage controls execute under both compilers. No directly relevant predecessor Temporary-field naming implementation found; retain existing declaration/overload identity policy (1318).

Record method/field separation: `methods-auto/record/record-recordid-method.md` allows property access; pinned `Tenant Database Tables/RecordLink.Table.al` separately declares field 2. Same BCApps main: `Layers/W1/BaseApp/Integration/D365Sales/CRMNotesSynchJob.Codeunit.al::CreateCRMAnnotationBufferEntry`. User `business-central/ui-work-with-notes.md`: legacy intent only. Earlier 917 preserves declaration/arity priority; 1337 separates display and encoded storage.

Text overloads: `methods/devenv-overload-method.md`, `devenv-al-type-conversion-expressions.md`, `methods-auto/{text/text-data-type.md,code/code-data-type.md,secrettext/secrettext-data-type.md,record/record-{tablecaption,fieldcaption}-method.md}`. Same BCApps main: `src/Layers/W1/BaseApp/{OtherCapabilities/AccountantPortal/InviteExternalAccountant.Codeunit.al,System/HttpWebRequestMgt.Codeunit.al}`. User `business-central/finance-accounting.md#inviteaccountant`. Earlier 1318 preserves overload identity; 943's unconditional last-variant default is rejected.

Control/procedure allocation: `src/gen/PageWriter.cpp::ControlIdentifiers`, `test/gate/GenPageGate.cpp`; `devenv-page-object.md`; same BCApps main `src/Layers/W1/BaseApp/DataSyncStatus.Page.al`. Earlier `openerp/board/1318_gleichnamige-definitionen-ueberschreiben-sich-still-im-genera.md`: preserve both declaration/overload identities.

XMLport parameters: `src/tc/Main.cpp::IndexXmlPorts`, `src/gen/CodeunitWriter.cpp::{LentParametersOf,MemberLentParametersOf}`; `devenv-al-methods.md#parameters`, `devenv-xmlport-object.md`; same BCApps main `src/Layers/W1/BaseApp/Finance/Consolidation/ConsolidationImportExport.XmlPort.al::GetGlobals`. Earlier `openerp/board/1059_var-parameter-index-kennt-xmlport-prozeduren-nicht.md`: omitted XMLport var metadata; retain caller-owned Code/Text/Decimal/Date.

Ordinary table signatures: src/gen/TableWriter.cpp::BindTable, src/tc/Main.cpp::{IndexTables,RefreshTableIndex}, src/gen/CodeunitWriter.cpp::MemberLentParametersOf, src/gen/BodyWriter.cpp::TableNames; test/gate/GenSourceBindingGate.cpp, test/transpiler/source-binding/, test/tooling/toolchain.py::TableSourceBindingGate. Platform devenv-{al-methods,table-ext-object,handling-errors-using-try-methods}.md and methods-auto/option/option-data-type.md. BCApps main a9ea4d84534cebba852c44bf0f841c2ea149de4e: src/Layers/W1/BaseApp/{Purchases,Sales}/{Document/*Header.Table.al::BatchConfirmUpdateDeferralDate,Posting/*BatchPostMgt.Codeunit.al}, Sales/Customer/Customer.Table.al::SelectCustomer. User business-central/ui-extensions.md. Predecessor 917 requires declaration priority and unchanged-population activation; 906 rejects guessed platform fields; 1008 distinguishes ordinary option ordinals from captions and unrelated coercion paths. No Python runtime machinery adopted.

Platform: devenv-al-operators.md, type conversion tables, array methods, interface/enum properties. AL: Round(1/4*100,1) and Evaluate in compound conditions. Predecessor: WI-1057 proves eager effects; array and var-parameter findings identify copying traps.

Property scope: `assignmentcompatibility`, `assignmentcompatibilityreason`, `defaultimplementation`, `implementation`, `singleinstance`, `unknownvalueimplementation`.
