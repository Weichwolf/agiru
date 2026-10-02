# 0058 — Every UT run will reconcile results with an independent source manifest

Status: open | Priority: P0 | Stage: UT; All population expansion | Reviewed: 2026-09-28
Depends on: 0589 trustworthy execution; 0004 sealed seed for acceptance.

## Evidence

- `ut_manifest.py` counts AL text independently, but selects only UT-suffixed codeunits under W1 Tests. This is the milestone, not the whole AL suite.
- `ut_results.py` reconciles per-method identities and process exits; direct CLI totals still come from linked registrations.
- Current measurement, revision and artifact paths belong in README only; old run narratives are removed.
- Full-tree text audit finds 19 UTF-16 AL files and 53 `[Test] internal procedure` methods the current matcher cannot reconcile. These are outside the measured UT set, not permission to omit them from the final manifest.

## Implementation

1. Keep the UT source manifest authoritative. Record app + kind + ID + method, source/manifest/generated-image hashes, compiler symbols, runner/isolation, work date and seed identity.
2. Expose the expected manifest to the CLI or its mandatory verifier; label deliberately filtered diagnostics. A direct linked-only total must never claim the full milestone.
3. Report missing, duplicate, refused, crashed, timed-out, skipped and unexpected results separately; none leaves the expected population. Retain process failures even after a printed success summary.
4. Keep process-group cancellation, startup/second-interrupt cleanup and per-run artifacts under negative controls. Reconcile queued and running methods after interruption.
5. After 0720 parity, add an explicit full-suite manifest over all test apps/codeunits, without the UT name suffix. Inventory omitted app roots independently of apps.json; exclusions remain visible ERP gaps. Preserve the UT manifest as a separate regression set.
   Decode declared/BOM encodings and recognize internal procedures independently of the generator. Classify localization overlays and compiler-symbol branches by build variant; retain source identities and reasons rather than collapsing duplicate IDs or adding mutually exclusive variants to one executable run.

## Acceptance

- Lost parse/link/method, duplicate result, timeout, exit 42 after green summary, zero population and interrupted queued work all fail with unchanged totals.
- UT gate: every expected method passed, zero incomplete/refused/crashed/skipped. Final gate: the same rule over the complete AL test population and client-driven workflows.
- Status comparison lists every gain/loss and identity change. Sealed provenance is required for causal A/B; legacy-seed runs are diagnostic only.

## References

Code: `scripts/{ut_manifest.py,ut_results.py,ut_milestone.py,verify_snapshot.py}`, `src/{cli/Main.cpp,rt/TestRunner.cpp}`, `test/{toolchain.py,gate/TestReportGate.cpp}`. Platform: `devenv-test-codeunits-and-test-methods.md`. Predecessor: WI-1088; runner policy changes measured outcomes.
