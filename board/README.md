# Open work after the 2026-09-22 review

This board contains implementation work, not session transcripts. The review started at
commit `136a5a7`. Source paths in WIs are repository-relative; platform references are under
`~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer/`. Historical measurements are identified
as historical and must not be used as the current pass count.

## Execution order for Sol

1. Establish reliable measurements (0058, 0589), then reproduce the memory and transaction
   defects (0718, 0012). Keep production durability and test isolation distinct.
2. Repair the runner's isolation/lifecycle (0039); activate fixes only with a per-method A/B
   over the same AL source manifest and seed. Then investigate the remaining UT failures
   through their generic primitive, starting with 0061/0719 and record/page paths.
3. Continue complete-tree compilation (0038), close semantic gaps and deliver HTTP/TestPage
   parity (0030). Permissions and company/session isolation precede multi-user exposure.
4. Complete reports, XMLports, queries, installation, background work and the whole AL suite.
   Later priority means ordering, not exclusion from the ERP target.

## Verification cadence on six cores

Use an agent-owned worktree for the short edit loop: `make gate GATE=RecordRefGate JOBS=2`
for an affected C++ gate, `make test JOBS=2` for the full local gate set, `make tc JOBS=2` for generator changes, `make gap` for one generated root
and `make lint-one UNIT=<changed.cpp>` for one static-analysis unit. A broad `make
lint` selects every changed header consumer; in this review that was 144 of 170
handwritten units, so it belongs before integration rather than between edits.
Batch public-header edits because they invalidate many consumers. Start expensive
integration with `make verify-start JOBS=6`; it copies the current tracked, untracked
and generated inputs into `build/verify/<id>/source`, checks the content digest and
syncs changed inputs into a serialized reusable build lane. Unchanged files keep
their timestamps and compiled objects; the immutable archive retains each run's
source identity. `make verify-status` and its `verify.log` report the result.
The active source can change while integration runs. Keep only one six-job integration
run on this host; a second agent uses short checks with at most two jobs. CI can put
Clang, GCC, full lint and AL/HTTP suites on separate runners for the same commit; each
job reports its source/seed identity. A local green fast gate never stands in for the
full integration gate.

## Architecture decisions for the next implementation

- Production transactions use real PostgreSQL COMMIT. Test isolation is an explicit policy below
  nested AL error boundaries; globally ignoring Commit is rejected (0012/0039).
- Record images keep stable ownership while references remain live. Copying filters/cursors must
  not replace that owner; temporary storage sharing is a separate operation (0718).
- Mutable runtime services belong to Session, not to a worker thread. Connections are leased to
  transactions; thread-local storage only locates the active session (0006/0012).
- Generated app libraries enforce declared dependency direction. A combined slice and shared PCH
  are build conveniences, never architectural proof (0033/0589).
- TestPage and HTTP share the page lifecycle and generated metadata. Build the renderer on that
  state machine; do not introduce a second set of page templates or business rules (0030).

The WIs specify the proof required before activation. Remaining choices that need measurements
or contract experiments are stated there; this review does not claim every future implementation
detail is decided or every proposed behaviour has been demonstrated.

## Findings that change implementation decisions

- Production `Commit` now performs SQL COMMIT; independent connections prove visibility and
  durability. New failing-before controls exposed false success for an aborted transaction and
  stale savepoints after a deferred constraint failure; both are repaired. A new failing-before
  control also proved that an inner rollback erased an outer `Consistent(false)` mark; each
  savepoint now restores its own mark set on rollback (0012).
- Record-image ownership is stable across Copy, Insert, Get and Modify; allocation-failure
  controls and nested validation/modification events pass ASan/UBSan. Boxed RecordRef
  lifetime and SharedRecord allocation failure are also repaired. Remaining lifetime bridges
  and item-tracking failures are still open (0718).
- Unused-handler failure now discards the method's writes. TestRunner callbacks and the full
  isolation/lifecycle matrix remain open (0039); historic green totals do not prove them.
