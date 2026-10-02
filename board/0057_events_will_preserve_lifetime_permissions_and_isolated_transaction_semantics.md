# 0057 — Events will preserve lifetime, permissions and isolated transaction semantics

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Publisher/subscriber registration and dispatch already exist in src/rt/Events.cpp. The old assertion that events all fire into empty bodies is false. Manual/automatic subscriber state is thread-local, and isolated execution depends on the currently incomplete transaction layer.

## Implementation for Sol

1. Build a dispatch matrix for integration/business/internal events and platform table/page events. Preserve var writeback, IncludeSender, GlobalVarAccess, subscriber instance mode and declared subscription filters.
2. Make bindings session-owned and remove them on every instance destruction/error path. Keep deterministic dispatch order as an explicit agiru choice without making AL business logic depend on it.
3. Enforce skip-on-missing-permission/license and internal app visibility with 0062/0033. An event with no subscribers is legal and must not raise simply for being unobserved.
4. Implement isolated subscriber transactions on top of 0012, respecting an already active write transaction and documented commit/error rules.

## Acceptance

Fixtures distinguish manual from automatic instances, object lifetime, nested sessions, var outputs, permission skip versus error, empty publisher, subscriber failure and isolated commit. A two-connection test proves isolation durability.

## References

Platform: devenv-eventsubscriber-attribute.md, devenv-integrationevent-attribute.md, devenv-eventsubscriberinstance-property.md, devenv-events-isolated.md. AL: event declarations and system triggers. Predecessor: WI-1036/1127/1145/1169/1216.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `eventsubscriberinstance`.
