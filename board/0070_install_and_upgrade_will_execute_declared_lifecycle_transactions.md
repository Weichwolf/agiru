# 0070 — Install and upgrade will execute declared lifecycle transactions

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

Install/Upgrade subtypes can parse as codeunits, but the CLI and runtime lack a complete application lifecycle driver. ProvisionInstalled writes platform catalogue rows directly and is not an AL install/upgrade implementation.

## Implementation for Sol

1. Persist installed app/version/data-version state and company identity. Drive install and upgrade hooks in documented database/company phases, with deterministic ordering where BC leaves order unspecified.
2. Run preconditions before writes and validation after migration; rollback the correct phase on failure. Expose NavApp module metadata with declaring-app identity.
3. Implement DataTransfer set operations only in allowed upgrade context. Preserve obsolete/moved-field storage and schema migrations instead of dropping columns from current source visibility.
4. Support app-owned packaged files under an installation root with path validation and app access checks.

## Acceptance

Install, reinstall and upgrade fixtures cover multiple companies, failing preconditions, validation rollback, schema/data version changes and cross-app file denial. Restart after interruption must not report a completed upgrade.

## References

Platform: install/upgrade codeunits, DataTransfer and NavApp methods, moved/obsolete properties. AL: Install/Upgrade subtypes. Predecessor: lifecycle findings guide failure cases; no Python startup apparatus is needed.