- Lock/isolation/load-field declarations can accept arguments without enforcing their
  behaviour. Separate signature coverage from implementation and contract tests (0059).
- Session lifetime is not equivalent to thread lifetime; thread-local caches must belong to
  the current session. A shared slice also does not prove app dependency isolation (0006/0033).
- IsolatedStorage omits extension identity, and SetEncrypted stores plaintext (0062).
- Report/query/XMLport implementations now exist. Their original “no generator” descriptions
  and the duplicate 0063 file were obsolete. Rendering and complete semantics remain open.

## Verification

No historical failure is accepted as a new baseline. Commands below run from the repository root.

| Check | Review result |
|---|---|
| Initial `make` | Passed: 14,209 slice sources, 1,081 s build phase; immediate no-op build phase 0 s. |
| Repaired C++ gates (`make gates`, then `make test -o all`) | 68 programs/scripts, 0 red; the original run had 7 red. Final ordinary commands are recorded below. |
| Toolchain negative controls | 19 tests pass, including missing gate binaries, staged/untracked files, transitive headers, empty compilation databases, failed analyzer/nm tools and misleading UT summaries. |
| Full analysis (`make lint -o all FULL=1 JOBS=2`) | Red: 167 units checked, 152 failed, 1,009 distinct diagnostics. Three missing direct includes were subsequently repaired. Baseline remains 0. |
| Suppression/empty-handler counter | 60 existing silent places against baseline 13; unchanged by the review and still red. |
| Documentation (`make doc`) | Generation completes; 1,444 warning lines remain after the existing recursive-class exception. Documentation baseline remains 0. |
| Independent UT source manifest | 80 codeunits / 2,310 Test procedures. No complete AL milestone pass is claimed by this review. |

The review completion build passed (`make JOBS=6`: 1,096 s, 14,209 slice sources).
`make gcc JOBS=6` passed for runtime/transpiler/C++ gates without the generated slice.
The subsequent WI 0058 work added a reporting gate: its first `make test JOBS=6` run
had 69 programs/scripts with 0 red; the current local run has 74 with 0 red. The new
ScopeDiscard gate failed before its allocation-safety fix and passes four checks under
normal and ASan/UBSan builds. The toolchain now has
43 passing controls, including a real CMake/Ninja freshness fixture, compiler-name resolution,
detached-result publication, explicit queued-case status on interruption, and the focused gate's
build/test failure propagation. A controlled Clang 19/CMake PCH fixture changed from
2/2 uncacheable ccache calls to 3/3 cacheable calls and one repeat hit after configuring
the required PCH options; the next full snapshot must measure the project-wide effect
(0589). A frozen BCApps copy previously reported the agiru Git HEAD as its own revision;
future snapshots capture the original BCApps HEAD and pass it to the UT runner, with two
regression controls. Seed transfer now checks both process exits; new seeds carry a durable
`building`/`complete` provenance row and the UT runner refuses an incomplete row. The current
legacy seed has no such identity, so two disposable database probes verified the new contract
without altering it (0004/0058). Existing content digests remain valid (0058). The first real CLI/AL smoke
ran `Bank Rec. Test Report UT` against a disposable seeded database: 1/1 passed, and the
JSONL record carried codeunit ID 134279 and the exact method identity. This is not a
completed AL milestone. The added independent-connection transaction gate proves
production Commit visibility and later rollback separately; the full UT comparison
has now completed. Frozen snapshot `20260922T103145Z-88633` passed `all`, `test`
and `gcc`, then reported 2,197/2,310 AL methods green across all 80 codeunits,
113 failed and 0 incomplete (six workers, 1,017 s for UT). Its source digest
matched after verification. The 113 failures span 33 codeunits, led by SCM
Available to Pick (21), Incoming Doc. To Data Exch. (20), and ERM General Journal
(14). These failures are open work, not a new baseline. Frozen snapshot `20260922T120756Z-158412` independently repeated 2,197/2,310 across 80 codeunits, 0 incomplete (1,066 s UT; 2,362 s total). All/test/GCC passed and the source digest matched. Comparing every method against the previous run found 0 gained, 0 lost and no changed failure text. Snapshot `20260922T125708Z-196091` then included the Variant/SharedRecord and handler-lifecycle repairs; its all/test/GCC gates passed and all 2,310 AL identities, statuses and error texts remained identical, 2,197 green, 113 failed, 0 incomplete. Its source and seed hints matched the previous snapshot, while immutable seed identity remains unavailable. Frozen snapshot `20260922T133830Z-237456` passed `all`, `test` and `gcc`; UT correctly returned red because 113 AL methods still fail. Its source-counted manifest held 2,310 methods in 80 codeunits, with 2,197 passed and 0 incomplete. A method-by-method comparison against `20260922T125708Z-196091` found 0 missing, 0 added and 0 changes in status or error text. The source digest matched after the run, and the UT metadata now records BCApps revision `a9ea4d84534cebba852c44bf0f841c2ea149de4e`. Seed identity remains null for the legacy database, so this is repeatability evidence rather than a sealed-seed causal A/B. Runner isolation policy, seed provenance enforcement and the LibraryXMLRead slice addition postdate this snapshot. Frozen snapshot `20260922T141823Z-284533` passed `all`, `test` and `gcc`; its UT target returned red as required by the remaining failures. It reached the same source population, 2,310 methods in 80 codeunits and 0 incomplete, with 2,198 passed and 112 failed. The source digest matched after the run. Exact comparison against `20260922T133830Z-237456` found no missing or added identity; `Data Exch. Def UT::CheckLineTypesExport` gained because `LibraryXMLRead::Initialize(Text)` is now linked through the slice. The only other changed record retained its failure and differed solely in the generated transformation-rule GUID, so it is not a semantic gain. The immutable seed identity remains unavailable for this legacy seed.

