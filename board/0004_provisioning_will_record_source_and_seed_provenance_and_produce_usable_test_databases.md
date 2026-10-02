# 0004 — Provisioning will record source and seed provenance and produce usable test databases

Status: open | Priority: P0 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0013 schema contract; 0589 reproducible inputs.

## Evidence

- Integrated scratch guards reject source/maintenance/template targets, unmarked databases, foreign owners, stale handles and identifier truncation. Acquisition is serialized in PostgreSQL. Clang: 80 local gate cases green; GCC: ConnectionInfo 16/16, ScratchGuard 18/18, RunnerDatabase 12/12. Legacy negative control: 4 red; all newly created fixture databases/roles were removed.
- Changed-code analysis: 164/180 available units, 161 failed. New nullable-access and argument-identity findings were repaired; targeted RunnerDatabase/ScratchGuard checks now report only the pre-existing common Error.h finding. Full frozen integration of the scratch patch is pending; the live snapshot predates it.
- `agiru_seeded`: template=true, connections allowed, no provenance row (2026-09-28 inspection). It supports a diagnostic rerun, not a sealed-seed A/B.
- `seed_demo.py` records building/complete identity and checks both transfer processes. `provision.sh` still ends with the obsolete transfer-not-implemented message.

## Implementation

1. Verify the integrated guards in the next frozen run. Preserve unique per-run names; do not adopt unmarked legacy databases. Canonical libpq keyword serialization replaces the effective database, including URI query overrides, without echoing credentials in parse errors.
2. Build a fresh seed from immutable imported CRONUS data. Record BC_VERSION/checksum, BCApps revision, company, scope, schema hashes, unmapped columns and transfer counts; only zero-refusal completion may seal it.
   Include System.app package identity/runtime/symbol hash separately from the demo version and BCApps commit. Validate field/enum/schema compatibility rather than assuming all version strings coincide.
3. Support provenance inspection through maintenance metadata when template connections are disabled; clone only a complete sealed identity. Do not retrofit identity onto the legacy seed.
4. Make provisioning resume by checked stages. Populate AllObj, Field and Page Metadata independently; an existing AllObj row must not skip the others. Record work-date policy.

## Acceptance

- Disposable fixtures: source=scratch, template target, unowned existing scratch and interrupted transfer refuse before mutation.
- Two fresh clones share the sealed identity and schema/data checksums; interrupted provisioning cannot advertise completion; cleanup removes only run-owned databases.

## References

Code: `scripts/{provision.sh,seed_demo.py,ut_milestone.py}`, `src/rt/{RunnerDatabase,Storage}.cpp`. Predecessor: WI-832 (held-open master), WI-847 (work date).

Guard code: `include/runtime/ConnectionInfo.h`, `src/db/ConnectionInfo.cpp`, `test/gate/{ConnectionInfo,ScratchGuard,RunnerDatabase}Gate.cpp`. Ownership comment: `agiru.runner.v1.from.<source OID>` plus current PostgreSQL role and acquired target OID. Session lock: `hashtextextended('agiru.runner:' || name, 0)`; zero is the protocol seed, collisions only serialize unrelated names, connection closure releases the lock. This coordinates cooperating calls, not malicious privileged DDL. Physical source OID is not sealed seed/data provenance.

PostgreSQL 17: [libpq connection parsing](https://www.postgresql.org/docs/17/libpq-connect.html), [database identities and owners](https://www.postgresql.org/docs/17/catalog-pg-database.html), [shared object comments](https://www.postgresql.org/docs/17/functions-info.html), [session advisory locks](https://www.postgresql.org/docs/17/explicit-locking.html#ADVISORY-LOCKS).
