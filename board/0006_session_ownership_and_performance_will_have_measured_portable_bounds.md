# 0006 — Mutable runtime state will belong to the session

Status: open | Priority: P0 | Stage: UT session correctness; Clients ownership | Reviewed: 2026-09-28
Depends on: 0012 transaction lease contract.

## Evidence

- `Session::~Session` clears thread-owned automatic/SingleInstance maps. A nested session shares and then destroys its parent's instances.
- Mutable thread locals also hold manual event bindings, handlers, page traps, error/commit scopes, validation context and random state. `Session` holds a PostgreSQL connection for its entire lifetime.
- Reproduced: nested child reads parent value 42, frees its instance and leaves parent value 0 (`build/review-20260928/runtime-probe.log`).

## Implementation

1. Add a private `SessionState` owned by Session; move maps/stacks from `Events.cpp`, `SingleInstance.cpp`, `Handlers.cpp`, `TestPage.cpp`, `Scopes.cpp`, `Table.cpp` and `BuiltinsWritten.cpp` into it. Keep TLS only as a scoped current-session locator.
2. Separate Session lifetime from worker activation. Serialize commands for one session; allow different sessions on reused workers. Restore parent state on nested activation; clear company-bound instances on company close.
3. Lease a connection for each active transaction, including its live cursors; return it after commit/rollback and reset. Bound pool size independently of logged-in users; retain no transaction during user think time.

## Acceptance

- Nested and sequential sessions never share subscribers, handlers, traps, language, random/error/validation state; parent instances survive child destruction.
- Closed sessions release resources; idle users do not reserve DB connections. Memory grows with active session state and bounded blocks, not table size.

## References

Code: `include/runtime/Session.h`, `src/rt/{Session,SingleInstance,Events,Scopes,Cursor}.cpp`. Platform: `properties/devenv-singleinstance-property.md`. Predecessor: WI-768/772/976 (ownership/races); old 0008/0009 are optimization hypotheses. Performance comparison: 0721.
