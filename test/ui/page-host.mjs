import assert from "node:assert/strict";
import { test, after } from "node:test";
import { spawn, execFile } from "node:child_process";
import { promisify } from "node:util";
import { chmod } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient, readAuth } from "../../build/client/http.mjs";
import { commandEnvelope, parsePage } from "../../build/client/profile.mjs";

const execute = promisify(execFile);
const container = process.env.AGIRU_DEV_CONTAINER ?? "agiru-dev";
const origin = `http://127.0.0.1:${process.env.AGIRU_DEV_HTTP_PORT ?? "8080"}`;
const native = process.env.AGIRU_PAGE_HOST_NATIVE;
const proof = process.env.AGIRU_PAGE_HOST_PROOF;
const server = spawn("make", ["--no-print-directory", "dev-exec",
  `COMMAND=${process.env.AGIRU_PAGE_HOST_PRELOAD ? `env LD_PRELOAD=${process.env.AGIRU_PAGE_HOST_PRELOAD} ` : ""}${native}/host ${native}/auth.json ${origin}`],
  { env: { ...process.env, AGIRU_DEV_INTERACTIVE: "1" }, stdio: ["pipe", "pipe", "pipe"] });
let output = "", diagnostic = "";
server.stdout.on("data", chunk => { output += chunk; });
server.stderr.on("data", chunk => { diagnostic += chunk; });
const closed = new Promise(resolve => server.once("close", resolve));
after(async () => {
  server.stdin.end("Q");
  const timer = setTimeout(() => server.kill(), 15000);
  try { assert.equal(await closed, 0, "fixture must shut down and drop its owned database"); }
  finally { clearTimeout(timer); }
});
await new Promise((resolve, reject) => {
  const timer = setTimeout(() => reject(new Error("Generated page host did not start")), 15000);
  server.once("error", reject);
  server.once("close", () => { clearTimeout(timer); reject(new Error("Generated page host exited before readiness")); });
  server.stdout.on("data", () => { if (output.includes("READY\n")) { clearTimeout(timer); resolve(); } });
});
const database = output.match(/^DATABASE (agiru_owned_gate_[0-9]+_page_host)$/m)?.[1];
assert.ok(database);
for (const suffix of ["", ".second"]) {
  await execute("podman", ["cp", `${container}:${native}/auth.json${suffix}`, `${proof}/auth.json${suffix}`]);
  await chmod(`${proof}/auth.json${suffix}`, 0o600);
}
const first = await readAuth(`${proof}/auth.json`);
const second = await readAuth(`${proof}/auth.json.second`);
const client = new AgentClient(origin, first);
async function sql(statement) {
  const result = await execute("podman", ["exec", "--user", "1000:1001", container,
    "psql", "-X", "-A", "-t", "-v", "ON_ERROR_STOP=1",
    `postgresql://agiru:agiru@127.0.0.1:5432/${database}`, "-c", statement]);
  return result.stdout.trim();
}
const field = (result, name) => result.page.controls.find(control => control.identity === name)?.scalar?.value;
const operation = (result, name) => {
  const command = result.page.controls.find(control => control.identity === name)?.operation;
  assert.ok(command, `native page must advertise ${name}`);
  return { page: result.page.handle, revision: result.page.revision, ...command };
};
const path = result => `/?handle=${result.page.handle}`;
async function post(result, name, text, headers = first) {
  const selected = operation(result, name);
  const envelope = commandEnvelope(result.page, selected, text);
  const body = new URLSearchParams(envelope.fields).toString();
  const response = await fetch(origin + envelope.path, { method: "POST",
    headers: { ...headers, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body });
  return { response, body, html: await response.text() };
}
const opened = await client.read("/?page=50341&company=Fixture%20%2B%20Company");

test("native generated list retains its row and handle across request-local connections", async () => {
  assert.equal(opened.page.page, "50341");
  assert.equal(field(opened, "ID"), "1");
  assert.equal(field(opened, "Value"), "11");
  assert.equal(opened.page.revision, "0");
  assert.deepEqual(await client.read(path(opened)), opened);
  assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), "2");
});

let moved, card, saved, originalPost;
test("external agent navigates list to exact selected card through the common lifecycle commands", async () => {
  moved = await client.execute(path(opened), operation(opened, "$agiru.next"));
  assert.equal(field(moved, "ID"), "2");
  assert.equal(moved.page.revision, "1");
  card = await client.execute(path(moved), operation(moved, "$agiru.card"));
  assert.equal(card.page.page, "50340");
  assert.equal(field(card, "ID"), "2");
  assert.equal(field(card, "Value"), "22");
  assert.equal(card.page.handle, opened.page.handle);
  assert.equal(card.page.unsupported, 6, "computed variable bindings remain counted gaps");
});

