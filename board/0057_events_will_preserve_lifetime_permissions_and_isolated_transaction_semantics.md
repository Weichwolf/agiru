# 0057 — Events will preserve lifetime, permissions and isolated transaction semantics

Status: open | Priority: P1 | Stage: UT event semantics; Clients isolation | Reviewed: 2026-09-28
Depends on: 0006 subscriber ownership; 0012 transactions; 0033 visibility.

## Evidence

- `Events.cpp::Dispatch` always catches isolated-subscriber errors inside a Scope and calls Keep on success.
- The platform requires separate committed subscriber transactions only outside an active write transaction; inside a write transaction the event behaves normally. Scope::Keep only releases a savepoint.
- Reproduced: throwing isolated subscriber after a write returns normally (`build/review-20260928/runtime-probe.log`).

## Implementation

1. Track transaction write state and lifecycle mode. Select ordinary dispatch for active writes/install/upgrade; otherwise commit each successful isolated subscriber and roll back only its failure. Preserve var outputs even after rollback; integrate 0062 permission decisions.
2. Build a dispatch matrix for integration/business/internal events and platform table/page events. Preserve var writeback, IncludeSender, GlobalVarAccess, subscriber instance mode and declared subscription filters.
3. Make bindings session-owned and remove them on every instance destruction/error path. Keep deterministic dispatch order as an explicit agiru choice without making AL business logic depend on it.
4. Enforce skip-on-missing-permission/license and internal app visibility with 0062/0033. An event with no subscribers is legal and must not raise simply for being unobserved.
5. Implement isolated subscriber transactions on top of 0012, respecting an already active write transaction and documented commit/error rules.

## Acceptance

- Fixtures distinguish manual from automatic instances, object lifetime, nested sessions, var outputs, permission skip versus error, empty publisher, subscriber failure and isolated commit. A two-connection test proves isolation durability.

## References

Code: `src/rt/Events.cpp`, `src/rt/Subscribers.h`, `src/gen/CodeunitWriter.cpp`, `test/gate/EventGate.cpp`.

Platform: devenv-eventsubscriber-attribute.md, devenv-integrationevent-attribute.md, devenv-eventsubscriberinstance-property.md, devenv-events-isolated.md. AL: event declarations and system triggers. Predecessor: WI-1036/1127/1145/1169/1216.

Property scope: `eventsubscriberinstance`.
