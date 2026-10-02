# 0006 — Mutable runtime state will belong to the session

Status: open | Priority: P0 | Stage: UT session correctness; Clients ownership | Reviewed: 2026-09-28
Depends on: 0012 transaction lease contract.

## Evidence

- SingleInstance storage is now owned by a private, lazy SessionState, with typed CodeunitId keys and owning deleters. Session close releases its own map; nested and sequential sessions cannot adopt a worker's prior map. InstanceGate now opens a real session rather than treating a thread as one.
- SessionInstanceGate: 12 checks green under Clang 19/GCC 14 (nesting, parent state survival, exact disposal, worker reuse, a second thread and sessionless refusal). Matching frozen headers/runtime negative control: 5 red, also 12 checks. Full local suite: 81 cases, 0 red; InstanceGate 35/35 under both compilers. Logs: `build/development-session/`; the completed frozen UT run does not include this patch.
- Targeted analysis still fails inherited public-header findings, with no finding in the changed functions or new gate; unused direct includes were repaired. No suppression/baseline increase. A matched full-milestone run on a sealed seed is still required before attributing UT effects.
- Mutable thread locals also hold manual event bindings, handlers, page traps, error/commit scopes, validation context and random state. `Session` holds a PostgreSQL connection for its entire lifetime.
- Automatic-instance storage still belongs to the thread and can be cleared by a nested Session destructor. Company-close invalidation and handle escape/lifetime enforcement remain open; the SingleInstance map fix is not complete session isolation.

## Implementation

1. Extend the existing private SessionState: move remaining maps/stacks from `Events.cpp`, `Handlers.cpp`, `TestPage.cpp`, `Scopes.cpp`, `Table.cpp` and `BuiltinsWritten.cpp` into it, after checking each lifetime contract. Keep TLS only as a scoped current-session locator; preserve the completed SingleInstance gates.
2. Separate Session lifetime from worker activation. Serialize commands for one session; allow different sessions on reused workers. Restore parent state on nested activation; clear company-bound instances on company close.
3. Lease a connection for each active transaction, including its live cursors; return it after commit/rollback and reset. Bound pool size independently of logged-in users; retain no transaction during user think time.

## Acceptance

- Nested and sequential sessions never share subscribers, handlers, traps, language, random/error/validation state; parent instances survive child destruction.
- Closed sessions release resources; idle users do not reserve DB connections. Memory grows with active session state and bounded blocks, not table size.

## References

Code: `include/runtime/Session.h`, `src/rt/{Session,SingleInstance,Events,Scopes,Cursor}.cpp`. Platform: `properties/devenv-singleinstance-property.md`. Predecessor: WI-768/772/976 (ownership/races); old 0008/0009 are optimization hypotheses. Performance comparison: 0721.

SingleInstance references: platform `properties/devenv-singleinstance-property.md` and `methods-auto/system/system-clear-joker-method.md`; BCApps main `src/System Application/App/Environment Information/src/EnvironmentInformationImpl.Codeunit.al` holds testability flags in a SingleInstance codeunit. Predecessor `772_free-threading-legt-4-echte-races-frei-die-der-gil-verdeckt.md` measured cold-cache losses after removing shared instances: investigate any full-population loss rather than restoring leakage. `976_mem-project-thread-safety.md` establishes session-private state; native C++ ownership replaces Python ContextVar machinery.
