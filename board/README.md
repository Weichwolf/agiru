# Delivery board

Review: 2026-09-28. Repository text is English. Open outcomes only; no historical pass count is a current measurement.

## Release gates

| Gate | Required outcome | Owner |
|---|---|---|
| G1 — UT first | Complete in-scope generated tree compiles/links; every source-counted UT method passes through `agiru run-tests`; no missing/refused/crashed/skipped/incomplete cases. Reproducible seed, runner and image identities. | 0038, 0039, 0058, 0004 |
| G2 — usable clients | After G1, build ERP CLI and htmx web UI over the same production page/command runtime. Prove every operation's parity, including database effects and errors; browser checks are representative samples. | 0030, 0720, 0006, 0062 |
| G3 — complete ERP | After G2, execute the complete AL test population and CLI workflows; close all object/extension/integration gaps. Prove multi-user correctness and BC-relative performance/resource targets at scale. | 0058, 0034, 0721 |

- Existing `run-tests` CLI and TestPage repairs belong to G1. Business CLI/HTTP construction starts only after G1.
- CLI becomes the exhaustive business-operation/testing client. Sampling applies to browser rendering/interaction, never to the UT or full AL denominator.
- Namespace exclusions, unlinked fallbacks and unsupported providers remain counted scope gaps; a green subset cannot satisfy complete BC ERP functionality.
- P0: unsafe state or unreliable proof. P1: UT/semantic blockers. P2: clients and their bounds. P3: complete-suite/ERP/scale work. Promote any later task when a measured UT failure requires it.
- WI dependencies name prerequisite contracts, not completion of unrelated later-stage work in the same WI. Local fixtures/review can start earlier; activation requires their stated gates.

## Next implementation order

| Order | Deliverable | WIs / first action |
|---|---|---|
| 1 | Trustworthy source/seed/image baseline | 0589 + 0058 diagnostic census; 0004 scratch guards and fresh sealed seed; 0013 required schema/provenance |
| 2 | Correct execution boundaries | 0718 record ownership; 0722 JSON; 0035 XML safety; 0012 transactions; 0006 sessions; 0723 atomic sequence ranges; 0039 runner hooks/isolation |
| 3 | Compile/semantic closure | 0034 object and platform-symbol census → 0033 app/type identity → 0073 lowering → 0038 full-app link; 0035 only genuine native/.NET contracts |
| 4 | Remaining UT failures | 0044 navigation/company; 0018 filters; 0043 validation/events; 0061 original errors; 0030 page lifecycle; 0063 datasets; 0065 encoding/import; 0719 context drilldown |
| 5 | Clients after G1 | 0062 enforced identity/permissions + 0006 session ownership → 0030 production dispatcher → 0720 CLI/htmx/parity |
| 6 | Complete suite and ERP qualification | 0058 full manifest; remaining object/extension/lifecycle/media/background work; 0721 multi-user and matched BC benchmarks |

## Architecture decisions

- `SessionState` owns mutable AL state; TLS only selects an active session. PostgreSQL owns shared authority, permissions/version state, durable writes and worker claims.
- One transaction lease holds the connection and live cursors. No connection or open transaction is retained during user think time.
- Production Commit is durable. Test isolation floors and TryFunction/Codeunit.Run/asserterror boundaries remain distinct.
- One typed production page dispatcher executes TestPage, CLI JSON and htmx commands. Generated immutable metadata supplies identity, factories and typed accessors; adapters contain no business rules.
- One library per BC app; public headers plus declared dependency roots. PCH/unity/slice remain build optimizations, not dependency or completeness proof.
- Exact Decimal values, bounded blocks/streams and observed-version writes precede optimization. Proposed performance thresholds and matched-BC proof live in 0721.

## Imported implementation evidence (2026-09-30)

- The refreshed predecessor has a fixed-list CI result of 2,539/2,578 and a separate corpus result of 1,853/2,157 in-scope cases; 126 more IDs were explicitly out of scope. These are not agiru UT results, not the same denominator, and not proof of complete BC functionality. Keep agiru's independent source census and explicit exclusions (0058).
- Its page-open smoke run reached 2,517/2,653 non-API pages; open/render success is not operation parity. A single stateful page protocol, modal answers, runtime-derived lookup metadata and separate AL-error/runtime-error classifications inform 0030/0720, only after G1.
- Static test metadata reduced predecessor discovery memory from 615 to 418 MB; lazy object loading reduced its Python image from 812 to 405 MB. These are hypotheses for measuring agiru's immutable C++ metadata and per-session cost, not transferable performance claims (0006/0721).
- A simultaneous CI run, development server and client gate exhausted memory; keep heavyweight integration serialized. Audits must detect stateful mutable objects as well as containers (0006/0589).

