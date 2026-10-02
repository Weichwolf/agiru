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
| P1 | SetEncrypted stores plaintext and isolated-storage keys omit extension identity. | 0062 |
| P1 | net calls rt Session/HandlerTable despite an empty reaches list; a foundation-only link exposes the unresolved symbols. | 0589 |

Diagnostic source/logs: `build/review-20260928/`: RuntimeProbe (six contract failures), ScopeProbe (matcher disagreement), JsonProbe (two numeric failures and ASan alias failure), BoundaryProbe (XML policy, sequence quoting and overlapping ranges).
Production code is unchanged by this review.

Development after review: nested CommitBehavior and Next(0) fixes are integrated, with meaningful negative controls. Independent manifests now recognize internal procedures and BOM UTF-16 sources. No commits were made.

Scratch ownership/identity guards and canonical libpq parsing are also integrated. Unmarked existing databases are refused rather than adopted. Newly created disposable database/role fixtures prove refusal, stale-handle safety and concurrent acquisition; no demo/master data was mutated.

## Verification

| Evidence | Result |
|---|---|
| Latest completed frozen run | `20260928T155053Z-49374`, finished 2026-09-28 17:18:30 UTC: all/test/GCC exits 0, test 78 cases / 0 red; UT 2,199/2,310, 111 failed, 0 incomplete. Overall failed; legacy-seed diagnostics, not sealed A/B proof. Source and post-source hashes match. |
| Interrupted frozen integration | `20260928T173853Z-198401`: stopped at user request before PC shutdown, 2026-09-28 17:59:25 UTC. All build interrupted (143); later test/GCC/tree/apps/UT targets did not run. Source/logs retained; no complete result. Includes regenerated main apps and scope/navigation/scratch/page-name/SingleInstance/tree-exit fixes, not the later automatic/manual binding work. |
| Complete-tree integration | Same snapshot: tree 9,866/9,872 headers pass, six fail but incorrectly exit 0; apps exit 2 at 108/14,754 objects on NavTenantSettingsHelper.TryGetStringTenantSetting. Remaining app failures unmeasured. Tree exit repair and later runtime/generator patches require a new snapshot. |
| Source identity | agiru HEAD `136a5a77c87aade461e72e5722728525c9294fa2`; snapshot includes the existing dirty worktree. |
| BCApps source | local main/HEAD `a9ea4d84534cebba852c44bf0f841c2ea149de4e`; unchanged checkout inspected. |
| Demo / platform symbols | BC_VERSION 28.4.53241.0; local System symbols 28.0.53152.0 / Runtime 17.0. Compatibility/provenance must be recorded, not inferred from matching version text. |
| Wider source census | W1 Tests: 1,400 test codeunits / 41,524 Test attributes. All BCApps/src: 36,696 AL files, 4,135 test codeunits / 111,961 Test attributes and recognized methods; zero matcher discrepancies. Includes localization/conditional variants, not a deduplicated runnable suite. Internal procedures and 19 UTF-16 sources are recognized; zero unreadable files. Diagnostic: `build/review-20260928/full-source-census.json`; pre-fix census retained as `full-source-census.before-manifest-fix.json`. |
| Previous complete UT evidence | 2026-09-22 snapshot `20260922T170721Z-421118`: 2,199/2,310, 111 failed, 0 incomplete; diagnostic legacy seed, not sealed A/B. |
| Method comparison | September 22 and September 28 results retain the same 2,310 method identities/statuses. The September 28 142909 and 155053 snapshots also match all 2,310 diagnostic strings: zero missing, added or changed results. Diagnostic repeatability, not causal A/B proof; comparison files in `build/development-session/`. |
| Seed | `agiru_seeded`: template, connections allowed, legacy identity absent. Fresh clones only; no mutations to the seed. |
| Static analysis | Current frozen sources: 175/175 handwritten units checked, 161 failed; 1,042 unique diagnostic lines. Logs: `build/review-20260928/tidy.log`, `tidy-units.json`; Doxygen is separate. |
| Development checks | Own worktree: 78 local gate cases / 0 red, 48 toolchain tests green; GCC 14 build successful. CommitBehavior 11/11 and NextZero 32/32 also pass under GCC. Final source patches integrated into the active tree. |
| Scratch checks | Own worktree: 80 local gate cases / 0 red. GCC 14 build successful; ConnectionInfo 16/16, ScratchGuard 18/18, RunnerDatabase 12/12. Seven files integrated after the live frozen snapshot; next integration must include them. |
| Navigation development | Partial/exhausted/extreme Next integrated after the live snapshot. 80 local cases / 0 red; Clang 19 and GCC 14: NextZero 112/112, Cursor 217/217. Typed SQL/temporary and RecordRef, bounded multi-block and exact-block exhaustion. Same 112-check gate against the frozen pre-fix runtime: 44 red. Full UT effect remains unmeasured; targeted analysis still fails inherited headers and TempFind complexity, with no new suppressions. WI 0044 records sources and logs. |
| Generator development | Control/procedure name collision fixed centrally after snapshot 155053. 80 local cases / 0 red; GenPage 12/12 under Clang 19/GCC 14, pre-fix generator 7 red. Generated executable fixture: 6/6 under both compilers. Own-worktree and main regeneration retain exactly the same 9,872 header paths; real DataSyncStatus header Clang/GCC and body Clang syntax checks pass. Main `make transpile JOBS=2` exits 0; population comparison in `build/development-toolchain/`. Targeted analysis still fails existing findings; no suppression increase. WI 0073, `build/development-generator/`. |
| Scope development | Integrated one allocation-free namespace matcher; equal-length exclusion wins. GenScope 229/229 under Clang/GCC, frozen pre-fix generator 3 red; 80 local cases / 0 red and tc complete. Existing rule sets have zero ties, so selected namespaces remain unchanged. For 2,000 admitted checks: 0 new allocations versus 50,000 before. Scope/Apps/GenScope targeted lint passes; global lint remains red. WI 0033, `build/development-scope/`; absent-interface closure remains open. |
| Session development | SingleInstance cache owned by Session, not thread: 81 local cases / 0 red; SessionInstance 12/12 and Instance 35/35 under Clang/GCC. Frozen original headers/runtime negative control: 5 red. Changed functions have no targeted findings; inherited-header lint remains red. Automatic caches, activation, company close and full-population cold-cache effects remain open. WI 0006, `build/development-session/`. |
| Tree gate development | Failed headers now exit 1; header/PCH flags include Werror. Cache keys include compiler identity and flags, preventing reuse of permissive passes. Six real-compiler controls pass, full toolchain suite 54/54. Frozen old script fails two controls (invalid header and warning-only header). WI 0589, `build/development-toolchain/`. |
| Subscriber development | Automatic subscribers are fresh per invocation; Manual bindings are Session-owned. SubscriberLifetime 22/22 and SessionBinding 18/18 under Clang/GCC; old Events.cpp controls 14 and 10 red respectively. 83 local cases / 0 red. Targeted lint still fails inherited headers. Integrated after snapshot 173853; full UT effect pending. Cross-worker object lifetime and isolated transactions remain open. WIs 0006/0057, `build/development-{subscribers,bindings}/`. |
| Scratch analysis | 164/180 available units selected, 161 failed; changed selection, not a full baseline. New-code nullable/argument findings repaired; targeted guard checks now show only the pre-existing shared Error.h finding. Logs: `build/review-20260928/scratch-development-{tidy.log,units.json}`, `*.scratch-tidy.log`; no baseline increase. |
| Development analysis | Changed-code selection: 160/176 available units checked, 157 failed. This is not a smaller full baseline; the frozen 175-unit full analysis above remains authoritative. Targeted checks cover Scopes, Navigate, Temporary, RecordRef and NextZero; existing shared-header/style/complexity failures remain. Direct include findings in Navigate/Temporary were fixed, with no baseline increase. |
| Suppressions | 55 suppression/silent-place directives versus baseline 13; unchanged baseline. No full-lint green claim. |
| Doxygen | Current frozen public headers: 1,502 warnings versus baseline 0; process exit 0 is not a clean documentation gate. Log: `build/review-20260928/doxygen-warnings.txt`. |
| Full tree / clients / complete suite / performance | Not proved by a slice build or UT run; respective WIs remain open. |

- Results: `build/verify/<id>/result.json`, `verify.log`, `artifacts/ut.log{,.manifest.json,.results.jsonl,.run.json}`.
- Short loop: `make gate GATE=<affected> JOBS=2`; generator: `make tc JOBS=2`; syntax root: `make gap`; targeted analysis: `make lint-one UNIT=<file>`.
- Integration: one `make verify-start JOBS=6 VERIFY_TARGETS='all test gcc ut'` at a time; `make verify-status`. Never edit its compiler inputs.
- Activation: compare every method identity/status on the same sealed seed; investigate every loss. Missing, refused and crashed methods remain in totals.

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
