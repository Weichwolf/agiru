import assert from "node:assert/strict";
import { test, after } from "node:test";
import { spawn, execFile } from "node:child_process";
import { promisify } from "node:util";
import { chmod } from "node:fs/promises";
import { createRequire } from "node:module";
import { readAuth } from "../../build/client/http.mjs";
const execute = promisify(execFile);
const container = process.env.AGIRU_DEV_CONTAINER ?? "agiru-dev";
const origin = `http://127.0.0.1:${process.env.AGIRU_DEV_HTTP_PORT ?? "8080"}`;
const proof = process.env.AGIRU_AUTH_PROOF;
const nativePath = process.env.AGIRU_NATIVE_AUTH;
const server = spawn("make", ["--no-print-directory", "dev-exec",
  `COMMAND=/workspace/build/podman/gate_ClientCredentialsGate --serve ${process.env.AGIRU_NATIVE_HTTP_HTML} ${nativePath}`],
  { env: { ...process.env, AGIRU_DEV_INTERACTIVE: "1" }, stdio: ["pipe", "pipe", "pipe"] });
let output = "", diagnostic = "";
server.stdout.on("data", chunk => { output += chunk; });
server.stderr.on("data", chunk => { diagnostic += chunk; });
const closed = new Promise(resolve => server.once("close", resolve));
await new Promise((resolve, reject) => {
  const timer = setTimeout(() => reject(new Error("Authentication fixture did not start")), 15000);
  server.on("error", reject);
  server.once("close", () => { clearTimeout(timer); reject(new Error("Authentication fixture exited before readiness")); });
  server.stdout.on("data", () => { if (output.includes("READY\n")) { clearTimeout(timer); resolve(); } });
});
const database = output.match(/^DATABASE (agiru_owned_gate_[0-9]+_http_credentials)$/m)?.[1];
assert.ok(database, "native fixture must identify its owned database");
after(async () => {
  server.stdin.end("Q");
  const timer = setTimeout(() => server.kill(), 15000);
  try { assert.equal(await closed, 0, "authentication fixture shutdown must remove its owned database"); }
  finally { clearTimeout(timer); }
});
for (const suffix of ["", ".second"]) {
  await execute("podman", ["cp", `${container}:${nativePath}${suffix}`, `${proof}/auth.json${suffix}`]);
  await chmod(`${proof}/auth.json${suffix}`, 0o600);
}
const first = await readAuth(`${proof}/auth.json`);
const second = await readAuth(`${proof}/auth.json.second`);
async function sql(statement) {
  const result = await execute("podman", ["exec", "--user", "1000:1001", container,
    "psql", "-X", "-A", "-t", "-v", "ON_ERROR_STOP=1",
    `postgresql://agiru:agiru@127.0.0.1:5432/${database}`, "-c", statement]);
  return result.stdout.trim();
}
const count = () => sql("SELECT count(*) FROM authenticated_receipts");

test("missing, forged identity, cookie, URL and malformed credentials never authenticate", async () => {
  const before = await count();
  for (const headers of [{}, { "X-User": "FIRST USER" }, { Cookie: "agiru_session=forged" },
    { Authorization: "Bearer invalid" }, { Authorization: "Basic anonymous" }]) {
    const response = await fetch(origin + "/?token=forged&user=FIRST%20USER", { headers });
    assert.equal(response.status, 401);
    assert.equal(response.headers.get("www-authenticate"), 'Bearer realm="agiru"');
    assert.equal(response.headers.get("cache-control"), "no-store");
    assert.ok(!(await response.text()).includes("FIRST USER"));
  }
  assert.equal(await count(), before);
});

test("native authentication resolves two exact PostgreSQL users without shared worker identity", async () => {
  for (const [headers, name] of [[first, "FIRST USER"], [second, "SECOND USER"], [first, "FIRST USER"]]) {
    const response = await fetch(origin + "/?page=50400", { headers });
    assert.equal(response.status, 200);
    assert.equal(response.headers.get("x-fixture-user"), name);
    assert.ok(!(await response.text()).includes(headers.authorization));
  }
  assert.equal(await sql("SELECT user_security_id::text FROM authenticated_receipts ORDER BY ctid"),
    "00000000-0000-0000-0000-000000000001\n00000000-0000-0000-0000-000000000002\n00000000-0000-0000-0000-000000000001");
});