## Review findings

| Priority | Reproducible finding | WI |
|---|---|---|
| P0 | SingleInstance and Manual bindings now use per-Session ownership; other mutable TLS state still requires isolation. | 0006 |
| P0 | LockTable arguments/state are not enforced by SQL reads; update/delete predicates omit observed version. | 0012 |
| P0 | JSON child aliases become dangling after sibling insertion (ASan); Decimal JSON conversion rounds exact values. | 0722 |
| P0 | XmlReader ignores Prohibit and reads a local external entity; demonstrated with a review-owned fixture. | 0035 |
| P0 | Concurrent NumberSequence.Range(100) calls reserve overlapping [4,103] and [1,100]. Apostrophes also break Next SQL. | 0723 |
| P1 | Descending SQL Next(-1) moves 2 → 1 instead of 3; Next(0) is now fixed through all four typed/RecordRef SQL/temporary paths. | 0044 |
| P1 | Isolated subscriber failure is swallowed inside an active write transaction. | 0057 |
| P1 | Include/exclude matcher disagreement repaired; generated app/interface closure remains incomplete. | 0033 |
| P1 | Base64Convert, EntityText and WebService absence is reported as '.NET member', obscuring platform/AL work. | 0034, 0038 |
| P1 | Page.Update(false) returns immediately; substantial production behavior remains inside TestPage. | 0030 |
| P1 | Decimal calculation is globally rounded to SQL scale 20 instead of documented CLR scale 28; Format/field/input limits need separate boundaries. | 0066 |
| P1 | SetEncrypted stores plaintext and isolated-storage keys omit extension identity. | 0062 |
| P1 | Session bridges now belong to rt; all three foundations independently link and reject reverse edges. Public-header/runtime-error coupling still needs an audit. | 0589 |

Diagnostic sources/logs: `build/review-20260928/`: RuntimeProbe, ScopeProbe, JsonProbe and BoundaryProbe. Initial review was read-only; subsequent fixes and negative controls are recorded in the owning WIs and verification below. No commits or demo/master mutations.

## Verification

