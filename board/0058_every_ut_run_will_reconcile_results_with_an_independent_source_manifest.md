# 0058 — Every UT run will reconcile results with an independent source manifest

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

The review added `scripts/ut_manifest.py` and repaired milestone aggregation: the current AL source contains 80 UT codeunits / 2,310 Test procedures; totals come from this manifest, and missing summaries, mismatched totals and failing processes remain red. Fixture controls prove that a runner reporting 1/1 cannot pass a source population of 2, and that exit 42 cannot pass after printing 2/2. The in-progress implementation adds --results-jsonl, a caller-owned reporting context, and scripts/ut_results.py to reconcile every method identity. Twenty-eight toolchain controls and the 15-check TestReportGate pass; the latest local make test run reported 71 programs/scripts with 0 red. `scripts/ut_milestone.py` retains per-codeunit artifacts and verifies timeout, interrupt and database cleanup in negative fixtures. An attempted AL run with an old slice and newly rebuilt runtime crashed every completed codeunit at `PQclear`; it was stopped and marked 0/2,310 with 80 incomplete. The runner now checks Ninja's dry-run graph before starting, and a real stale-image control refused that same mixed build. Source, manifest and linked-image hashes are recorded. The current seeded database has no durable seed-identity record, so provenance remains incomplete until WI 0004 supplies it. A coherent frozen snapshot is building and will run the full milestone. Result serialization stays in rt (which reaches net), so CLI keeps its declared dependency boundary.

## Implementation for Sol

1. Keep the independent source manifest as the only denominator. `make builtins` now consumes the same manifest. Fixtures cover multiline attributes, quoted names, unparsed Test declarations, duplicate methods and duplicate IDs. The current 80 UT codeunits contain no `#if` directives (source scan, 2026-09-22); the scanner counts text in every branch if that changes. Before filtering a conditional branch, define and record the compiler symbols of the generated image so the source denominator cannot silently shrink.
2. Match every reported method to this manifest. Missing, duplicate, unexpected and crashed cases are explicit failures. Compare the reported total even for a process that printed a summary.
3. Keep the new process-group termination and retained artifacts under negative controls. Complete cleanup for startup failures and a second interrupt; ensure every abnormal exit leaves a status file and does not leave a scratch database.
4. Keep recorded BCApps revision, agiru revision, source/image hashes, work date, command and sorted per-method results. WI 0004 must write a durable identity for the seeded database; a database OID and size are recorded only as a snapshot hint. Keep `make ut` as the entry point.

## Acceptance

Fixtures cover a lost parse, a missing linked method, duplicate method results, exit failure after a printed total, timeout, zero population and two names that sanitize to the same filename. The first six must make the aggregator red without lowering the expected denominator; the two-name control must keep both distinct and green.

## References

Platform: devenv-test-codeunits-and-test-methods.md. AL: Layers/W1/Tests, Subtype=Test with names ending UT/-UT/.UT. Predecessor: WI-1088 warns that measurement shape itself changes results.