Frozen snapshot `20260922T150719Z-329051` passed `all`, `test` and `gcc`; its UT target returned red only for the remaining AL failures. The independently source-counted manifest again held 2,310 methods in 80 codeunits with 0 incomplete, now 2,199 passed and 111 failed. The frozen and post-run source digests matched. Exact comparison against `20260922T141823Z-284533` found no missing or added identity and one gain: `ERM VAT VIES Lookup UT::ValidateVatRegistrationLogForContactCreatedByVendor`. Its earlier failure was the explicit out-of-slice refusal from `AgedAccPayableChart.OnOpenPage`; the generated user-control member and complete absent contract now make that page source link through the slice. The legacy seed still has no immutable identity, so the matching database hint is repeatability evidence rather than a sealed-seed causal A/B.

Frozen snapshot `20260922T164614Z-405873` passed `all`, `test` and `gcc` with the source digest unchanged. Its paired six-worker UT snapshot `20260922T170721Z-421118` measured the same independent population: 2,199 of 2,310 passed over 80 codeunits, 0 incomplete. Exact comparison with `20260922T150719Z-329051` found no changed identity or pass status. `ERM General Journal UT::TestCreateGenJnlBatchPostWebService` now reaches linked `LibraryGraphMgt` and reports the concrete unrebuilt `WebService.LockTable` member, replacing its earlier out-of-slice refusal; it remains a failure and is not counted as a gain.

The review repairs TextBuilder getter/setter separation and CRLF, temporary Field.Get,
reserved generated field names and the AL/.NET DateTime include distinction. Golden fixtures
were deliberately revised to the current namespace, shared-option and metadata-source contracts;
they were not copied from generator dumps. Tooling now preserves analyzer/symbol-reader failures,
counts gate sources instead of discovered binaries, builds C++ tests/lint without rebuilding the generated slice and checks builtin
reproduction in a temporary output tree. `make gates`, `make gcc` and `make ut` expose separate jobs.
Builtin ranking also uses the same 80/2,310 source manifest. Clang/GCC compiler caches cannot be silently mixed. Install dependencies are explicit for Debian 13.

