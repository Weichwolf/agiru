# 0726 — Run the development server and PostgreSQL in one Podman container

Status: queued | Priority: P0
Depends on: existing native compiler/runtime; 0720's HTTP server contract for client acceptance,
not its full business milestone. Container/toolchain provisioning can proceed now.
Next: connect 0720's authenticated ERP application command to the supervised entrypoint;
qualify the external Node CLI/browser and migrate a verified seed without changing agiru-pg.

## Implementation and acceptance

- One development container owns nginx, PostgreSQL and the private C++ server/static htmx assets.
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

Files: `deploy/dev/{Containerfile,entrypoint.sh,nginx.conf}`, `scripts/dev_container.sh`,
`Makefile`, `test/tooling/podman-development.sh`; HTTP acceptance belongs in `test/ui/`.

## Current evidence

- `make dev-image dev-start dev-configure`: Debian trixie, Clang 19/libc++, PostgreSQL 17;
  persistent `agiru-dev-postgres` volume, host HTTP port bound to loopback, no published SQL port.
  Existing agiru-pg and its source/seed databases remain unchanged.
- Qualified image: `a447dfce0ffd94a40c7a28634bb4b587d96740508892b1689c99fd5c51397554`.
  nginx 1.26.3/libmicrohttpd 1.0.1; only nginx's 8080 port is published on host loopback.
  Private backend defaults to 127.0.0.1:18080; SQL remains unpublished.
  `make dev-check` passes committed exact SQL persistence across restart, PostgreSQL failure,
  application exit 42, nginx failure, unowned container start/stop refusals and incompatible
  storage preservation. Supervision handles immediate exits without wait-n's completed-child race.
  nginx runs unprivileged with owned runtime directories and bounded shutdown.
  Existing container is preserved stopped as `agiru-dev-pre-nginx-20261006`; persistent
  data and the 18-MiB compiler cache were retained in the replacement, not reinitialized.
  New clusters explicitly use UTF8/C.UTF-8; the existing gate/master volume remains
  SQL_ASCII and is not a qualified production seed. Preserve it during later seed migration.
  `make http-test`: eleven nginx/native-HTTP cases, real external CMD/MCP and independent
  transport SQL records pass. This does not establish AL page saves or ERP/browser parity.
- `make dev-exec COMMAND='make gate GATE=GenXmlPortGate B=/workspace/build/podman JOBS=2'`:
  container compilation succeeds; 27 checks, zero failures.
- Default command is currently `database-only`: PostgreSQL/nginx are supervised; absent
  application requests return 502. No successful HTTP ERP server or client parity is claimed.
  `agiru_master` starts empty, not as a qualified BC seed. The development superuser/password
  and PostgreSQL-only healthcheck are not production security or application readiness.