test("actual shell CMD validates/saves a generated AL field and independent SQL attributes its exact value", async () => {
  const request = { path: path(card), ...operation(card, "Value"), text: "55" };
  delete request.enabled;
  const envelope = commandEnvelope(card.page, operation(card, "Value"), "55");
  originalPost = new URLSearchParams(envelope.fields).toString();
  const result = await execute(process.execPath, ["build/client/cmd.mjs", "--json", "execute", JSON.stringify(request)],
    { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
  assert.equal(result.stderr, "");
  saved = JSON.parse(result.stdout);
  assert.equal(field(saved, "Value"), "55");
  assert.equal(saved.page.revision, "3");
  assert.equal(await sql('SELECT "Value"::text || \'|\' || "SystemModifiedBy"::text FROM "Navigation Row" WHERE "ID" = 2'),
    "55|00000000-0000-0000-0000-000000000001");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_commands WHERE outcome = 'complete'"), "3");
  const current = await client.read(path(saved));
  assert.deepEqual(current, saved);
  saved = current;
});

test("identical completed command replays durably; altered payload and stale revision never execute again", async () => {
  const before = await sql('SELECT count(*) FROM ui_writes');
  const send = body => fetch(origin + "/commands", { method: "POST",
    headers: { ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body });
  const duplicate = await send(originalPost);
  assert.equal(duplicate.status, 200);
  assert.deepEqual(parsePage(await duplicate.text()), saved.page);
  const changed = new URLSearchParams(originalPost);
  changed.set("text", "99");
  assert.equal((await send(changed.toString())).status, 409);
  const old = await post(card, "$agiru.save");
  assert.equal(old.response.status, 409);
  assert.equal(await sql('SELECT count(*) FROM ui_writes'), before);
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_commands"), "3");
});

test("actual MCP saves the same generated page; native receipts and SQL count exactly one operation", async () => {
  const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
  const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
  const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
  const transport = new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"],
    env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
  const mcp = new Client({ name: "agiru-page-host-gate", version: "1" });
  await mcp.connect(transport);
  try {
    const request = { path: path(saved), ...operation(saved, "$agiru.save") };
    delete request.enabled;
    const reply = await mcp.callTool({ name: "agiru_execute", arguments: request });
    assert.ok(!reply.isError);
    assert.equal(reply.structuredContent.page.revision, "4");
    assert.equal(field(reply.structuredContent, "Value"), "55");
    const current = await client.read(path(reply.structuredContent));
    assert.deepEqual(current, reply.structuredContent);
    saved = current;
  } finally { await mcp.close(); }
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_commands WHERE outcome = 'complete'"), "4");
});

test("returning to the retained list rereads the committed selected row rather than stale card data", async () => {
  const back = await client.execute(path(saved), operation(saved, "$agiru.back"));
  assert.equal(back.page.page, "50341");
  assert.equal(field(back, "ID"), "2");
  assert.equal(field(back, "Value"), "55");
  saved = await client.execute(path(back), operation(back, "$agiru.card"));
  assert.equal(field(saved, "Value"), "55");
});

test("foreign identities, company names, unsupported URLs and forged form authority refuse", async () => {
  assert.equal((await fetch(origin + path(saved), { headers: second })).status, 410);
  assert.equal((await fetch(origin + "/?page=50341&company=Other", { headers: first })).status, 403);
  for (const target of ["/?page=50341&page=50340", "/?page=50341&mode=Unknown", "/?page=50341&filter=unsupported",
    "/?page=50341&company=Fixture+%2B+Company"]) {
    assert.ok((await fetch(origin + target, { headers: first })).status >= 400);
  }
  const envelope = commandEnvelope(saved.page, operation(saved, "Value"), "66");
  const body = new URLSearchParams(envelope.fields);
  body.set("csrf", "forged");
  assert.equal((await fetch(origin + "/commands", { method: "POST", headers: {
    ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body })).status, 403);
  assert.equal((await fetch(origin + "/commands", { method: "POST", headers: {
    ...first, Origin: "https://foreign.example", "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams(envelope.fields) })).status, 400);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 2'), "55");
});

test("SQL permission revocation affects existing page reads before values are returned", async () => {
  await sql("UPDATE ui_grants SET readable = false WHERE user_security_id = '00000000-0000-0000-0000-000000000001'");
  const response = await fetch(origin + path(saved), { headers: first });
  assert.equal(response.status, 403);
  assert.ok(!(await response.text()).includes('data-value="55"'));
  await sql("UPDATE ui_grants SET readable = true");
  assert.equal((await fetch(origin + path(saved), { headers: first })).status, 200);
});

test("invalid AL input cannot survive rollback as a reusable authoritative page", async () => {
  const rejected = await post(saved, "Value", "not an integer");
  assert.equal(rejected.response.status, 400);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 2'), "55");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_commands WHERE outcome = 'failed'"), "1");
  assert.equal((await fetch(origin + path(saved), { headers: first })).status, 410);
});

test("PostgreSQL owns revision fencing and expiry; idle windows retain no database leases", async () => {
  const fresh = await client.read("/?page=50341");
  await sql(`UPDATE agiru_client.page_contexts SET revision = revision + 1 WHERE handle = '${fresh.page.handle}'`);
  assert.equal((await fetch(origin + path(fresh), { headers: first })).status, 409);
  const expired = await client.read("/?page=50341");
  await sql(`UPDATE agiru_client.page_contexts SET expires_at = clock_timestamp() - interval '1 second' WHERE handle = '${expired.page.handle}'`);
  assert.equal((await fetch(origin + path(expired), { headers: first })).status, 410);
  assert.equal(await sql("SELECT count(*) FROM pg_stat_activity WHERE datname = current_database()"), "1");
  for (const auth of [first, second]) {
    assert.ok(!output.includes(auth.authorization.slice(7)));
    assert.ok(!diagnostic.includes(auth.authorization.slice(7)));
  }
});

test("a noncommitted AL action rolls back; production Commit survives a later error without a successful receipt", async () => {
  for (const [name, expected] of [["WriteAndFail", "11"], ["CommitAndFail", "111"]]) {
    const fresh = await client.read("/?page=50347&mode=Edit");
    const failed = await post(fresh, name);
    assert.equal(failed.response.status, 500);
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'), expected);
    assert.equal((await fetch(origin + path(fresh), { headers: first })).status, 410);
    const receipt = operation(fresh, name).command;
    assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id = '${receipt}'`), "failed");
    assert.equal((await fetch(origin + "/commands", { method: "POST", headers: {
      ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body: failed.body })).status, 410);
  }
  assert.equal(await sql('SELECT count(*) FROM ui_writes WHERE "value" = 111'), "1");
});