test("external shell CMD and actual MCP read the same authenticated semantic HTML", async () => {
  const before = Number(await count());
  const env = { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` };
  const cmd = await execute(process.execPath, ["build/client/cmd.mjs", "--json", "read", "/?page=50400"], { env });
  assert.equal(cmd.stderr, "");
  const cmdResult = JSON.parse(cmd.stdout);
  const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
  const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
  const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
  const transport = new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"], env });
  const client = new Client({ name: "agiru-authentication-gate", version: "1" });
  await client.connect(transport);
  try {
    const result = await client.callTool({ name: "agiru_read", arguments: { path: "/?page=50400" } });
    assert.ok(!result.isError);
    assert.deepEqual(result.structuredContent, cmdResult);
  } finally { await client.close(); }
  assert.equal(Number(await count()), before + 2);
  assert.equal(await sql("SELECT count(*) FROM authenticated_receipts WHERE user_security_id = '00000000-0000-0000-0000-000000000002'"), "1");
});

test("a valid credential is not an implementation of undeclared ERP writes", async () => {
  const before = await count();
  const response = await fetch(origin + "/commands", { method: "POST", headers: first, body: "operation=set&text=forged" });
  assert.equal(response.status, 405);
  assert.equal(response.headers.get("allow"), "GET");
  assert.equal(await count(), before);
});

test("disabling an account is rechecked before the next request; another user's credential remains valid", async () => {
  const before = await count();
  await sql('UPDATE "User" SET "State" = 1 WHERE "User Security ID" = \'00000000-0000-0000-0000-000000000001\'');
  assert.equal((await fetch(origin + "/", { headers: first })).status, 401);
  assert.equal(await count(), before);
  assert.equal((await fetch(origin + "/", { headers: second })).status, 200);
  await sql('UPDATE "User" SET "State" = 0');
  assert.equal((await fetch(origin + "/", { headers: first })).status, 200);
});

test("an expired system account refuses even while its bearer credential is unexpired", async () => {
  const before = await count();
  await sql('UPDATE "User" SET "Expiry Date" = \'2000-01-01 00:00:00\' WHERE "User Security ID" = \'00000000-0000-0000-0000-000000000001\'');
  assert.equal((await fetch(origin + "/", { headers: first })).status, 401);
  assert.equal(await count(), before);
  await sql('UPDATE "User" SET "Expiry Date" = \'1753-01-01 00:00:00\'');
});

test("SQL revocation takes effect across requests without restarting the server", async () => {
  const before = await count();
  await sql("UPDATE agiru_client.credentials SET revoked_at = clock_timestamp() WHERE user_security_id = '00000000-0000-0000-0000-000000000001'");
  assert.equal((await fetch(origin + "/", { headers: first })).status, 401);
  assert.equal(await count(), before);
  const response = await fetch(origin + "/", { headers: second });
  assert.equal(response.status, 200);
  assert.equal(response.headers.get("x-fixture-user"), "SECOND USER");
});

test("SQL expiry takes effect before any accepted command receipt", async () => {
  const before = await count();
  await sql("UPDATE agiru_client.credentials SET issued_at = clock_timestamp() - interval '2 hours', expires_at = clock_timestamp() - interval '1 hour'");
  assert.equal((await fetch(origin + "/", { headers: second })).status, 401);
  assert.equal(await count(), before);
});

test("credentials are not persisted as plaintext, echoed, logged or held on idle PostgreSQL connections", async () => {
  const verifiers = await sql("SELECT digest FROM agiru_client.credentials ORDER BY digest");
  assert.match(verifiers, /^[0-9a-f]{64}\n[0-9a-f]{64}$/);
  for (const auth of [first, second]) {
    assert.ok(!verifiers.includes(auth.authorization.slice(7)));
    assert.ok(!output.includes(auth.authorization.slice(7)));
    assert.ok(!diagnostic.includes(auth.authorization.slice(7)));
  }
  assert.equal(await sql("SELECT count(*) FROM pg_stat_activity WHERE datname = current_database()"), "1");
});
