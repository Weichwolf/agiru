# 0726 — Run the development server and PostgreSQL in one Podman container

Status: queued | Priority: P0
Depends on: existing native compiler/runtime; 0720's HTTP server contract for client acceptance,
not its full business milestone. Container/toolchain provisioning can proceed now.
Next: package Clang 19/libc++ and PostgreSQL 17 with one supervised development
entrypoint; preserve agiru-pg and seed data. Run the Node agent CLI on the host over HTTP.

## Implementation and acceptance

- One development container owns PostgreSQL and the C++ server/static htmx assets.
  No Node ERP server; CMD/MCP and browsers are external clients of the same endpoints.
- Use persistent PostgreSQL storage, explicit development-only credentials, loopback
  port publishing, graceful signal forwarding and observable child failures.
  Refuse incompatible existing storage; never reinitialize or silently erase it.
- Bind the editable repository; keep container compiler outputs separate from host
  ABI/cache state. Make remains the build/test entrypoint, with explicit DB settings.
- Restore a read-only export into a new seed database; record source/version/hash and
  never test against the source or master. No destructive migration of agiru-pg.
- Prove start/health/stop/restart, committed SQL persistence, failed PostgreSQL/server
  processes, and actual external CLI/web HTTP operation parity. Image build or SQL
  readiness alone is not server/client completion. Production topology remains separate.

Files: `Containerfile`, `scripts/`, `Makefile`, `test/tooling/`, `test/ui/`.
Evidence: packaging not implemented; existing agiru-pg is PostgreSQL 17 on host port 5433.
