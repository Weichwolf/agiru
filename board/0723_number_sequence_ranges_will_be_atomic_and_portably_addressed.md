# 0723 — Number-sequence ranges will be atomic and portably addressed

Status: open | Priority: P0 | Stage: UT primitive; Clients multi-user safety | Reviewed: 2026-10-03
Depends on: 0006 connection ownership; 0013 company identity.

## Evidence

- Current full UT: 2,062/2,314, zero incomplete/crashed; 116 direct aborted-transaction
  failures. PostgreSQL logs identify missing sequence errors before the cascades.
  Original `NoSeriesSequenceImpl.Codeunit.al::{GetCurrentSequenceNo,GetNextNoInternal}`
  catches Current/Next through TryFunction and then calls Exists/Insert; server RAISE
  poisoned that recovery. The primitive now returns zero rows for a missing identity,
  releases its allocator lock and raises the original AL error in C++; no extra RPC,
  broad rollback or commit. `make number-sequences JOBS=2`: 396 checks, 300 cross-process
  ranges and all controls pass; restoring server RAISE fails the prior-write/recovery
  check; four extra checks reject leaked allocator locks inside an open transaction.
  `/tmp/agiru-ut-recovery-sequence-controls-final.log`; full replay
  `20261003T132242Z-466167`: 2,159/2,314, 155 failed, zero incomplete/crashed,
  zero aborted transactions; 97 gains/zero losses against the same previous 2,314
  identities (diagnostic legacy seed, not sealed A/B proof). Other SQL
  errors, including caught duplicate Insert/range errors, still need statement-error
  recovery; do not claim complete TryFunction transaction parity. Targeted analysis adds
  no finding to NumberSequence.cpp; existing Char.h diagnostics remain red.

- `NumberSequence.cpp` binds one seven-parameter request; Next and both Range forms share checked, constant-work allocation. PostgreSQL owns real sequences and a unique `(company_specific, company, name)` registry with numeric physical IDs; no persisted C++ hash or string-literal identity interpolation.
- `NumberSequenceStorage.cpp` separates shared transaction-lifetime guards from short exclusive allocation locks; transactional Insert/Delete/Restart use exclusive lifetime guards. Exists does not lock out concurrent creation. Session locks release on success/error/cancellation; the acquisition is inside the exception boundary. Exact numeric arithmetic checks the entire range before setval; CACHE 1/NO CYCLE are enforced.
- `make test JOBS=2`: 110 local cases/149 toolchain tests green, zero skipped; 376 direct sequence checks. Three sessions mix 200 Range(100) and 100 Next requests; three processes reserve 300 further ranges. Signed steps, bounds, rollback, restart/delete races, arbitrary names, scope framing, search-path shadows and reprovisioning pass. Controlled cancellation occurs after acquiring the allocator lock. One-value and billion-value reservations each use one client RPC.
- Anchored mutants remove serialization, restore per-value requests or interpolate the name: every control fails; ordinary-name and delayed-but-serialized controls pass. The harness restores the production functions even on failure. Receipts: `build/number-sequence-{tests-final.log,controls-final.log}`, `build/number-sequences.xNBiYS/`. Independent UT identities retain 80/2,314; raw upstream refresh retains all 113,005 previous methods and adds eight (0058). No AL execution/G1/scale claim.
- Provisioning refuses legacy public `NumSeq$` storage rather than silently starting parallel counters. Mapping migration, company rename semantics and populated-runner qualification remain open. The sealed master was not opened or changed.

## Implementation

1. Inventory legacy identities from a disposable sealed-source clone. Require an explicit logical identity→qualified legacy relation manifest; reject ambiguous company delimiters/hash-only identities. Preserve current value, called state and increment; migrate atomically with backup/rollback and collision controls, never infer identity from a truncated name.
2. Tie company-specific identities to the stable company contract (0013); prove rename/duplicate-name behaviour before changing the persisted registry version.
3. Prove cancellation while waiting, concurrent lifecycle-lock upgrades, supported isolation levels and role permissions; investigate spurious unlock warnings and retry boundaries without swallowing the original error. Add structural schema/version refusal controls.
4. Run generated No. Series/Sequence No. Mgt. workflows on disposable databases and the unchanged full UT population once 0038 compiles. Measure PostgreSQL/server/session lock and memory costs, then prove aarch64 and the separate WASM adapter; current receipts are native x86_64 only.

## Acceptance

- Legacy migration preserves identity/state and refuses ambiguous mappings; company rename cannot orphan or alias sequences.
- No leaked lock or replaced AL error under cancellation/DDL upgrades/isolation/role failures; schema upgrades refuse incompatible layouts.
- Generated business workflows and all source-counted UT pass; compiler-independent persisted identity and bounded SQL/resource use hold on both production architectures. Keep the existing concurrency/rollback/name/mutant gates green.

## References

Code: `src/rt/{NumberSequence,NumberSequenceStorage,Storage}.cpp`, `include/{type/NumberSequence,runtime/NumberSequenceStorage}.h`, `test/gate/NumberSequenceGate.cpp`, `test/runtime/number-sequences.sh`. Platform docs at `ff5939a46e`: `devenv-number-sequences.md`, every `methods-auto/numbersequence/` overload and `methods-auto/biginteger/biginteger-data-type.md` (the indirect signed minimum is valid). AL at `bb7111877f`: `src/Layers/W1/BaseApp/Foundation/NoSeries/SequenceNoMgt.Codeunit.al::{TryGetRange,CreateSequence}`; `src/Layers/W1/Tests/ERM/TestSequenceNoMgt.codeunit.al`. User docs at `0ff62b2266`: `business-central/ui-create-number-series.md`; this primitive does not replace gapless business number-series rules. Predecessor board: 1158 (var Increment), 819 (seed/Next evidence), 976 (database-owned authority).

PostgreSQL 17: [sequence state/rollback](https://www.postgresql.org/docs/17/functions-sequence.html), [transactional restart](https://www.postgresql.org/docs/17/sql-altersequence.html), [exception conditions](https://www.postgresql.org/docs/17/plpgsql-control-structures.html#PLPGSQL-ERROR-TRAPPING). SQL Server: [range bounds](https://learn.microsoft.com/en-us/sql/relational-databases/system-stored-procedures/sp-sequence-get-range-transact-sql), [nonzero signed increment](https://learn.microsoft.com/en-us/sql/t-sql/statements/create-sequence-transact-sql). The SQL lock mask is `2^63−2`: even lifetime and odd allocation keys; hashes coordinate locks only, never persisted identity.