| Evidence | Current result |
|---|---|
| Active frozen run | None; error-payload separation is under local verification in the own worktree. |
| Latest completed frozen run | `20260930T194856Z-716931`, finished 2026-09-30 19:51:49 UTC, 140 seconds. All/UT/tree/apps 2; test/GCC 0 (88 cases/68 toolchain). Includes foundation ownership repair; AllObjWithCaption.ALNamespace blocks the UT runner. |
| Frozen source identity | agiru HEAD `136a5a77c87aade461e72e5722728525c9294fa2` plus dirty inputs; latest completed source/post-source SHA256 both `41ed35704ce8b521c410214c51901182498ca93bbcb5f710aaf512a21adb4b55`. |
| Current UT proof | Completed snapshot refuses AllObjWithCaption.ALNamespace: 0/2,310 executed; all methods missing, 80 codeunits incomplete. User Personalization/Feature Key compile boundaries passed. Independent population retains 80/2,310, zero added/removed identities; own `build/feature-key-proof/ut-identity-comparison.json`. Snapshot artifacts, never historical lane logs, are authoritative. |
| Last executed full UT | Previous snapshot `20260930T114935Z-65182`: 2,199/2,310; 111 failed, 0 incomplete; 80 codeunits, six workers, 1,258 seconds. Same identities/diagnostics as snapshot 105054. Legacy seed lacks complete provenance: diagnostic repeatability, not sealed A/B proof. |
| Complete tree/apps | Latest completed tree 9,867/9,872; five absent-interface failures. All/apps block on AllObjWithCaption.ALNamespace. Last entered full-app build (snapshot 114935): 62/14,719 edges, missing TryGetStringTenantSetting. Pinned native contract inspected; provider absent (0035). No scope reduction. |
| Computed sources, integrated after snapshot | GenPage 25/25 and PageSource 14/14 under Clang 19/GCC 14; main gates 25/25 and 14/14. Generated AL execution fixture 13/13 both; final local suite 85 cases green, toolchain 59/59. Old generator eight red; old reader refuses. One variable lookup replaces four loops; existing complexity finding removed. Main regeneration exits 0; all generated file paths and 9,872 header identities retained; independent census 80/2,310. Two real changed BC page bodies pass Clang syntax. Full UT effects pending. WI 0030; `build/page-source-*` in main and own worktree. |
| Other integrated gate controls | RangeBound 32/32 both, old runtime 20 red; UT preflight 59 toolchain tests, five old-script controls red, old build-cancellation control times out. NextZero 112/112 and Cursor 217/217 both; old runtime 44 red. GenScope 229/229 both; old matcher three red. SessionInstance 12/12, SubscriberLifetime 22/22 and SessionBinding 18/18 both; old runtime five/14/10 red. WIs 0044/0058/0033/0006/0057 retain inputs and logs. |
| Attribute census, integrated after snapshot | Only observed acted-on kinds counted; no unsigned catalogue subtraction. Six actual-transpiler controls pass under Clang/GCC; old binary six red. Own/main local suites 85 cases and toolchain 65 tests green. Changed-code lint 174/185 units, 167 failed; no new census-specific diagnostic or suppression increase. Main `build/attribute-census-main-tests.log`; own `build/attribute-census-*`. WI 0059. Next frozen run still requires the native Field build blocker to be repaired. |
| Native Field metadata | One mapper for Get/provisioning; shared Field/FieldRef option strings. Type Name field 9/Text[30] uses existing primitive metadata and length: Code20/Text100/Option. PlatformField 106/106 Clang/GCC; archived old headers/runtime 46 red. Real generated BC page body compiles and executes 6/6 checks under both compilers; own/main local suites 85 cases / 65 toolchain tests green. Lint 174/185 units, 167 failed; no new mapper/gate findings or suppression increase. Native ordinals/schema and bounded read-only navigation remain open (0034/0044); no full-UT/G1 closure. Own `build/field-native-proof/`; main `build/field-native-main-tests.log`. |
| User Personalization/storage, integrated | All 19 source fields + five system fields; six lookup formulas, shared license vocabulary, correct properties. Independent 19/19 comparison; old surface 9/19. Gate 320/320 Clang/GCC; old headers/runtime 24 red. Native Removed-column contract: current seven green, archived old header/runtime seven red; thirteen Normal columns + five system columns and obsolete-value roundtrip. Real generated readers 7/7 both. Own serialized suites 86 cases / 66 toolchain tests green both; main recheck 86/66 green. Parent-Make override control red before fixture isolation; no runner relaxation. Lint 175/186 units, 168 failed; no new header/gate findings or baseline increase. Own `build/user-personalization-proof/`, main `build/user-personalization-main-tests.log`; 0013/0034. Page binding/provider gaps remain 0030/0019; no functional-page/G1 claim. |
| Source/platform provenance | BCApps main/HEAD `a9ea4d84534cebba852c44bf0f841c2ea149de4e`; BC_VERSION 28.4.53241.0; local System symbols 28.0.53152.0 / Runtime 17.0. Record schema/data mismatches; version text alone is not compatibility proof. |
| Feature Key, integrated after snapshot | All ten source fields; adds 9/10 Text[2048] using existing metadata/runtime. Independent 10/10; old 8/10. Gate 93/93 Clang/GCC; old headers/runtime six red; actual generated readers 3/3 both, old body compilation red. Own suites 87 cases / 66 toolchain tests green both; main 87/66 green (`build/feature-key-main-tests.log`). Lint 176/187, 169 failed; no new touched-header/gate findings or suppression increase. Own `build/feature-key-proof/`. Real option predicate still refuses (0073); source/control IDs absent (0030). No feature activation or functional-page/G1 claim. |
| Foundation ownership, integrated after snapshot | Three bridge bodies moved byte-identically net → rt; Linux no-undefined policy derives from reaches. All al/net/db independently link under Clang/GCC; actual production links reject injected reverse edges, guard-removal controls link. SessionBridge 16/16 on new and archived old runtime. Own suites 88 cases/68 toolchain green both; main 88/68 green. Own `build/foundation-proof/`, main `build/foundation-main-tests.log`; 0589. No new semantic implementation or session-authority claim. |
| Error value, own worktree only | ErrorValue separates owned text/code from unchanged transaction helpers; foundation consumers avoid boundary imports. Payload 15/15 on old/new headers; old boundary imports refuse under both compilers. Own Clang/GCC suites 89 cases/69 toolchain green. Fresh old/new AL generation: all 24,348 files byte-identical. Direct consumer imports and builtin generator aligned; final serial lint pending before main integration. Own `build/error-payload-proof/{direct-final-clang-tests.log,direct-final-gcc-gates.log,final-equivalence.json,serial-final-lint.log}`; 0589. Door comment/type-inventory defects recorded in 0034. |
| Wider independent census | W1 Tests: 1,400 codeunits / 41,524 Test attributes. All BCApps/src: 36,696 AL files, 4,135 test codeunits / 111,961 methods/attributes; zero matcher discrepancies/unreadable files. Internal procedures and 19 UTF-16 sources retained. Localization/conditional variants are not a deduplicated runnable suite. `build/review-20260928/full-source-census.json`. |
| Seed policy | `agiru_seeded` remains an unsealed legacy template. Disposable clones only; no demo/master mutations. WI 0004. |
| Static analysis | Full September 28: 175/175 units, 161 failed, 1,042 unique diagnostics. Latest changed selection: 177/188, 170 failed; added SessionBridge gate repeats inherited header findings, with no owned bridge/gate finding or suppression increase. Not full-surface measurement or a smaller baseline; lint remains red. Own `build/foundation-proof/lint.log`. |
| Suppressions/documentation | 55 suppression/silent-place directives versus unchanged baseline 13; public-header Doxygen 1,502 warnings versus baseline 0. `build/review-20260928/{tidy.log,tidy-units.json,doxygen-warnings.txt}`. Baselines may not rise. |
| Not proved | Complete AL tree/link, G1, clients/parity, full ERP suite, multi-user scale and BC-relative performance. Later stages remain gated. |

