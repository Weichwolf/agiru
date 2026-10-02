# 0006 — Session ownership and performance will have measured portable bounds

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

`Session` uses a thread-local current pointer, while Events.cpp holds thread-local instance maps. Nested sessions restore the pointer but release thread-owned instances, so thread-local storage is not by itself session ownership. Catalogue registration uses startup vectors despite the intended immutable metadata design. No current aarch64 result was established by this review.

## Implementation for Sol

1. Move mutable subscriber instances, manual bindings, random state and page traps under explicit Session ownership. Test nested sessions and sequential sessions reusing one worker thread.
2. Measure sizeof/session arena usage, marginal RSS/PSS per active session and shared-library relocation pages. Distinguish constexpr declarations from actual .rodata versus .data.rel.ro placement.
3. Benchmark indexed Get, bounded FindSet and posting against equivalent SQL with AGIRU_FAST_LOOP=OFF. Record compiler, architecture, row count, concurrency and database settings.
4. Only then optimize decimal division, allocation and code layout. Do not adopt a prescribed divider/layout merely because an old WI proposed it.

## Acceptance

Two concurrent and two nested sessions never see each other's mutable state. Publish x86_64 and aarch64 measurements with comparable workloads; no guessed portability claim. Metadata initialization and transaction-pool costs are counted separately.

## References

Repository: src/rt/Session.cpp, Events.cpp, SingleInstance.cpp, Catalogue.cpp; include/runtime/Session.h; src/net/Decimal.cpp. Platform: session and SingleInstance contracts. Original WIs 0006, 0008, 0009, 0596 describe hypotheses, not measured completion.