`make gap` now creates the durable header census through `make tree` when it is absent, using its
configured `JOBS` value, then selects the highest-impact recorded root. The former fallback ran a
serial, uncached 9,870-header sweep and competed with integration work while reproducing a census
that `make tree` already owns. Explicit `SOURCE=1` and `SWEEP=1` remain exhaustive measurements.
Shell syntax and dry-run recipes confirm the two-job local path; the next frozen snapshot will
exercise the changed Makefile alongside the current generator/runtime repairs.
Invalid job limits are rejected before any compiler is started.

Full logs are local artefacts under `build/review-*.log`; reproduce them with the commands above.
The linked slice still reports 1,400 unimplemented procedure stubs. Green C++ gates do not establish
complete AL behaviour, production transaction durability, HTTP parity or aarch64 portability.

## Open items and consolidation

The old board had 481 numbered files (480 distinct IDs) plus a 3,908-line reading ledger,
44,094 lines in total. The 598-line project instructions mixed goals, historical measurements
and contradictory environment claims. They are replaced by concise standing rules.

The rows below are a migration map, not a claim that all absorbed requirements were already
implemented. Duplicate property/trigger descriptions now live under their runtime outcome;
completed mechanisms are described as existing evidence, and unproved remainder stays open.
The reading ledger and speculative C++ reflection task are removed; Git retains the old text.
Use `git show 136a5a7:board/<old-name>` or `git log --all -- board/` for the recorded experiments.
No IDs have been reissued and no stale active ownership marks survive.

