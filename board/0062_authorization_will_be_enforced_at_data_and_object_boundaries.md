# 0062 — Authorization will be enforced at data and object boundaries

Status: open | Priority: P1 | Stage: UT permission semantics; Clients exposure gate | Reviewed: 2026-10-04
Depends on: 0006 session identity; 0033 declaring app; 0013 company schema.

## Evidence

- Table permission checks assume SUPER; Session identity is SYSTEM/blank GUID.
- Frozen 101600: eleven RapidStart workflows now reach missing `Database::Permission Set`
  identity in `ConfigValidateManagement::LookupObject`; original System-29 Permission
  Set/Permission declarations are not licensing exclusions. 0034's source-only
  constant path now passes authored execution/identity controls; original full-UT
  activation is pending, not live permission-table/provider or authorization proof.
- `IsolatedStorage.cpp` lacks extension identity and stores SetEncrypted values as plaintext plus secret=true.

## Implementation

1. Make authenticated user/company and declaring/caller app explicit runtime context. Compile permission decisions into bounded session caches keyed by a database-owned permission revision; invalidate across tiers.
2. Generate permission sets/extensions and composition/exclusion with declaring app identity. Build effective permissions per user/company/session, including indirect and scoped inherent grants. No commercial license/entitlement gates: 0725 owns their retirement, not an unconditional SUPER grant.
3. Check reads, writes, executable objects and related-table/FlowField access at runtime boundaries; UI hiding is additional presentation, never authorization. Implement SecurityFiltering modes explicitly.
4. Key isolated storage by extension plus documented DataScope dimensions. Replace plaintext SetEncrypted with a justified encryption/key-management mechanism, or refuse it explicitly until supported.
5. Carry TestPermissions and temporary-record exceptions according to documentation, coordinated with 0039. Implement HTTP/API identity before multi-user exposure.
6. Define browser/CLI credential issuance, expiry/revocation and logout. Enforce CSRF/origin checks for cookie-authenticated commands; escape AL values/captions by HTML context, reject script-bearing fragments and traversal filenames. Bound bodies/uploads and external resource loading; redact secrets from SQL/error/HTTP traces.

## Acceptance

- Two users, two companies and two extensions prove denial and isolation. Include forged HTTP requests, denied indirect writes, scoped elevation unwinding and encrypted-value-at-rest inspection. No test may pass merely because every user is SUPER.

## References

Code: `src/rt/{Session,IsolatedStorage}.cpp`, `include/runtime/{Table,RecordRef}.h`, `include/runtime/test/TestPermissions.h`, `src/db/Connection.cpp` tracing. Platform: security documentation, permissions/inherent permissions/TestPermissions properties, isolatedstorage method overloads. AL: LibraryLowerPermissions and permission set declarations. Predecessor: security findings identify traps; its subset success is not authorization proof.

Property scope: `accessbypermission`, `assignable`, `excludedpermissionsets`, `includedpermissionsets`, `inherententitlements`, `inherentpermissions`, `objectentitlements`, `permissions`, `roletype`, `testpermissions`, `type`, `type-entitlement`, `type-report`.
