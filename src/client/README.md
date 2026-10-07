# Agent client

Node 20+, outside the agiru/PostgreSQL container. `make client` installs the locked
dependencies and compiles into `build/client/`; `make client-test` qualifies a
declared C++ HTML/HTTP fixture, not production ERP parity. Server HTTP wiring,
native authentication and retained page sessions exist; dialogs, bounded lists and
business workflows remain incomplete.

```sh
export AGIRU_ORIGIN=http://127.0.0.1:8080
node build/client/cmd.mjs read '/?company=CRONUS%20CH&page=21'
node build/client/cmd.mjs --json read '/?company=CRONUS%20CH&page=21'
node build/client/cmd.mjs execute '{"path":"/?page=21","page":"<advertised-handle>","revision":"<advertised-revision>","command":"<advertised-command>","control":"<AL-control-name>","operation":"set","text":"<explicit-display-text>"}'
node build/client/mcp.mjs
```

- CMD: compact deterministic text, or lossless JSON. Errors go to stderr.
  Exit 0: successful response; 2: refusal/error; 3: uncertain write.
- MCP stdio: `agiru_read` and `agiru_execute`, shared schemas/library. Structured
  results and compact text; stdout is protocol-only. Input is bounded to 1 MiB,
  strictly UTF-8, with no automatic dialog answers.
- Commands require copied handles/revisions/command IDs and exact AL identities.
  Set takes explicit display text; C++ must parse/validate it. Action takes no text.
  Client fences supplement, never replace, server authorization and receipt checks.
- Profile 1 supports one current row, ordered groups/fields/actions/labels and
  counted unsupported alerts. Forms and htmx must advertise the same operation.
  Unknown profiles/effects, malformed HTML, duplicate identities and budget
  excesses refuse. Decimal/Int64 values remain strings; enum domains/members and
  temporal flags stay separate from captions. CSRF fields remain private.
  HTML is bounded to 1 MiB, structured responses to 4 MiB and ASCII to 256 KiB.
  An oversized ASCII view reports its refusal explicitly; full values remain in
  JSON/MCP structured results, with `presentation.ascii=refused`.
- HTTP is same-origin, HTTPS except loopback development; redirects refuse.
  No write retries. `WriteUncertain` includes the command ID: reconcile the
  future server receipt endpoint before choosing another command.
- Optional `AGIRU_AUTH_FILE`: owned regular file, mode 0600, at most 8 KiB;
  symlinks/devices/FIFOs refuse without waiting. JSON keys only `authorization`
  and/or `cookie`. Keep outside Git. Credentials
  never appear in argv, page output or diagnostics. Authentication is server-owned.

Dependencies: parse5 8.0.1 (MIT) supplies HTML entity/tree semantics, not a browser;
the official MCP SDK 1.32.1 (MIT) supplies stdio/schema/protocol interoperability;
zod 3.25.76 (MIT) supplies one strict operation schema for both adapters.
TypeScript 5.9.3 (Apache-2.0) and Node declarations are build-only dependencies.
`package-lock.json` pins the complete dependency graph and integrity hashes;
installed packages retain their own notices/licenses. None are ERP-server dependencies.

`make web` bundles htmx 2.0.11 (0BSD) and the same profile/parser/envelope into
`build/web/`; `make dev-web` publishes those static files to the owned Caddy container.
Open `/assets/index.html` with the privately issued development token. It stays in
tab memory, never URL/storage; production sign-in remains pending. Updated Caddy
configuration serves the same entry at `/?page=...&company=...` for browser navigation;
HX requests keep the native fragment route. Applying that configuration needs a
container restart/rebuild; publishing assets alone does not change the running proxy.
No CDN, business rules, inline scripts, eval or automatic command retries. Unknown
response effects and fragments refuse before htmx processes them. Browser and agent
commands share the advertised exact envelope; server authorization stays authoritative.
`make web-test` uses actual system Chromium with Playwright 1.63.0 (Apache-2.0,
test-only); ten native-HTML fixture cases, one actual Caddy asset/routing case and
three defective compiled bundles,
not production SQL/dialog/ERP acceptance. esbuild 0.28.2 (MIT) is build-only.
The bundle ships dependency licenses; the native server does not use Node.js.
