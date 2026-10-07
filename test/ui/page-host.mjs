import assert from "node:assert/strict";
import { test, after } from "node:test";
import { spawn, execFile } from "node:child_process";
import { promisify } from "node:util";
import { chmod } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient, readAuth } from "../../build/client/http.mjs";
import { commandEnvelope, parsePage } from "../../build/client/profile.mjs";
import { ServerConfigs } from "./server-config.mjs";
import { launchBrowser, openBrowserPage, assertBrowserPage, browserAction } from "./browser-client.mjs";
import { renderAscii } from "../../build/client/ascii.mjs";

const execute = promisify(execFile);
const container = process.env.AGIRU_DEV_CONTAINER ?? "agiru-dev";
const origin = `http://127.0.0.1:${process.env.AGIRU_DEV_HTTP_PORT ?? "8080"}`;
const native = process.env.AGIRU_PAGE_HOST_NATIVE;
const proof = process.env.AGIRU_PAGE_HOST_PROOF;
const nativeApplication = process.env.AGIRU_PAGE_HOST_APPLICATION === "1";
const disableTryWrites = process.env.AGIRU_TRY_WRITE_DISABLED === "1";
const rowLimit = Number(process.env.AGIRU_LIST_ROWS ?? "40");
const configurations = new ServerConfigs(container, native, proof);
let serverConfig;
let application, applicationPid, applicationClosed;
const server = spawn("make", ["--no-print-directory", "dev-exec",
  `COMMAND=${process.env.AGIRU_PAGE_HOST_PRELOAD ? `env LD_PRELOAD=${process.env.AGIRU_PAGE_HOST_PRELOAD} ` : ""}${native}/host ${native}/auth.json ${origin}${nativeApplication ? " --seed-only" : ""}`],
  { env: { ...process.env, AGIRU_DEV_INTERACTIVE: "1" }, stdio: ["pipe", "pipe", "pipe"] });
let output = "", diagnostic = "";
server.stdout.on("data", chunk => { output += chunk; });
server.stderr.on("data", chunk => { diagnostic += chunk; });
const closed = new Promise(resolve => server.once("close", resolve));
after(async () => {
  if (applicationPid) {
    await execute("podman", ["exec", "--user", "1000:1001", container, "kill", "-TERM", applicationPid]);
    assert.equal(await applicationClosed, 0, "agiru serve must drain and shut down cleanly");
  }
  await configurations.clean();
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
const dsn = `postgresql://agiru:agiru@127.0.0.1:5432/${database}`;
if (nativeApplication) {
  serverConfig = await configurations.write({ database: dsn, company: "Fixture + Company", origin,
    pages: { list_rows: rowLimit },
    transactions: { disable_write_inside_try_functions: disableTryWrites } });
  application = spawn("podman", ["exec", "--user", "1000:1001", container,
    "env", ...(process.env.AGIRU_PAGE_HOST_PRELOAD ? [`LD_PRELOAD=${process.env.AGIRU_PAGE_HOST_PRELOAD}`] : []),
    `${native}/agiru`, "serve", "--config", serverConfig],
    { stdio: ["ignore", "pipe", "pipe"] });
  applicationClosed = new Promise(resolve => application.once("close", resolve));
  let ready = "";
  await new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error("agiru serve did not start")), 15000);
    application.once("error", error => { clearTimeout(timer); reject(error); });
    application.once("close", code => { clearTimeout(timer); reject(new Error(`agiru serve exited: ${code}; ${diagnostic}`)); });
    application.stderr.on("data", chunk => { diagnostic += chunk; });
    application.stdout.on("data", chunk => {
      ready += chunk;
      const match = ready.match(/^READY ([1-9][0-9]*) 18080$/m);
      if (match) { applicationPid = match[1]; clearTimeout(timer); resolve(); }
    });
  });
}

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
  assert.equal(opened.page.profile, "2");
  assert.equal(opened.page.window.limit, String(nativeApplication ? rowLimit : 40));
  assert.equal(opened.page.rows.length, 2);
  assert.deepEqual(opened.page.rows.map(row => row.controls.find(cell => cell.identity === "ID").scalar.value), ["1", "2"]);
  assert.equal(opened.page.rows[0].selected, true);
  assert.equal(opened.page.revision, "0");
  assert.deepEqual(await client.read(path(opened)), opened);
  assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), "2");
});

test("native list rows retain exact Decimal, Int64, Code and untrusted Unicode data", async () => {
  for (const row of opened.page.rows) {
    const values = Object.fromEntries(row.controls.filter(control => control.scalar)
      .map(control => [control.identity, control.scalar.value]));
    assert.equal(values.Amount, "0.12345678901234567890");
    assert.equal(values.Exact, "9223372036854775807");
    assert.equal(values.Label, "Grüezi <script> 東京 🔧");
    assert.equal(values.Code, row.selected ? "0001" : "20");
  }
  assert.equal(await sql('SELECT "Amount"::text || \'|\' || "Exact"::text FROM "Navigation Row" WHERE "ID" = 1'),
    "0.12345678901234567890|9223372036854775807");
  assert.ok(renderAscii(opened.page).includes('Decimal="0.12345678901234567890"'));
});