| WI | Priority | Outcome | Absorbed IDs |
|---|---|---|---|
| [0004](0004_provisioning_will_record_source_and_seed_provenance_and_produce_usable_test_databases.md) | P1 | Provisioning will record source and seed provenance and produce usable test databases | 0613 |
| [0006](0006_session_ownership_and_performance_will_have_measured_portable_bounds.md) | P2 | Session ownership and performance will have measured portable bounds | 0008, 0009, 0596 |
| [0012](0012_production_commits_will_be_durable_and_concurrent_writes_will_be_checked.md) | P0 | Production commits will be durable and concurrent writes will be checked | 0060, 0077, 0079, 0087, 0267, 0458, 0463, 0490, 0621 |
| [0013](0013_system_fields_and_schema_keys_will_obey_their_declared_contracts.md) | P1 | System fields and schema keys will obey their declared contracts | 0080, 0353, 0371, 0511 |
| [0018](0018_sql_and_temporary_records_will_evaluate_the_same_filter_grammar.md) | P1 | SQL and temporary records will evaluate the same filter grammar | 0432, 0508, 0509, 0543 |
| [0019](0019_flowfields_will_preserve_semantics_and_use_bounded_aggregate_queries.md) | P1 | FlowFields will preserve semantics and use bounded aggregate queries | 0047, 0340, 0341, 0342, 0507, 0510, 0521, 0678 |
| [0030](0030_one_page_lifecycle_will_drive_testpage_and_the_http_ui.md) | P1 | One page lifecycle will drive TestPage and the HTTP UI | 0083, 0198, 0206, 0210, 0211, 0212, 0213, 0218, 0220, 0221, 0233, 0278, 0279, 0280, 0281, 0282, 0283, 0284, 0285, 0286, 0287, 0288, 0289, 0292, 0293, 0294, 0295, 0296, 0298, 0300, 0329, 0330, 0334, 0335, 0336, 0337, 0362, 0374, 0375, 0385, 0387, 0388, 0389, 0390, 0393, 0395, 0399, 0400, 0401, 0402, 0403, 0404, 0405, 0406, 0407, 0408, 0409, 0411, 0413, 0414, 0415, 0416, 0417, 0418, 0419, 0420, 0421, 0422, 0423, 0425, 0426, 0428, 0429, 0430, 0431, 0433, 0434, 0460, 0466, 0469, 0474, 0475, 0477, 0478, 0480, 0482, 0485, 0487, 0496, 0529, 0537, 0538, 0539, 0540, 0541, 0542, 0551, 0553, 0554, 0555, 0560, 0561, 0567, 0568, 0574, 0579, 0657, 0665, 0696, 0700, 0703 |
| [0033](0033_app_boundaries_and_extension_merges_will_be_explicit_and_enforced.md) | P1 | App boundaries and extension merges will be explicit and enforced | 0015, 0069, 0192, 0207, 0209, 0355, 0356, 0357, 0359, 0360, 0363, 0427, 0533, 0544, 0545, 0552, 0572 |
| [0034](0034_every_object_kind_will_have_a_truthful_translation_and_runtime_census.md) | P1 | Every object kind will have a truthful translation and runtime census | 0297, 0365, 0424, 0465, 0479, 0565, 0604, 0623 |
| [0035](0035_rebuilt_net_types_will_preserve_reference_and_culture_semantics.md) | P1 | Rebuilt .NET types will preserve reference and culture semantics | 0078, 0215, 0222, 0227, 0497, 0502, 0585, 0618, 0646, 0675, 0714 |
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
| [0063](0063_reports_will_execute_datasets_and_render_declared_layouts.md) | P2 | Reports will execute datasets and render declared layouts | 0301, 0302, 0303, 0304, 0305, 0306, 0307, 0308, 0309, 0391, 0396, 0397, 0436, 0450, 0451, 0452, 0454, 0455, 0457, 0459, 0486, 0488, 0489, 0546, 0547, 0549, 0557, 0575, 0576, 0577, 0628 |
| [0064](0064_queries_will_stream_correct_joins_filters_and_typed_aggregates.md) | P2 | Queries will stream correct joins, filters and typed aggregates | 0299, 0352, 0441, 0453, 0461, 0462, 0464, 0550, 0556 |
| [0065](0065_xmlports_will_preserve_schema_encoding_and_import_transaction_semantics.md) | P2 | XMLports will preserve schema, encoding and import transaction semantics | 0310, 0311, 0312, 0338, 0367, 0410, 0412, 0442, 0443, 0444, 0445, 0446, 0447, 0448, 0501, 0548 |
| [0066](0066_formatting_and_text_operations_will_follow_al_culture_and_character_rules.md) | P1 | Formatting and text operations will follow AL culture and character rules | 0007, 0010, 0016, 0041, 0053, 0075, 0082, 0323, 0324, 0326, 0437, 0438, 0440, 0491, 0503, 0527, 0632, 0716 |
| [0070](0070_install_and_upgrade_will_execute_declared_lifecycle_transactions.md) | P2 | Install and upgrade will execute declared lifecycle transactions | 0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277, 0500, 0534 |
| [0073](0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md) | P1 | Generated expressions will preserve AL types and evaluation effects | 0027, 0049, 0051, 0076, 0081, 0084, 0085, 0086, 0088, 0089, 0467, 0468, 0524, 0531, 0558, 0573, 0578, 0581, 0584, 0587, 0590, 0597, 0608, 0614, 0629, 0633, 0695 |
| [0074](0074_streams_and_media_will_have_explicit_ownership_and_persistent_storage.md) | P2 | Streams and media will have explicit ownership and persistent storage | 0031, 0200, 0494, 0532, 0535 |
| [0090](0090_background_work_will_have_database_backed_ownership_and_session_isolation.md) | P2 | Background work will have database-backed ownership and session isolation | 0290, 0291, 0498, 0536 |
| [0589](0589_the_toolchain_will_be_reproducible_and_every_gate_will_fail_reliably.md) | P0 | The toolchain will be reproducible and every gate will fail reliably | 0005, 0046, 0050, 0616 |
| [0718](0718_record_images_and_temporary_handles_will_survive_copies_without_dangling_references.md) | P0 | Record images and temporary handles will survive copies without dangling references | 0037, 0042, 0526, 0624, 0640 |
| [0719](0719_error_message_drilldown_will_retrieve_the_logged_record_context.md) | P1 | Error-message drilldown will retrieve the logged record context | — |
