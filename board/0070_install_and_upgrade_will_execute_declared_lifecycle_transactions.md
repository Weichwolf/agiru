# 0070 — Install and upgrade will execute declared lifecycle transactions

Status: open | Priority: P3 | Stage: All | Reviewed: 2026-09-28
Depends on: 0033 app versions; 0012 transaction phases; 0062 app access.

## Evidence

- Install/Upgrade subtypes parse, but no complete lifecycle driver exists. ProvisionInstalled catalog writes are not AL install/upgrade.

## Implementation

1. Persist a migration journal with expected schema/version and phase outcome; lock app upgrades in PostgreSQL. Restart resumes or refuses from the durable phase, never from a process-global flag.
   First deployment model: build/validate an immutable app composition, drain affected sessions, migrate, then activate matching binaries/schema. Do not promise ABI-safe live extension loading while merged native record layouts change. Preserve the previous artifact and distinguish binary rollback from data-migration reversibility.
2. Persist installed app/version/data-version state and company identity. Drive install and upgrade hooks in documented database/company phases, with deterministic ordering where BC leaves order unspecified.
3. Run preconditions before writes and validation after migration; rollback the correct phase on failure. Expose NavApp module metadata with declaring-app identity.
4. Implement DataTransfer set operations only in allowed upgrade context. Preserve obsolete/moved-field storage and schema migrations instead of dropping columns from current source visibility.
5. Support app-owned packaged files under an installation root with path validation and app access checks.

## Acceptance

- Install, reinstall and upgrade fixtures cover multiple companies, failing preconditions, validation rollback, schema/data version changes and cross-app file denial. Restart after interruption must not report a completed upgrade.

## References

Code: `src/rt/Storage.cpp`, `src/rt/Builtins.cpp`, `src/gen/CodeunitWriter.cpp`, `src/tc/Main.cpp`.

Platform: install/upgrade codeunits, DataTransfer and NavApp methods, moved/obsolete properties. AL: Install/Upgrade subtypes. Predecessor: lifecycle findings guide failure cases; no Python startup apparatus is needed.
