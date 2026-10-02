# 0090 — Background work will have database-backed ownership and session isolation

Status: open | Priority: P3 | Stage: All | Reviewed: 2026-09-28
Depends on: 0006 sessions/leases; 0012 transactions; 0062 authorization; 0030 callback lifetime.

## Evidence

- No complete task/session service exists. Reusing the caller's Session on a background thread violates ownership.

## Implementation

1. Use PostgreSQL claims with owner/lease/fencing generation, bounded worker concurrency and durable attempts. Fence external side effects or require idempotency; expired leases alone cannot stop a stalled worker.
2. Implement child sessions with explicit immutable launch context and separate transactions/connection leases. Page background tasks are read-only and must reject locks/writes.
3. Persist scheduled task state, claims, retries and failure-codeunit transitions in PostgreSQL. Use leases/transactional claims so two service tiers cannot execute the same claimed work concurrently.
   Check the fencing generation inside each effect transaction and prevent ownership replacement while that transaction commits. A stale worker must not write after lease takeover; external effects need their own idempotency/fencing contract.
4. Respect cancellation, page lifetime, timeout and completion/error callbacks through the page lifecycle. Marshal only supported parameter/result types.
5. Declare at-least-once retry behaviour and deterministic result ordering; do not claim exactly-once execution across process failure.

## Acceptance

- Two workers/tiers race for a task, one crashes, another recovers the lease. Read-only task writes fail, cancellation cannot update a closed page, and failed work rolls back before retry.

## References

Code: `src/rt/Builtins.cpp`, `include/runtime/Page.h`, `src/rt/Session.cpp`.

Platform: devenv-page-background-tasks.md, Session/TaskScheduler methods and scheduled-task lifecycle. AL: task consumers. Predecessor: consult retry findings, not Python thread/context infrastructure.
