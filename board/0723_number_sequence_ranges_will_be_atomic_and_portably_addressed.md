# 0723 — Number-sequence ranges will be atomic and portably addressed

Status: open | Priority: P0 | Stage: UT primitive; Clients multi-user safety | Reviewed: 2026-09-28
Depends on: 0006 connection ownership; 0013 company identity.

## Evidence

- `NumberSequence::Range` loops over Next. Two concurrent Range(100) calls returned [4,103] and [1,100]: overlapping reservations. The dedicated gate fixture was removed after the probe.
- Next interpolates a quoted identifier inside a SQL string literal; a valid name containing an apostrophe causes SQL syntax failure.
- Long persisted sequence names use std::hash; lookup ignores schema. Reproduction: `build/review-20260928/{BoundaryProbe.cpp,boundary-probe.log}`.

## Implementation

1. Replace per-value RPCs with one bounded atomic range-reservation operation. All Next/Range/Restart/Delete paths share the same PostgreSQL coordination protocol; never protect only Range while Next bypasses it.
2. Candidate: a platform SQL function with a per-sequence session advisory lock released on success/error, checked range arithmetic and validated sequence-cache policy. Preserve nontransactional consumption without holding the lock until the caller's posting transaction commits. Prove creation/rollback/overflow behavior before using setval or a separate allocation connection.
3. Bind resolved schema-qualified sequence identity as a regclass parameter; never interpolate it into a string literal. Persist an explicit name/company-to-identity mapping or a versioned stable digest with collision checks; do not use std::hash as a storage contract.
4. Read each overload independently: default seed, increment, Current, Restart, Range's var Increment and company scope. Preserve the platform primitive; do not rewrite business No. Series rules.

## Acceptance

- Two sessions reserve disjoint ranges while a third calls Next; consumed numbers are not reissued after caller rollback. Include increments other than one, overflow, invalid count and restart/delete races.
- Apostrophe/Unicode/long names and equal names in two companies work across processes, compiler families and x86_64/aarch64. Assert bounded SQL round trips independent of Count.
- Reintroducing the per-value loop or unbound SQL name fails dedicated gates; run unchanged full UT population.

## References

Code: `src/rt/NumberSequence.cpp`, `include/type/NumberSequence.h`, `src/db/Connection.cpp`. Platform: `devenv-number-sequences.md`, `methods-auto/numbersequence/numbersequence-{insert,next,current,restart}-method.md` and both Range overload files. AL: `Layers/W1/BaseApp/Foundation/NoSeries/SequenceNoMgt.Codeunit.al::TryGetRange`. Predecessor: WI-1158 (var Increment), WI-819 (do not guess disputed seed/Next behavior), WI-976 (database-owned state).
