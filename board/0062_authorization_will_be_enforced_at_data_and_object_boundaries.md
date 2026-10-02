# 0062 — Authorization will be enforced at data and object boundaries

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Table.h currently describes every session as SUPER until a permission system exists. PermissionSet/Entitlement kinds are not fully generated. IsolatedStorage.cpp omits extension identity from its primary key and SetEncrypted stores the supplied value as plaintext with a Boolean flag.

## Implementation for Sol

1. Generate permission sets/extensions, composition/exclusion and entitlement metadata with declaring app identity. Build effective permissions per user/company/session, including indirect and scoped inherent grants.
2. Check reads, writes, executable objects and related-table/FlowField access at runtime boundaries; UI hiding is additional presentation, never authorization. Implement SecurityFiltering modes explicitly.
3. Key isolated storage by extension plus documented DataScope dimensions. Replace plaintext SetEncrypted with a justified encryption/key-management mechanism, or refuse it explicitly until supported.
4. Carry TestPermissions and temporary-record exceptions according to documentation, coordinated with 0039. Implement HTTP/API identity before multi-user exposure.

## Acceptance

Two users, two companies and two extensions prove denial and isolation. Include forged HTTP requests, denied indirect writes, scoped elevation unwinding and encrypted-value-at-rest inspection. No test may pass merely because every user is SUPER.

## References

Platform: security documentation, permissions/inherent permissions/TestPermissions properties, isolatedstorage method overloads. AL: LibraryLowerPermissions and permission set declarations. Predecessor: security findings identify traps; its subset success is not authorization proof.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `accessbypermission`, `assignable`, `excludedpermissionsets`, `includedpermissionsets`, `inherententitlements`, `inherentpermissions`, `objectentitlements`, `permissions`, `roletype`, `testpermissions`, `type`, `type-entitlement`, `type-report`.
