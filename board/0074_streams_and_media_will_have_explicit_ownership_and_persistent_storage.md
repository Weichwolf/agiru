# 0074 — Streams and media will have explicit ownership and persistent storage

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

Stream engines exist, but Media.cpp still refuses imports/exports and MediaSet operations. Media identifiers therefore do not imply a working media store. Stream materialization and shared ownership need bounded-resource proofs.

## Implementation for Sol

1. Specify InStream/OutStream attachment, shared backing data, position, encoding and disposal. Keep BLOB lazy-load state distinct from stream ownership.
2. Implement tenant media/media-set storage with ordered membership, transactional references, import/export and orphan cleanup. Resolve identifiers through the database across service tiers.
3. Enforce app/company/user scope where documented; reject invalid/missing media IDs loudly. Keep external files and HTTP streams behind explicit size/timeout/error handling.
4. Expose actual HTTP request/response errors at both transport and status-code layers; integrate HttpClientHandler and per-test registration before enabling external fallthrough.

## Acceptance

Binary and encoded round trips, unattached/disposed streams, alias positions, ordered media sets and rollback/orphan cases. Another session/process retrieves imported bytes; large streams remain bounded.

## References

Platform: stream/BLOB/Media/MediaSet and HTTP methods. AL: media table fields and data-exchange consumers. Predecessor: stream position, encoding and two-level HTTP failure findings.
