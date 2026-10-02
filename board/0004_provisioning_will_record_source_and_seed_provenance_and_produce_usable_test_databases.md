# 0004 — Provisioning will record source and seed provenance and produce usable test databases

Status: open | Priority: P0 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0013 schema contract; 0589 reproducible inputs.

## Evidence

- `RunnerDatabase.cpp::RunnerDatabase` executes `DROP DATABASE` for any supplied `--scratch` with `--fresh`; no master/template/ownership guard. Reusing an existing scratch also accepts arbitrary contents.
- `agiru_seeded`: template=true, connections allowed, no provenance row (2026-09-28 inspection). It supports a diagnostic rerun, not a sealed-seed A/B.
- `seed_demo.py` records building/complete identity and checks both transfer processes. `provision.sh` still ends with the obsolete transfer-not-implemented message.

## Implementation

1. Parse DSNs with libpq, resolve actual database identities, then validate scratch ownership before DROP or reuse. Reject the source database, every template, maintenance databases and unowned existing databases; generate unique run-owned names.
2. Build a fresh seed from immutable imported CRONUS data. Record BC_VERSION/checksum, BCApps revision, company, scope, schema hashes, unmapped columns and transfer counts; only zero-refusal completion may seal it.
   Include System.app package identity/runtime/symbol hash separately from the demo version and BCApps commit. Validate field/enum/schema compatibility rather than assuming all version strings coincide.
3. Support provenance inspection through maintenance metadata when template connections are disabled; clone only a complete sealed identity. Do not retrofit identity onto the legacy seed.
4. Make provisioning resume by checked stages. Populate AllObj, Field and Page Metadata independently; an existing AllObj row must not skip the others. Record work-date policy.

## Acceptance

- Disposable fixtures: source=scratch, template target, unowned existing scratch and interrupted transfer refuse before mutation.
- Two fresh clones share the sealed identity and schema/data checksums; interrupted provisioning cannot advertise completion; cleanup removes only run-owned databases.

## References

Code: `scripts/{provision.sh,seed_demo.py,ut_milestone.py}`, `src/rt/{RunnerDatabase,Storage}.cpp`. Predecessor: WI-832 (held-open master), WI-847 (work date).
