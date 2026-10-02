# 0090 — Background work will have database-backed ownership and session isolation

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

The current Session opens a connection and thread-local state; there is no complete task/session service. Scheduled reports and page background tasks cannot be implemented by spawning a thread over the caller's mutable Session.

## Implementation for Sol

1. Implement child sessions with explicit immutable launch context and separate transactions/connection leases. Page background tasks are read-only and must reject locks/writes.
2. Persist scheduled task state, claims, retries and failure-codeunit transitions in PostgreSQL. Use leases/transactional claims so two service tiers cannot execute the same claimed work concurrently.
3. Respect cancellation, page lifetime, timeout and completion/error callbacks through the page lifecycle. Marshal only supported parameter/result types.
4. Declare at-least-once retry behaviour and deterministic result ordering; do not claim exactly-once execution across process failure.

## Acceptance

Two workers/tiers race for a task, one crashes, another recovers the lease. Read-only task writes fail, cancellation cannot update a closed page, and failed work rolls back before retry.

## References

Platform: devenv-page-background-tasks.md, Session/TaskScheduler methods and scheduled-task lifecycle. AL: task consumers. Predecessor: consult retry findings, not Python thread/context infrastructure.
