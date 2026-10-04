# 0061 — Consumed and discarded calls will keep AL error semantics

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-10-04
Depends on: 0073 resolved value context; 0012 separate error boundaries.

## Evidence

- Runtime now borrows lazy table-global identity only for var-Record Run, through a
  noncopyable scoped guard; success/error restores callee-owned globals. Ordinary
  copy/assignment remains separate, and caller filters/cursors retain their existing
  contract. Globals uses composition to preserve standard layout; its handle adds
  one pointer (24→32 bytes on this host), without eager allocation or session globals.
  `make codeunit-record JOBS=2`: InstanceGate 49 and generated AL 60 checks green;
  typed/static/dynamic consumed/statement forms preserve saved rows through rollback
  and restore them to SQL. Nested calls, handles and temporary cursors execute.
  Three compiled no-borrow/no-restore/assignment-alias controls reject
  (`/tmp/agiru-scoped-globals-generated-final.log`, `/tmp/agiru-codeunit-record.DtRHXd`).
  Final complete local Make replay retains 126 cases/223 tooling tests, all green
  (`/tmp/agiru-scoped-globals-local-tests.log`, `/tmp/agiru-codeunit-record.rr4usJ`).
  The new runner passes targeted analysis; InstanceGate has zero own findings,
  with 38 header findings retained, including the unchanged no-op Globals assignment.
  Completed activation `20261004T044804Z-1552154`: 2,161/2,314, 153 failed,
  zero incomplete; no identity/pass changes versus 023609. Two Incoming Document
  diagnostics now reach missing Table Metadata; `/tmp/agiru-scoped-xml-ut-comparison.json`.
  Original replay `/tmp/agiru-incoming-scoped-20261004.{log,jsonl}` retains 12/32:
  saved error 26 is restored as SQL row 27 after rollback al_621 and selected by
  Status drilldown. `/tmp/agiru-incoming-scoped-page-gdb-20261004.log` reaches
  the second, card-part assertion; the first Error Messages trap now works.
  0030 owns the missing system Edit/CardPageId navigation. This proves original
  restoration, not a full UT pass gain or sealed causal A/B.
- SQL replay `/tmp/agiru-incoming-sql-20261004-0345.{log,jsonl}` (023609 image,
  12/32) localizes the first Error Messages trap to post-Codeunit.Run restoration:
  error ID 20 exists and is read before rollback; no CopyFromTemp insert follows.
  0030 holds the exact SQL/context evidence. TakeIn_ copies fields/filters but
  Globals intentionally preserves separate variable blocks; failed Ok_Run never
  calls GiveBack_. The scoped implementation above addresses this candidate without
  global assignment changes or business-object-specific repairs; the original replay above confirms restoration.
- Consumed typed Record.SetCurrentKey calls lower to Ok_SetCurrentKey, including
  table-local implicit Rec; discarded calls retain the raising method. Blob and
  FlowFilter failures return false only when consumed, without swallowing missing
  fields or unsupported FlowField behaviour. The original callee still supplies
  field-argument binding. Generated AL: 48 checks green, six compiled call-context
  controls plus three common key-selection and one source-expression control reject
  (`/tmp/agiru-current-key-local-tests.log`, `/tmp/agiru-test-contexts.VFwyFa`).
  Actual unindexed AL field/implicit PK confirmed; full local replay passes
  124 cases/223 tooling tests. Runner analysis passes; the new call helpers have
  no own findings. Full generation changes only 44 consumed calls in eight files,
  with no path losses and unchanged 80/2,314 UT identities. The key-selection fix
  belongs to 0044 and is outside snapshot 011421. Completed 023609 keeps
  2,161/2,314, zero identity/pass losses or gains; four SCM planning diagnostics
  change to missing Inventory Profile 3 (`/tmp/agiru-current-key-ut-comparison.json`).
  Trace these changed paths in 0044; no full UT pass gain is claimed.
- Shared procedure-attribute lookup now resolves own table/page calls, implicit table
  Rec/xRec, page source-table Rec and codeunit this. Explicit per-expression ValueUse
  replaces the leaking discarded-call flag; nested arguments remain consumed even
  when the outer call is discarded or wrapped by Tried.
- Generated AL execution: 40 checks green, including exact last errors, field mutation,
  discarded propagation, early exit, assignments/conditions/unary/exit/case, nested
  catches and argument errors preventing the outer body. Case selectors are now bound
  once, including ordinary side-effectful selectors across range/else branches.
  Four compiled call-context mutants plus the existing source-expression control fail
  (`/tmp/agiru-call-context-final-controls.log`, proof
  `/tmp/agiru-test-contexts.gdp2R7`). This does not prove database write policy.
- The former 154-line Call emitter is split into typed/filter/field argument helpers;
  new call/argument helpers have no targeted tidy findings. Runner targeted analysis
  passes (`/tmp/agiru-call-context-runner-final-lint.log`); inherited generator/header
  findings remain unsuppressed. Case conversion rules still need their own proof.
