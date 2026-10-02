# 0721 — Equivalent ERP workloads will prove lower latency and resource cost

Status: open | Priority: P3 | Stage: All; baseline design before optimization | Reviewed: 2026-09-28
Depends on: G2 client parity; 0058 full-suite census; 0006 session bounds; 0045 bounded reads; 0012 concurrency.

## Evidence

- No matched BC/agiru ERP benchmark, 2 TB run, 10,000-user result or aarch64 result is recorded. C++ and PostgreSQL alone establish no performance advantage.
- Predecessor observations: static test metadata cut discovery RSS 615→418 MB; lazy Python object loading cut image 812→405 MB; a concurrent CI/server/gate run was killed by memory pressure. None measures agiru or BC. Measure whether agiru's generated read-only metadata is loaded/paged on demand and whether page rendering repeats metadata probes or row fetches.

## Implementation

1. Define workload/data manifests for sales/purchase posting, journals, inventory/warehouse, filtered navigation, imports and reports. Match BC version, enabled features, hardware, durability, dataset, concurrency and think time; record platform differences.
2. Measure release builds with AGIRU_FAST_LOOP=OFF on x86_64/aarch64. Record latency p50/p95/p99, throughput, CPU/request, RSS/PSS, marginal session memory, DB calls/bytes, allocations, WAL, lock waits and pool occupancy.
   Report whole-stack totals as well as service/DB/renderer splits; no apparent saving by moving work into PostgreSQL, FOP or a sidecar. Include browser payload/render latency separately from the server comparison.
3. Measure idle logged-in users separately from active transactions; scale towards 10,000 users and 2 TB with skewed/hot keys. Include cold/warm cache, contention, long reads and failure/recovery.
4. Proposed qualification target: at least 2x business throughput and at most 50% service CPU/request and marginal session memory versus matched BC, with no p95 latency regression. Treat these as proposed thresholds pending baseline feasibility, not measured promises.
5. Use equivalent SQL to locate runtime overhead; profile before optimizing Decimal, aggregation, allocation or code layout. Preserve exact amounts, durable commits and bounded reads.
6. Record repeated trials/confidence intervals, bottlenecks and resource limits. Retain executable commands and raw results under build/; maintain small versioned workload specifications under test/.

## Acceptance

- Full AL suite and posting invariants pass at each qualified workload/concurrency. Independent users/companies remain isolated; deterministic reconciliation matches expected ledger totals.
- No claimed speedup from narrower semantics, missing work, warmed-only runs or disabled durability. Without a comparable BC baseline, report agiru measurements and leave superiority unproved.
- Restart, connection loss, competing writers and exhausted pool preserve correctness and bounded resource use.

## References

Code: `CMakeLists.txt::AGIRU_FAST_LOOP`, `src/rt/{Cursor,Navigate,Storage,Session}.cpp`, `src/net/Decimal.cpp`. Related: 0019 aggregates, 0064 queries, 0065 XMLports, 0090 workers. Split from 0006; preserves absorbed 0008/0009/0596 requirements.
