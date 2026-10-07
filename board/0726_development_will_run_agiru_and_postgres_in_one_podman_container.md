# 0726 — Run the development server and PostgreSQL in one Podman container

Status: queued | Priority: P0
Depends on: existing native compiler/runtime; 0720's HTTP server contract for client acceptance,
not its full business milestone. Container/toolchain provisioning can proceed now.
Next: connect 0720's authenticated ERP application command to the supervised entrypoint;
qualify the external Node CLI/browser and migrate a verified seed without changing agiru-pg.

## Implementation and acceptance

- One development container owns Caddy, PostgreSQL and the private C++ server/static htmx assets.
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

Files: `deploy/dev/{Containerfile,entrypoint.sh,Caddyfile}`, `scripts/dev_container.sh`,
`Makefile`, `test/tooling/podman-development.sh`; HTTP acceptance belongs in `test/ui/`.

## Current evidence

- `make dev-image dev-start dev-configure`: Debian trixie, Clang 19/libc++, PostgreSQL 17;
  persistent `agiru-dev-postgres` volume, host HTTP port bound to loopback, no published SQL port.
  Existing agiru-pg and its source/seed databases remain unchanged.
- Qualified, deployed image: `916d9ee8fe2cd925c6041f293ff72168db993dbe26cc6fcd36b832187c1df9a6`.
  PostgreSQL starts with `max_locks_per_transaction=1024`, matching `scripts/pg_master.sh`.
  A real transaction retains 16,384 advisory locks; a separate disposable container with
  an entrypoint mutation reduced to 64 fails the same SQL with out-of-shared-memory.
  The full development check remains green. Replacement followed the successful native
  build; the live PostgreSQL instance confirms the new setting.
  Official Debian Caddy package `2.6.2-12+deb13u1`/libmicrohttpd 1.0.1; only Caddy's
  8080 port is published on host loopback. Builds bypass cached package installation,
  refresh signed Debian indexes and upgrade installed packages before provisioning.
  Private backend defaults to 127.0.0.1:18080; SQL remains unpublished.
  `make dev-check` passes committed exact SQL persistence across restart, PostgreSQL failure,
  application exit 42, Caddy failure, unowned container start/stop refusals and incompatible
  storage preservation. Supervision handles immediate exits without wait-n's completed-child race.
  Caddy runs unprivileged with bounded shutdown, disabled admin API and persistent certificate
  storage. Local TLS verifies with its explicit CA; untrusted TLS fails. Redirects and certificate
  persistence survive restart. Public ACME issuance/renewal and production load remain unqualified.
  The development replacement is authorized; the previous container is stopped as
  `agiru-dev-pre-lock-budget-20261007`. All seven database identities, seed Customer/G/L
  Entry counts and complete provenance are unchanged. Compiler-cache statistics and
  frozen verification-input hashes match; native agiru ownership is restored.
  The preserved 5.2-GiB compiler cache has an explicit 16-GiB ceiling, raised before
  the former 5-GiB limit evicted entries. This is not a demonstrated build speedup.
  New clusters explicitly use UTF8/C.UTF-8; the existing gate/master volume remains
  SQL_ASCII and is not a qualified production seed. Preserve it during later seed migration.
- `make http-test JOBS=2`: twelve native transport and nine credential cases pass with
  external CMD/SDK MCP and independent SQL. Conflicting lengths/hosts refuse; CL/TE
  normalization forwards only decoded bytes, no conflicting upstream length or hidden
  SQL request. This follows RFC 9112 section 6.3, not nginx-specific rejection behaviour:
  `https://www.rfc-editor.org/rfc/rfc9112.html#section-6.3`.
- `make page-host-test JOBS=2`: twelve generated-page fixture cases and seventeen native
  application cases pass through Caddy/C++/PostgreSQL with external CMD/MCP. Three compiled
  ownership/revision/replay defects reject. This is not full ERP or actual browser acceptance.
- Default command is currently `database-only`: PostgreSQL/Caddy are supervised; absent
  application requests return 502. No successful HTTP ERP server or client parity is claimed.
  `agiru_master` starts empty, not as a qualified BC seed. The development superuser/password
  and PostgreSQL-only healthcheck are not production security or application readiness.
- Native startup is `agiru serve --config <file>`; the complete default profile is
  `deploy/dev/agiru.json` in the repository bind mount. Protect deployment credentials
  with a private mounted file or libpq passfile. The example database is not auto-seeded;
  missing company/permission/client storage refuses before listener startup.

## TLS configuration

- Default `make dev-start`: loopback HTTP on 8080. No host Caddy dependency.
- A newly created container accepts `AGIRU_DEV_SITE=<domain>`, `AGIRU_DEV_HTTP_PORT=80`
  and `AGIRU_DEV_HTTPS_PORT=443`; internal unprivileged ports are 8080/8443. Publishing
  remains loopback-only; public deployment must forward external 80/443 and configure DNS.
- `AGIRU_DEV_SITE=localhost` uses the local CA, never a publicly trusted ACME certificate.
  Certificate state lives under the persistent volume's `caddy/` directory; do not discard it.
- Install Caddy exclusively from signed official Debian repositories; `/usr/bin/caddy`
  belongs to the Debian package, whose notice is `/usr/share/doc/caddy/copyright`.
  No upstream binary archives or extra package repositories. Native amd64 is qualified;
  arm64 execution is not. The packaged Caddy accepts the bounded edge configuration;
  admin API remains disabled and certificate/configuration storage stays private.
  No automatic running-container replacement or unattended service restart is implemented.
