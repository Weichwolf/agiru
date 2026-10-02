# 0057 — Events will preserve lifetime, permissions and isolated transaction semantics

Status: open | Priority: P1 | Stage: UT event semantics; Clients isolation | Reviewed: 2026-09-30
Depends on: 0006 subscriber ownership; 0012 transactions; 0033 visibility.

## Evidence

- Non-SingleInstance StaticAutomatic subscribers now have one owned instance per subscriber invocation, disposed immediately on normal/error/isolated/reentrant paths. The per-thread catalogue cache and its private Subscribers.h cleanup hook are removed; SingleInstance subscribers still use the Session-owned map. This implements properties/devenv-eventsubscriberinstance-property.md, not a session-owned replacement for a forbidden cache.
- SubscriberLifetimeGate: 22/22 under Clang 19/GCC 14; same gate with frozen pre-fix Events.cpp and unchanged remaining runtime: 14 red. Full local suite: 82 cases, 0 red. Targeted Events/gate analysis has no findings in changed functions/new gate; inherited public-header failures remain. No new suppression or raised baseline. Logs: build/development-subscribers/. Snapshot 20260928T173853Z-198401 predates this patch; full-population UT effects remain unmeasured.
- `Events.cpp::Invoke` catches isolated-subscriber errors inside a Scope and calls Keep on success; Dispatch delegates to it.
- The platform requires separate committed subscriber transactions only outside an active write transaction; inside a write transaction the event behaves normally. Scope::Keep only releases a savepoint.
- Reproduced: throwing isolated subscriber after a write returns normally (`build/review-20260928/runtime-probe.log`).
- Manual bindings now belong to the active SessionState; destroyed instances are unbound from active nested ancestors and dispatch snapshots check binding generation before invocation. SessionBindingGate: 18/18 Clang/GCC; old Events.cpp: 10 red. Full local suite: 83 cases, 0 red. Cross-worker object escape remains unsupported; full UT effects await a frozen run.

## Implementation

1. Track transaction write state and lifecycle mode. Select ordinary dispatch for active writes/install/upgrade; otherwise commit each successful isolated subscriber and roll back only its failure. Preserve var outputs even after rollback; integrate 0062 permission decisions.
2. Preserve the completed automatic-lifetime gates and compare the same full UT population after the fix; investigate cold-state losses rather than restoring the cache. Extend the dispatch matrix for integration/business/internal events and platform table/page events: var writeback, IncludeSender, GlobalVarAccess and declared subscription filters.
3. Preserve session-owned binding/lifetime gates; define cross-worker object-lifetime policy and deterministic dispatch order without making AL business logic depend on it.
4. Enforce skip-on-missing-permission/license and internal app visibility with 0062/0033. An event with no subscribers is legal and must not raise simply for being unobserved.
5. Implement isolated subscriber transactions on top of 0012, respecting an already active write transaction and documented commit/error rules.

## Acceptance

- Fixtures distinguish manual from automatic instances, object lifetime, nested sessions, var outputs, permission skip versus error, empty publisher, subscriber failure and isolated commit. A two-connection test proves isolation durability.

## References

Code: `src/rt/{Events,Session}.cpp`, `src/gen/CodeunitWriter.cpp`, `test/gate/{Event,SubscriberLifetime}Gate.cpp`.

Platform: devenv-eventsubscriber-attribute.md, devenv-integrationevent-attribute.md, devenv-eventsubscriberinstance-property.md, devenv-events-isolated.md. AL: event declarations and system triggers. Predecessor: WI-1036/1127/1145/1169/1216.

Property scope: `eventsubscriberinstance`.

Lifecycle/binding references: platform `properties/devenv-eventsubscriberinstance-property.md`, `methods-auto/session/session-{bindsubscription,unbindsubscription}-method.md`; BCApps current main `src/System Application/App/Language/src/LanguageImpl.Codeunit.al` (SingleInstance state) and `src/System Application/Test/Language/src/LanguageTest.Codeunit.al` (Manual binding/var outputs). Predecessor `1339_bindsubscription-auf-einer-nicht-manuellen-codeunit-muss-wer.md` rejects silently accepting automatic BindSubscription; current gate covers that refusal.
