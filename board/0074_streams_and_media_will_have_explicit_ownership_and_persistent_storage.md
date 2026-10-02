# 0074 — Streams and media will have explicit ownership and persistent storage

Status: open | Priority: P1 | Stage: UT streams; All media | Reviewed: 2026-09-28
Depends on: 0012 persistence; 0006 ownership.

## Evidence

- Stream implementations exist; Media/MediaSet import/export remains refused. Stream materialization can grow with the full payload.

## Implementation

1. Separate byte streams from decoded text, retain encoding and bounded backing ownership. Database-backed media metadata plus chunked payloads must preserve transaction visibility and content hashes across processes.
2. Specify InStream/OutStream attachment, shared backing data, position, encoding and disposal. Keep BLOB lazy-load state distinct from stream ownership.
3. Implement tenant media/media-set storage with ordered membership, transactional references, import/export and orphan cleanup. Resolve identifiers through the database across service tiers.
4. Enforce app/company/user scope where documented; reject invalid/missing media IDs loudly. Keep external files and HTTP streams behind explicit size/timeout/error handling.
5. Expose actual HTTP request/response errors at both transport and status-code layers; integrate HttpClientHandler and per-test registration before enabling external fallthrough.

## Acceptance

- Binary and encoded round trips, unattached/disposed streams, alias positions, ordered media sets and rollback/orphan cases. Another session/process retrieves imported bytes; large streams remain bounded.

## References

Code: `src/net/Stream.cpp`, `src/net/Media.cpp`, `src/rt/Builtins.cpp`, `include/type/Stream.h`.

Platform: stream/BLOB/Media/MediaSet and HTTP methods. AL: media table fields and data-exchange consumers. Predecessor: stream position, encoding and two-level HTTP failure findings.