- Frozen native/scope integration: 2,161/2,314, 153 failures, zero incomplete;
  unchanged identities/statuses/errors versus its predecessor. It predates this fix.
- Frozen Incoming Doc. To Data Exch.UT trace: 12/32 pass; early exit already emits
  true. Optional mapping-field errors are caught; they are not yet proven causes of
  conversion failures. Retain `/tmp/agiru-incoming-doc-trace-before.{log,jsonl}`.
- Current full regeneration retains all 80/2,314 source identities and 14,225 slice
  inputs; original IncomingDocument now calls UpdateDocumentFields through Tried.
  The complete 123-case/223-tooling local replay passes. Frozen activation run
  `20261004T011421Z-1315211` passes build/local tests; terminal UT is 2,161/2,314,
  153 failed, zero incomplete. No identities or pass statuses change; only
  TestProcessWithDataExchSucceeds changes to "The TestPage is not open."
  (`/tmp/agiru-call-context-ut-comparison.json`). Post-activation trace stays
  12/32; pinpoint the unmet Trap in its expected-error workflow (0030), not a
  permissive unopened-page workaround. The unsealed seed does not prove causal A/B.

## Implementation

1. Inspect the frozen build and replay the unchanged 80/2,314 UT population. Trace
   Incoming Doc. To Data Exch.UT after activation; classify original errors, not summary assertions.
   Scoped restoration and the first error-list Trap are proved above. Replay the
   later card-navigation batch in 0030 against all 2,314 identities; investigate
   gains/losses and changed diagnostics, including the four planning paths in 0044.
   Preserve caller cursors, per-variable globals and session isolation.
2. Extend execution coverage to loops, bare calls,
   report/XMLport scopes and nested receivers. Resolve overloads from the actual
   callee symbol/type, not a name-only attribute set; chained calls remain unproved.
3. Do not insert a savepoint around TryFunction: documented write behaviour differs from Codeunit.Run and asserterror. Keep the three mechanisms separate.
   Make DisableWriteInsideTryFunctions an explicit runtime policy: on-premises default rejects writes in consumed try calls, online/explicitly allowed mode does not roll them back. Argument evaluation belongs inside the consumed-call catch/write-policy boundary; discarded calls use ordinary rules.
4. Extend the same consumption model to the remaining Boolean Record/File/XML operations, preserving only their documented failure-to-false cases. Keep the SetCurrentKey execution/negative controls above.

## Acceptance

- Generate/execute every consumption context. In write-allowed mode, a write before a caught error survives; in write-disabled mode it refuses before mutation, including argument-side writes. Discarded calls raise normally; last-error replacement remains exact. Run Incoming Doc. To Data Exch.UT as an activation A/B.

## References

Code: `src/gen/BodyWriter.cpp`, `include/runtime/Error.h`, `include/runtime/Codeunit.h`.

Platform: devenv-handling-errors-using-try-methods.md and attributes/devenv-tryfunction-attribute.md. AL: incoming-document conversion and table-local TryFunctions. Predecessor: WI-1141 and prior value-context findings; obsolete rollback advice from 0226 is rejected.

Revisions: developer docs `ff5939a46e05`; BCApps `bb7111877ff7`; user docs
`0ff62b2266fd` (no additional TryFunction guarantee). Also read
`devenv-al-this-keyword.md` (codeunit-only this), BCApps
`src/Layers/W1/BaseApp/eServices/EDocument/IncomingDocument.Table.al` and
predecessor 1056 (early exit already correct) / 1141 (caught last error).
Case authority: `devenv-al-control-statements.md#case-statements`; predecessor
810 K6 (repeated selector evaluation, bind once).
Execution fixtures: `test/runtime/test-contexts/{TryScopes.Table.al,TryScopes.Page.al,
Fixture.Codeunit.al,Runner.cpp}`; orchestration/controls: `test/runtime/test-contexts.sh`.

var-Record: developer `methods-auto/codeunit/codeunit-run-integer-table-method.md`;
original IncomingDocument.Table.al::SaveErrorMessages/CreateWithDataExchange and
IncomingDocWithDataExch.Codeunit.al::RollbackIfErrors. Runtime:
`include/runtime/Codeunit.h::{Globals,TakeIn_,GiveBack_}`, `include/runtime/Table.h::Copy`.
Predecessor 1204 maps the temporary-save/rollback/restore chain but identifies a
different failure; its rejected FindSet workaround lost 49 tests. Do not repeat it.

Authored execution: `test/runtime/codeunit-record/{Row.Table.al,Buffer.Table.al,
Writer.Codeunit.al,Nested.Codeunit.al,Forms.Codeunit.al,Runner.cpp}`;
`test/runtime/codeunit-record.sh` is registered in `make test` and independent discovery.