- Frozen results/logs: `build/verify/<id>/{result.json,verify.log,artifacts/}`; no live source inputs may be edited.
- Local loop: `make gate GATE=<affected> JOBS=2`; generator: `make tc JOBS=2`; targeted analysis: `make lint-one UNIT=<file>`.
- Integration: one `make verify-start JOBS=6 VERIFY_TARGETS='all test gcc ut tree apps'` at a time; `make verify-status`.
- Activation: unchanged source-counted population, identical sealed seed, every method compared; investigate every loss. Missing/refused/crashed remain failures.

## Open items and consolidation

- Existing IDs and absorbed requirements retained. New IDs 0720–0723 follow the maximum across all Git history (0719).
- 0030 retains page semantics; 0720 owns client delivery/parity. 0006 retains session ownership; 0721 owns performance qualification.
- 0035 retains native/.NET bridges; 0722 isolates shared JSON number/lifetime safety.
- 0723 isolates the newly reproduced number-sequence reservation/identity contract from 0012's record transactions.
- Older consolidation source: `git show 136a5a7:board/<old-name>`. The pre-review uncommitted board is also frozen in `build/verify/20260928T142909Z-7923/source/board/`.
- Absorbed IDs record requirement ownership, not implementation completion.

| WI | Priority | Outcome | Absorbed IDs / mapping |
|---|---|---|---|
| [0004](0004_provisioning_will_record_source_and_seed_provenance_and_produce_usable_test_databases.md) | P0 | Provisioning will record source and seed provenance and produce usable test databases | 0613 |
| [0006](0006_session_ownership_and_performance_will_have_measured_portable_bounds.md) | P0 | Mutable runtime state will belong to the session | 0008, 0009, 0596 |
| [0012](0012_production_commits_will_be_durable_and_concurrent_writes_will_be_checked.md) | P0 | Production commits will be durable and concurrent writes will be checked | 0060, 0077, 0079, 0087, 0267, 0458, 0463, 0490, 0621 |
| [0013](0013_system_fields_and_schema_keys_will_obey_their_declared_contracts.md) | P1 | System fields and schema keys will obey their declared contracts | 0080, 0353, 0371, 0511 |
| [0018](0018_sql_and_temporary_records_will_evaluate_the_same_filter_grammar.md) | P1 | SQL and temporary records will evaluate the same filter grammar | 0432, 0508, 0509, 0543 |
| [0019](0019_flowfields_will_preserve_semantics_and_use_bounded_aggregate_queries.md) | P1 | FlowFields will preserve semantics and use bounded aggregate queries | 0047, 0340, 0341, 0342, 0507, 0510, 0521, 0678 |
| [0030](0030_one_page_lifecycle_will_drive_testpage_and_the_http_ui.md) | P1 | One page lifecycle will serve TestPage and both clients | 0083, 0198, 0206, 0210, 0211, 0212, 0213, 0218, 0220, 0221, 0233, 0278, 0279, 0280, 0281, 0282, 0283, 0284, 0285, 0286, 0287, 0288, 0289, 0292, 0293, 0294, 0295, 0296, 0298, 0300, 0329, 0330, 0334, 0335, 0336, 0337, 0362, 0374, 0375, 0385, 0387, 0388, 0389, 0390, 0393, 0395, 0399, 0400, 0401, 0402, 0403, 0404, 0405, 0406, 0407, 0408, 0409, 0411, 0413, 0414, 0415, 0416, 0417, 0418, 0419, 0420, 0421, 0422, 0423, 0425, 0426, 0428, 0429, 0430, 0431, 0433, 0434, 0460, 0466, 0469, 0474, 0475, 0477, 0478, 0480, 0482, 0485, 0487, 0496, 0529, 0537, 0538, 0539, 0540, 0541, 0542, 0551, 0553, 0554, 0555, 0560, 0561, 0567, 0568, 0574, 0579, 0657, 0665, 0696, 0700, 0703 |
| [0033](0033_app_boundaries_and_extension_merges_will_be_explicit_and_enforced.md) | P1 | App boundaries and extension merges will be explicit and enforced | 0015, 0069, 0192, 0207, 0209, 0355, 0356, 0357, 0359, 0360, 0363, 0427, 0533, 0544, 0545, 0552, 0572 |
| [0034](0034_every_object_kind_will_have_a_truthful_translation_and_runtime_census.md) | P1 | Every object kind will have a truthful translation and runtime census | 0297, 0365, 0424, 0465, 0479, 0565, 0604, 0623 |
| [0035](0035_rebuilt_net_types_will_preserve_reference_and_culture_semantics.md) | P0 | Rebuilt .NET types will preserve reference and culture semantics | 0078, 0215, 0222, 0227, 0497, 0502, 0585, 0618, 0646, 0675, 0714 |
| [0038](0038_the_complete_generated_tree_will_compile_and_link_without_slice_fallbacks.md) | P1 | The complete generated tree will compile and link without slice fallbacks | 0580, 0591, 0595, 0598, 0599, 0612, 0702 |
| [0039](0039_the_al_runner_will_control_test_lifecycle_and_isolation_explicitly.md) | P0 | The AL runner will control test lifecycle and isolation explicitly | 0223, 0225, 0268, 0269, 0470, 0472, 0493, 0630, 0715 |
| [0043](0043_validation_and_relations_will_run_in_the_documented_order.md) | P1 | Validation and relations will run in the documented order | 0029, 0068, 0228, 0229, 0230, 0232, 0234, 0235, 0236, 0237, 0238, 0239, 0240, 0241, 0242, 0243, 0316, 0317, 0318, 0319, 0320, 0321, 0322, 0325, 0328, 0331, 0332, 0530, 0677 |
| [0044](0044_record_operations_will_share_one_correct_sql_and_temporary_contract.md) | P1 | Record operations will share one correct SQL and temporary contract | 0025, 0032, 0056, 0364, 0398, 0449, 0481, 0505, 0522, 0523, 0607, 0620, 0659, 0697 |
| [0045](0045_reads_will_remain_bounded_and_partial_records_will_be_real.md) | P2 | Reads will remain bounded and partial records will be real | 0017, 0048, 0344, 0345, 0348, 0350, 0351, 0370, 0372, 0520, 0660 |
| [0055](0055_errors_and_labels_will_keep_bc_text_codes_and_navigation_context.md) | P1 | Errors and labels will keep BC text, codes and navigation context | 0382, 0384, 0506, 0518, 0519, 0528, 0566 |
| [0057](0057_events_will_preserve_lifetime_permissions_and_isolated_transaction_semantics.md) | P1 | Events will preserve lifetime, permissions and isolated transaction semantics | 0191, 0196, 0197, 0203, 0204, 0244, 0245, 0246, 0247, 0248, 0249, 0251, 0252, 0253, 0254, 0255, 0256, 0257, 0258, 0259, 0260, 0261, 0262, 0263, 0264, 0265, 0266, 0512, 0513, 0514, 0515, 0516, 0706 |
| [0058](0058_every_ut_run_will_reconcile_results_with_an_independent_source_manifest.md) | P0 | Every UT run will reconcile results with an independent source manifest | — |
| [0059](0059_surface_coverage_will_distinguish_declarations_refusals_and_tested_behaviour.md) | P1 | Surface coverage will distinguish declarations, refusals and tested behaviour | 0028, 0040, 0071, 0358, 0484, 0525, 0562, 0563, 0564, 0588, 0593, 0594, 0610 |
| [0061](0061_consumed_and_discarded_calls_will_keep_al_error_semantics.md) | P1 | Consumed and discarded calls will keep AL error semantics | 0226, 0517 |
| [0062](0062_authorization_will_be_enforced_at_data_and_object_boundaries.md) | P1 | Authorization will be enforced at data and object boundaries | 0202, 0214, 0313, 0314, 0315, 0376, 0377, 0378, 0379, 0380, 0381, 0473, 0483, 0492, 0495, 0499, 0559, 0671 |
| [0063](0063_reports_will_execute_datasets_and_render_declared_layouts.md) | P1 | Reports will execute datasets and render declared layouts | 0301, 0302, 0303, 0304, 0305, 0306, 0307, 0308, 0309, 0391, 0396, 0397, 0436, 0450, 0451, 0452, 0454, 0455, 0457, 0459, 0486, 0488, 0489, 0546, 0547, 0549, 0557, 0575, 0576, 0577, 0628 |
| [0064](0064_queries_will_stream_correct_joins_filters_and_typed_aggregates.md) | P2 | Queries will stream correct joins, filters and typed aggregates | 0299, 0352, 0441, 0453, 0461, 0462, 0464, 0550, 0556 |
| [0065](0065_xmlports_will_preserve_schema_encoding_and_import_transaction_semantics.md) | P1 | XMLports will preserve schema, encoding and import transaction semantics | 0310, 0311, 0312, 0338, 0367, 0410, 0412, 0442, 0443, 0444, 0445, 0446, 0447, 0448, 0501, 0548 |
| [0066](0066_formatting_and_text_operations_will_follow_al_culture_and_character_rules.md) | P1 | Formatting and text operations will follow AL culture and character rules | 0007, 0010, 0016, 0041, 0053, 0075, 0082, 0323, 0324, 0326, 0437, 0438, 0440, 0491, 0503, 0527, 0632, 0716 |
| [0070](0070_install_and_upgrade_will_execute_declared_lifecycle_transactions.md) | P3 | Install and upgrade will execute declared lifecycle transactions | 0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277, 0500, 0534 |
| [0073](0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md) | P1 | Generated expressions will preserve AL types and evaluation effects | 0027, 0049, 0051, 0076, 0081, 0084, 0085, 0086, 0088, 0089, 0467, 0468, 0524, 0531, 0558, 0573, 0578, 0581, 0584, 0587, 0590, 0597, 0608, 0614, 0629, 0633, 0695 |
| [0074](0074_streams_and_media_will_have_explicit_ownership_and_persistent_storage.md) | P1 | Streams and media will have explicit ownership and persistent storage | 0031, 0200, 0494, 0532, 0535 |
| [0090](0090_background_work_will_have_database_backed_ownership_and_session_isolation.md) | P3 | Background work will have database-backed ownership and session isolation | 0290, 0291, 0498, 0536 |
| [0589](0589_the_toolchain_will_be_reproducible_and_every_gate_will_fail_reliably.md) | P0 | The toolchain will be reproducible and every gate will fail reliably | 0005, 0046, 0050, 0616 |
| [0718](0718_record_images_and_temporary_handles_will_survive_copies_without_dangling_references.md) | P0 | Record images and temporary handles will survive copies without dangling references | 0037, 0042, 0526, 0624, 0640 |
| [0719](0719_error_message_drilldown_will_retrieve_the_logged_record_context.md) | P1 | Error-message drilldown will retrieve the logged record context | — |
| [0720](0720_cli_and_web_will_execute_the_same_erp_operations.md) | P2 | CLI and web will execute the same ERP operations | Client delivery/parity split from 0030 |
| [0721](0721_equivalent_erp_workloads_will_prove_lower_latency_and_resource_cost.md) | P3 | Equivalent ERP workloads will prove lower latency and resource cost | Performance qualification split from 0006; 0008/0009/0596 retained |
| [0722](0722_json_numbers_and_aliases_will_preserve_values_and_lifetimes.md) | P0 | JSON numbers and aliases will preserve values and lifetimes | Shared JSON safety split from 0035 |
| [0723](0723_number_sequence_ranges_will_be_atomic_and_portably_addressed.md) | P0 | Number-sequence ranges will be atomic and portably addressed | New concurrency finding; related boundary ownership remains in 0012 |
