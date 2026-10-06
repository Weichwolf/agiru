# 0726 — Run the development server and PostgreSQL in one Podman container

Status: queued | Priority: P0
Depends on: existing native compiler/runtime; 0720's HTTP server contract for client acceptance,
not its full business milestone. Container/toolchain provisioning can proceed now.
Next: connect 0720's C++ HTTP server and static assets to the supervised entrypoint;
qualify the external Node CLI/browser and migrate a verified seed without changing agiru-pg.

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

Files: `deploy/dev/{Containerfile,entrypoint.sh}`, `scripts/dev_container.sh`,
`Makefile`, `test/tooling/podman-development.sh`; HTTP acceptance belongs in `test/ui/`.

## Current evidence

- `make dev-image dev-start dev-configure`: Debian trixie, Clang 19/libc++, PostgreSQL 17;
  persistent `agiru-dev-postgres` volume, host HTTP port bound to loopback, no published SQL port.
  Existing agiru-pg and its source/seed databases remain unchanged.
- Qualified image: `710003f1690b0264f8e1a40a2694cc07e3cc03e5b742034bae3c86e558a1cc6b`.
  Includes ripgrep, required by repository regression scripts; `make dev-exec COMMAND='rg --version'` passes.
  `make dev-check` passes committed exact SQL persistence across restart, PostgreSQL failure,
  application exit 42, unowned container start/stop refusals and incompatible storage preservation.
- `make dev-exec COMMAND='make gate GATE=GenXmlPortGate B=/workspace/build/podman JOBS=2'`:
  container compilation succeeds; 27 checks, zero failures.
- Default command is currently `database-only`; no HTTP ERP server or client parity is claimed.
  `agiru_master` starts empty, not as a qualified BC seed. The development superuser/password
  and PostgreSQL-only healthcheck are not production security or application readiness.