let moved, card, saved, originalPost;
test("external agent navigates list to exact selected card through the common lifecycle commands", async () => {
  moved = await client.execute(path(opened), operation(opened, opened.page.rows[1].select.control));
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
  await sql(nativeApplication
    ? `UPDATE "Tenant Permission" SET "Read Permission" = 0 WHERE "Role ID" = 'EDITOR' AND "Object Type" = 0`
    : "UPDATE ui_grants SET readable = false WHERE user_security_id = '00000000-0000-0000-0000-000000000001'");
  const response = await fetch(origin + path(saved), { headers: first });
  assert.equal(response.status, 403);
  assert.ok(!(await response.text()).includes('data-value="55"'));
  await sql(nativeApplication
    ? `UPDATE "Tenant Permission" SET "Read Permission" = 1 WHERE "Role ID" = 'EDITOR' AND "Object Type" = 0`
    : "UPDATE ui_grants SET readable = true");
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

test("authorized page actions cannot bypass denied transitive table reads or inserts", async () => {
  for (const name of ["ReadOtherTable", "WriteOtherTable"]) {
    const fresh = await client.read("/?page=50347&mode=Edit");
    const failed = await post(fresh, name);
    assert.equal(failed.response.status, 403);
    assert.ok(failed.html.includes("TableData 50348 Restricted Row"));
    assert.ok(!failed.html.includes('data-value="999"'));
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'), "11");
    assert.equal(await sql('SELECT count(*) FROM "Restricted Row"'), "1");
    assert.equal((await fetch(origin + path(fresh), { headers: first })).status, 410);
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

if (nativeApplication) {
  const operator = (command, ...args) => execute("podman", ["exec", "--user", "1000:1001", container,
    `${native}/agiru`, command, ...args, ...(command === "serve" ? [] : ["--database", dsn])]);

  test("startup configuration selects TryFunction write policy with independent SQL effects", async () => {
    try {
      const fresh = await client.read("/?page=50347&mode=Edit");
      const caught = await post(fresh, "CaughtTryWrite");
      assert.equal(caught.response.status, 200);
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'),
        disableTryWrites ? "111" : "777");
    } finally { await sql('UPDATE "Navigation Row" SET "Value" = 111 WHERE "ID" = 1'); }
  });

  test("trusted client storage initialization is idempotent and grants no ERP rights", async () => {
    await operator("client-init");
    assert.equal(await sql('SELECT count(*) FROM "User"'), "2");
    assert.equal(await sql('SELECT count(*) FROM "Access Control"'), "2");
    assert.equal(await sql('SELECT count(*) FROM "Tenant Permission"'), "7");
  });

  test("operator credential issuance authenticates an existing account without granting writes", async () => {
    const issued = await operator("client-token", "--user", "00000000-0000-0000-0000-000000000002");
    const auth = JSON.parse(issued.stdout);
    assert.match(auth.authorization, /^Bearer ag1_/);
    const reader = new AgentClient(origin, auth);
    const view = await reader.read("/?page=50341");
    const next = await reader.execute(path(view), operation(view, "$agiru.card"));
    const denied = await post(next, "Value", "999", auth);
    assert.equal(denied.response.status, 403);
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'), "111");
    for (const seconds of ["0", "86401", "1x"]) {
      await assert.rejects(operator("client-token", "--user", "00000000-0000-0000-0000-000000000002", "--seconds", seconds));
    }
  });

  test("missing compiled system declarations never become a synthetic SUPER grant", async () => {
    await sql(`UPDATE "Access Control" SET "Scope" = 0, "Role ID" = 'SUPER' WHERE "Role ID" = 'EDITOR'`);
    const denied = await fetch(origin + "/?page=50341", { headers: first });
    assert.ok(denied.status >= 400);
    assert.ok(!(await denied.text()).includes('data-value="111"'));
    await sql(`UPDATE "Access Control" SET "Scope" = 1, "Role ID" = 'EDITOR' WHERE "Role ID" = 'SUPER'`);
    assert.equal((await fetch(origin + "/?page=50341", { headers: first })).status, 200);
  });

  test("server settings cannot be supplied as CLI flags and invalid files refuse before a listener starts", async () => {
    for (const [args, message] of [[[], /required option --config/],
      [["--database", dsn], /service option --database/],
      [["--workers", "2"], /service option --workers/],
      [["--disable-write-inside-try-functions", "true"], /service option --disable/],
      [["--config", serverConfig, "--config", serverConfig], /service option --config/],
      [["--config", `${native}/missing.json`], /invalid server configuration/]]) {
      await assert.rejects(operator("serve", ...args), error => message.test(error.stderr));
    }
    for (const [name, patch, message] of [
      ["wrong-company", { company: "Missing" }, /configured company/],
      ["zero-port", { http: { port: 0 } }, /invalid server configuration/],
      ["many-workers", { http: { workers: 257 } }, /invalid server configuration/],
      ["unknown-field", { unknown: "DO-NOT-ECHO" }, /invalid server configuration/]]) {
      const file = await configurations.write({ database: dsn, company: "Fixture + Company", origin,
        ...patch }, `${name}.json`);
      await assert.rejects(operator("serve", "--config", file), error =>
        message.test(error.stderr) && !error.stderr.includes("DO-NOT-ECHO"));
    }
  });

  test("a flat database with multiple original companies cannot be relabeled as one company", async () => {
    await sql(`INSERT INTO "Company" ("Name") VALUES ('Other Company')`);
    try {
      await assert.rejects(operator("serve", "--config", serverConfig),
        error => error.stderr.includes("configured company must be the only company"));
    } finally {
      await sql(`DELETE FROM "Company" WHERE "Name" = 'Other Company'`);
    }
  });
}

test("shared HTTP list windows obey the trusted bound at zero/one/39/40/41 and never accept a URL limit", async () => {
  const bound = nativeApplication ? rowLimit : 40;
  const rowIds = result => result.page.rows.map(row => row.controls.find(cell => cell.identity === "ID").scalar.value);
  for (const population of [0, 1, 39, 40, 41, 81]) {
    await sql(`DELETE FROM "Navigation Row";
      INSERT INTO "Navigation Row" ("ID","Value","Label","Amount","Exact","Code")
      SELECT n,n,'ROW 東京 ' || n,0.12345678901234567890,9223372036854775807,lpad(n::text,4,'0')
      FROM generate_series(1,${population}) n`);
    const fresh = await client.read("/?page=50341");
    assert.equal(fresh.page.window.limit, String(bound));
    assert.equal(fresh.page.window.more, population > bound);
    assert.deepEqual(rowIds(fresh), Array.from({ length: Math.min(population, bound) }, (_, index) => String(index + 1)));
    assert.equal(fresh.page.rows.filter(row => row.selected).length, population ? 1 : 0);
    assert.deepEqual(await client.read(path(fresh)), fresh, "a GET must not repeat AL loading or change row handles");
    if (population > bound) {
      const next = await client.execute(path(fresh), operation(fresh, "$agiru.next"));
      assert.deepEqual(rowIds(next), Array.from({ length: Math.min(population - bound, bound) }, (_, index) => String(bound + index + 1)));
      const previous = await client.execute(path(next), operation(next, "$agiru.previous"));
      assert.deepEqual(rowIds(previous), rowIds(fresh));
      const last = await client.execute(path(previous), operation(previous, "$agiru.last"));
      assert.deepEqual(rowIds(last), Array.from({ length: Math.min(population, bound) }, (_, index) => String(Math.max(1, population - bound + 1) + index)));
    }
    assert.equal((await fetch(origin + "/?page=50341&limit=1", { headers: first })).status, 400);
    assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), String(population));
  }
  const fresh = await client.read("/?page=50341");
  const received = await fetch(origin + path(fresh), { headers: first });
  assert.equal(received.status, 200);
  const decoded = parsePage(await received.text());
  assert.deepEqual(decoded, fresh.page);
  for (const bad of [
    ["data-limit", 'data-limit="' + bound + '"', 'data-limit="1"'],
    ["duplicate row", `data-row="${fresh.page.rows[1].handle}"`, `data-row="${fresh.page.rows[0].handle}"`],
    ["missing selection", 'data-selected="true"', 'data-selected="false"'],
  ]) {
    const response = await fetch(origin + path(fresh), { headers: first });
    const html = await response.text();
    assert.ok(html.includes(bad[1]));
    assert.throws(() => parsePage(html.replace(bad[1], bad[2])), error => error.code === "ProfileRefused");
  }
});

test("external CMD/MCP and actual Chromium consume identical native list rows and row-selection commands", async () => {
  const fresh = await client.read("/?page=50341");
  const cmd = await execute(process.execPath, ["build/client/cmd.mjs", "--json", "read", path(fresh)],
    { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
  assert.deepEqual(JSON.parse(cmd.stdout), fresh);
  const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
  const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
  const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
  const mcp = new Client({ name: "agiru-list-window", version: "1" });
  const transport = new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"],
    env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
  const browser = await launchBrowser();
  try {
    await mcp.connect(transport);
    const reply = await mcp.callTool({ name: "agiru_read", arguments: { path: path(fresh) } });
    assert.deepEqual(reply.structuredContent, fresh);
    const opened = await openBrowserPage(browser, origin, path(fresh), first.authorization);
    assert.equal(opened.response.status(), 200);
    await assertBrowserPage(opened.page, fresh.page);
    await opened.page.screenshot({ path: `${proof}/list-window.png`, fullPage: true });
    const selected = await browserAction(opened.page, origin, fresh.page, fresh.page.rows[1].select.control);
    assert.equal(field(selected, "ID"), "2");
    assert.equal(selected.page.rows[1].selected, true);
    assert.deepEqual(await client.read(path(fresh)), selected);
    assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), "81");
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 2'), "2");
  } finally { await mcp.close(); await browser.close(); }
});
