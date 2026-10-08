import assert from "node:assert/strict";
import { test, after } from "node:test";
import { spawn, execFile } from "node:child_process";
import { promisify } from "node:util";
import { chmod } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient, readAuth } from "../../build/client/http.mjs";
import { commandEnvelope, parseFailure, parsePage } from "../../build/client/profile.mjs";
import { ServerConfigs } from "./server-config.mjs";
import { launchBrowser, openBrowserPage, assertBrowserPage, browserAction, browserSet, browserFailure, finishedResponse } from "./browser-client.mjs";
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
    http: { workers: 1 },
    pages: { list_rows: rowLimit, dialog_timeout_seconds: 5, response_wait_ms: 1 },
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
async function post(result, name, text, headers = first, complete = true) {
  const selected = operation(result, name);
  const envelope = commandEnvelope(result.page, selected, text);
  const body = new URLSearchParams(envelope.fields).toString();
  let response = await fetch(origin + envelope.path, { method: "POST",
    headers: { ...headers, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body });
  const resultResponse = await finish(response, headers, complete);
  return { ...resultResponse, body };
}

async function finish(response, headers = first, complete = true) {
  let html = await response.text();
  const deadline = Date.now() + 15000;
  while (complete && response.status === 200) {
    const interaction = parsePage(html).interaction;
    if (interaction?.state !== "working" && interaction?.poll?.state !== "failed") break;
    assert.ok(Date.now() < deadline, "native call must finish without repeating the POST");
    await new Promise(resolve => setTimeout(resolve, 50));
    response = await fetch(origin + (interaction.poll?.path ?? `/calls/${interaction.call}`), { headers, signal: AbortSignal.timeout(15000) });
    html = await response.text();
  }
  return { response, html };
}
const opened = await client.read("/?page=50341&company=Fixture%20%2B%20Company");

async function modalAdapter(name, fresh) {
  let browser, web, mcp;
  const env = { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` };
  if (name === "Web") {
    browser = await launchBrowser();
    web = await openBrowserPage(browser, origin, path(fresh), first.authorization);
    await assertBrowserPage(web.page, fresh.page);
  } else if (name === "MCP") {
    const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
    const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
    const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
    mcp = new Client({ name: "agiru-modal-parity", version: "1" });
    await mcp.connect(new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"], env }));
  }
  return {
    async failure(current, identity, text) {
      if (web) return browserFailure(web.page, origin, current.page, identity, text);
      const { enabled: _enabled, ...command } = operation(current, identity);
      const input = { path: path(current), ...command, ...(text === undefined ? {} : { text }) };
      if (mcp) {
        const result = await mcp.callTool({ name: "agiru_execute", arguments: input });
        assert.equal(result.isError, true);
        return result.structuredContent;
      }
      try {
        await execute(process.execPath, ["build/client/cmd.mjs", "--json", "execute", JSON.stringify(input)], { env });
      } catch (error) {
        assert.equal(error.code, 2);
        return JSON.parse(error.stderr);
      }
      assert.fail("CMD must report the native modal error");
    },
    async action(current, identity, text) {
      if (web) return text === undefined ? browserAction(web.page, origin, current.page, identity)
        : browserSet(web.page, origin, current.page, identity, text);
      const { enabled: _enabled, ...command } = operation(current, identity);
      const input = { path: path(current), ...command, ...(text === undefined ? {} : { text }) };
      if (mcp) {
        const result = await mcp.callTool({ name: "agiru_execute", arguments: input });
        assert.notEqual(result.isError, true, JSON.stringify(result.structuredContent));
        assert.ok(result.content.some(item => item.type === "text" && item.text.includes("unsupported=")));
        return result.structuredContent;
      }
      return JSON.parse((await execute(process.execPath, ["build/client/cmd.mjs", "--json", "execute", JSON.stringify(input)], { env })).stdout);
    },
    async screenshot(name) { if (web) await web.page.screenshot({ path: `${proof}/${name}.png`, fullPage: true }); },
    async close() { await mcp?.close(); await browser?.close(); },
  };
}

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
  assert.equal(card.page.unsupported, 0, "declared variable bindings have exact scalar transport");
  assert.equal(field(card, "LoadedValue"), "22");
  assert.equal(field(card, "OpeningMode"), "true");
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
  const duplicate = await finish(await send(originalPost));
  assert.equal(duplicate.response.status, 200);
  assert.deepEqual(parsePage(duplicate.html), saved.page);
  const changed = new URLSearchParams(originalPost);
  changed.set("text", "99");
  assert.equal((await finish(await send(changed.toString()))).response.status, 409);
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
  assert.equal((await finish(await fetch(origin + "/commands", { method: "POST", headers: {
    ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body }))).response.status, 403);
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
    assert.deepEqual(parseFailure(failed.html), { code: "AlError", command: receipt, outcome: "failed",
      message: name === "WriteAndFail" ? "rollback fixture error" : "durable fixture error" });
    assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id = '${receipt}'`), "failed");
    assert.equal((await fetch(origin + "/commands", { method: "POST", headers: {
      ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body: failed.body })).status, 410);
  }
  assert.equal(await sql('SELECT count(*) FROM ui_writes WHERE "value" = 111'), "1");
});

test("CMD/MCP/htmx preserve native AL diagnostics and failed receipts without implying rollback of prior commits", async () => {
  for (const transport of ["CMD", "MCP", "web"]) {
    await sql('UPDATE "Navigation Row" SET "Value" = 11 WHERE "ID" = 1');
    const browser = transport === "web" ? await launchBrowser() : undefined;
    let mcp;
    try {
      const fresh = await client.read("/?page=50347&mode=Edit");
      const { enabled: _enabled, ...selected } = operation(fresh, "CommitAndFail");
      const input = { path: path(fresh), ...selected };
      let error;
      if (transport === "CMD") {
        try {
          await execute(process.execPath, ["build/client/cmd.mjs", "execute", JSON.stringify(input)],
            { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
          assert.fail("the failed command must not produce successful CMD output");
        } catch (failure) {
          assert.equal(failure.code, 2);
          assert.equal(failure.stdout, "");
          error = JSON.parse(failure.stderr);
        }
      } else if (transport === "MCP") {
        const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
        const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
        const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
        mcp = new Client({ name: "agiru-native-error-test", version: "1" });
        await mcp.connect(new StdioClientTransport({ command: process.execPath,
          args: ["build/client/mcp.mjs"], env: { AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } }));
        const result = await mcp.callTool({ name: "agiru_execute", arguments: input });
        assert.equal(result.isError, true);
        error = result.structuredContent;
        assert.deepEqual(JSON.parse(result.content[0].text), error);
      } else {
        const opened = await openBrowserPage(browser, origin, input.path, first.authorization);
        const page = opened.page;
        await assertBrowserPage(page, fresh.page);
        const received = page.waitForResponse(response => response.url() === `${origin}/commands` && response.request().method() === "POST");
        await page.locator('[data-control="CommitAndFail"] button').click();
        const response = await finishedResponse(page, origin, await received);
        assert.equal(response.status(), 500);
        const failure = parseFailure(await response.text());
        error = { error: failure.code, message: failure.message, command: failure.command, outcome: failure.outcome };
        await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Server error"));
        assert.equal(await page.locator("#status").textContent(),
          `Server error AlError: durable fixture error outcome=failed command=${input.command}; prior explicit commits may persist.`);
        await assertBrowserPage(page, fresh.page);
      }
      assert.deepEqual(error, { error: "AlError", message: "durable fixture error", command: input.command, outcome: "failed" });
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'), "111");
      assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id = '${input.command}'`), "failed");
    } finally {
      await mcp?.close();
      await browser?.close();
      await sql('UPDATE "Navigation Row" SET "Value" = 111 WHERE "ID" = 1');
    }
  }
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
    assert.equal(await sql('SELECT count(*) FROM "Tenant Permission"'), "10");
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

test("a pending native write keeps one HTTP worker available, retains ownership and commits only once", async () => {
  const fresh = await client.read("/?page=50340&mode=Edit");
  const other = await client.read("/?page=50341");
  const old = await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1');
  await sql(`CREATE FUNCTION ui_wait() RETURNS trigger LANGUAGE plpgsql AS $body$
    BEGIN PERFORM pg_sleep(0.6); RETURN NEW; END $body$;
    CREATE TRIGGER ui_wait BEFORE UPDATE ON "Navigation Row" FOR EACH ROW EXECUTE FUNCTION ui_wait()`);
  try {
    const sent = await post(fresh, "Value", "9100", first, false);
    assert.equal(sent.response.status, 200);
    const pending = parsePage(sent.html);
    assert.equal(pending.interaction.state, "working");
    assert.equal(pending.interaction.originCommand, operation(fresh, "Value").command);
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'), old);
    assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id = '${pending.interaction.originCommand}'`), "started");
    const foreign = await fetch(origin + `/calls/${pending.interaction.call}`, { headers: second });
    assert.equal(foreign.status, 410);
    assert.deepEqual(await client.read(path(other)), other, "unrelated retained reads must not wait for the AL writer");
    const result = await client.read(`/calls/${pending.interaction.call}`);
    assert.equal(field(result, "Value"), "9100");
    assert.equal(BigInt(result.page.revision), BigInt(fresh.page.revision) + 1n);
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1'), "9100");
    assert.equal(await sql('SELECT count(*) FROM ui_writes WHERE value=9100'), "1");
    assert.deepEqual(await client.read(`/calls/${pending.interaction.call}`), result);
  } finally { await sql('DROP TRIGGER ui_wait ON "Navigation Row"; DROP FUNCTION ui_wait()'); }
});

test("failed native call polling preserves rollback, durable Commit and the original command diagnostic", async () => {
  await sql(`CREATE FUNCTION ui_wait() RETURNS trigger LANGUAGE plpgsql AS $body$
    BEGIN PERFORM pg_sleep(0.3); RETURN NEW; END $body$;
    CREATE TRIGGER ui_wait BEFORE UPDATE ON "Navigation Row" FOR EACH ROW EXECUTE FUNCTION ui_wait()`);
  try {
    for (const action of ["WriteAndFail", "CommitAndFail"]) {
      const before = await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1');
      const fresh = await client.read("/?page=50347&mode=Edit");
      const sent = await post(fresh, action, undefined, first, false);
      assert.equal(sent.response.status, 200);
      const pending = parsePage(sent.html);
      assert.equal(pending.interaction.originCommand, operation(fresh, action).command);
      await assert.rejects(client.read(`/calls/${pending.interaction.call}`), error =>
        error.code === "AlError" && error.outcome === "failed" && error.command === operation(fresh, action).command &&
        error.message === (action === "WriteAndFail" ? "rollback fixture error" : "durable fixture error"));
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'),
        String(BigInt(before) + (action === "CommitAndFail" ? 100n : 0n)));
      assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id='${pending.interaction.originCommand}'`), "failed");
    }
  } finally { await sql('DROP TRIGGER ui_wait ON "Navigation Row"; DROP FUNCTION ui_wait()'); }
});

test("external CMD MCP and htmx complete delayed native saves with identical typed and SQL effects", async () => {
  await sql(`CREATE FUNCTION ui_wait() RETURNS trigger LANGUAGE plpgsql AS $body$
    BEGIN PERFORM pg_sleep(0.3); RETURN NEW; END $body$;
    CREATE TRIGGER ui_wait BEFORE UPDATE ON "Navigation Row" FOR EACH ROW EXECUTE FUNCTION ui_wait()`);
  try {
    for (const [index, adapter] of ["CMD", "MCP", "web"].entries()) {
      const browser = adapter === "web" ? await launchBrowser() : undefined;
      let mcp;
      try {
        const fresh = await client.read("/?page=50340&mode=Edit");
        const text = String(9200 + index);
        const previousWrites = BigInt(await sql(`SELECT count(*) FROM ui_writes WHERE value=${text}`));
        const { enabled: _enabled, ...selected } = operation(fresh, "Value");
        const input = { path: path(fresh), ...selected, text };
        let result;
        if (adapter === "CMD") {
          const output = await execute(process.execPath, ["build/client/cmd.mjs", "--json", "execute", JSON.stringify(input)],
            { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
          result = JSON.parse(output.stdout);
        } else if (adapter === "MCP") {
          const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
          const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
          const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
          mcp = new Client({ name: "agiru-delayed-save", version: "1" });
          await mcp.connect(new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"],
            env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } }));
          const reply = await mcp.callTool({ name: "agiru_execute", arguments: input });
          assert.notEqual(reply.isError, true);
          result = reply.structuredContent;
        } else {
          const opened = await openBrowserPage(browser, origin, path(fresh), first.authorization);
          result = await browserSet(opened.page, origin, fresh.page, "Value", text);
        }
        assert.equal(field(result, "Value"), text);
        assert.equal(result.page.interaction, undefined);
        assert.deepEqual(await client.read(path(fresh)), result);
        assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), text);
        assert.equal(BigInt(await sql(`SELECT count(*) FROM ui_writes WHERE value=${text}`)), previousWrites + 1n);
        assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id='${input.command}'`), "complete");
      } finally { await mcp?.close(); await browser?.close(); }
    }
  } finally { await sql('DROP TRIGGER ui_wait ON "Navigation Row"; DROP FUNCTION ui_wait()'); }
});

test("native questions preserve the AL transaction, require explicit answers and roll back declines", async () => {
  const fresh = await client.read("/?page=50347&mode=Edit");
  const before = await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1');
  const writes = await sql('SELECT count(*) FROM ui_writes');
  const question = await client.execute(path(fresh), operation(fresh, "ConfirmWrite"));
  assert.equal(question.page.interaction.state, "confirm");
  assert.equal(question.page.interaction.defaultChoice, "1");
  assert.equal(question.page.interaction.originCommand, operation(fresh, "ConfirmWrite").command);
  assert.equal(question.page.interaction.prompt, `Save value ${BigInt(before) + 10n}?`);
  assert.deepEqual(question.page.messages.map(item => item.text), ["Before question <script> 東京."]);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), before);
  assert.equal(await sql('SELECT count(*) FROM ui_writes'), writes);
  const dialog = question.page.interaction.dialog;
  assert.equal(await sql(`SELECT answer IS NULL AND NOT closed FROM agiru_client.page_dialogs WHERE handle='${dialog}'`), "t");
  const original = operation(fresh, "ConfirmWrite").command;
  const selected = question.page.controls[0].identity;
  const envelope = commandEnvelope(question.page, operation(question, selected));
  const send = async (changes = {}, headers = first) => fetch(origin + "/answers", { method: "POST",
    headers: { ...headers, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams({ ...envelope.fields, ...changes }).toString() });
  assert.equal((await send({}, second)).status, 410);
  assert.equal((await send({ revision: "999" })).status, 409);
  assert.equal((await send({ csrf: "foreign" })).status, 403);
  assert.equal((await send({ control: "foreign" })).status, 400);
  assert.equal((await send({ command: `${dialog}_00`, control: `$agiru.answer_${dialog}_00` })).status, 400);
  assert.equal((await send({ command: `${dialog}_9`, control: `$agiru.answer_${dialog}_9` })).status, 400);
  assert.equal((await send({ command: "foreign_0", control: "$agiru.answer_foreign_0" })).status, 409);
  if (nativeApplication) {
    await sql(`UPDATE "Tenant Permission" SET "Execute Permission"=0 WHERE "Role ID"='EDITOR' AND "Object Type"=8 AND "Object ID"=50347`);
  } else {
    await sql("UPDATE ui_grants SET writable=false WHERE user_security_id='00000000-0000-0000-0000-000000000001'");
  }
  try { assert.equal((await send()).status, 403, "answering must reauthorize the original action"); }
  finally {
    if (nativeApplication) await sql(`UPDATE "Tenant Permission" SET "Execute Permission"=1 WHERE "Role ID"='EDITOR' AND "Object Type"=8 AND "Object ID"=50347`);
    else await sql("UPDATE ui_grants SET writable=true WHERE user_security_id='00000000-0000-0000-0000-000000000001'");
  }
  assert.deepEqual(await client.read(path(fresh)), question, "polls must not answer a presentation default");
  await assert.rejects(client.execute(path(question), operation(question, selected)), error =>
    error.message === "explicit decline" && error.command === original && error.outcome === "failed");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), before);
  assert.equal(await sql('SELECT count(*) FROM ui_writes'), writes);
  assert.equal(await sql(`SELECT answer::text || ':' || closed::text FROM agiru_client.page_dialogs WHERE handle='${dialog}'`), "0:true");
});

test("nested native questions reject replaced answers and preserve explicit Commit on later error", async () => {
  const fresh = await client.read("/?page=50347&mode=Edit");
  const before = BigInt(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'));
  const firstQuestion = await client.execute(path(fresh), operation(fresh, "TwoQuestions"));
  const yes = firstQuestion.page.controls[1].identity;
  const sent = await post(firstQuestion, yes);
  assert.equal(sent.response.status, 200);
  const secondQuestion = { page: parsePage(sent.html), status: 200 };
  assert.equal(secondQuestion.page.interaction.prompt, "Second?");
  assert.notEqual(secondQuestion.page.interaction.dialog, firstQuestion.page.interaction.dialog);
  const replay = await post(firstQuestion, yes);
  assert.deepEqual(parsePage(replay.html), secondQuestion.page);
  const changed = await post(firstQuestion, firstQuestion.page.controls[0].identity);
  assert.equal(changed.response.status, 409);
  const completed = await client.execute(path(secondQuestion), operation(secondQuestion, secondQuestion.page.controls[1].identity));
  assert.equal(field(completed, "Value"), String(before + 1n));
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), String(before + 1n));
  const committed = await client.read("/?page=50347&mode=Edit");
  const question = await client.execute(path(committed), operation(committed, "CommittedConfirm"));
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), String(before + 11n));
  await assert.rejects(client.execute(path(question), operation(question, question.page.controls[0].identity)), error =>
    error.message === "declined after commit" && error.command === operation(committed, "CommittedConfirm").command);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), String(before + 11n));
});

test("external CMD MCP and htmx explicitly answer native menus with matching messages and SQL effects", async () => {
  for (const [index, adapter] of ["CMD", "MCP", "web"].entries()) {
    const browser = adapter === "web" ? await launchBrowser() : undefined;
    let mcp;
    try {
      const fresh = await client.read("/?page=50347&mode=Edit");
      const input = selected => {
        const { enabled: _enabled, ...command } = selected;
        return { path: path({ page: { handle: command.page } }), ...command };
      };
      const agent = async selected => {
        if (adapter === "CMD") {
          const reply = await execute(process.execPath, ["build/client/cmd.mjs", "--json", "execute", JSON.stringify(input(selected))],
            { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
          return JSON.parse(reply.stdout);
        }
        const reply = await mcp.callTool({ name: "agiru_execute", arguments: input(selected) });
        assert.notEqual(reply.isError, true);
        assert.ok(reply.content.some(item => item.type === "text" &&
          item.text.includes(reply.structuredContent.page.interaction ? "Choose explicitly" : "Selected")));
        return reply.structuredContent;
      };
      if (adapter === "MCP") {
        const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
        const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
        const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
        mcp = new Client({ name: "agiru-explicit-dialog", version: "1" });
        await mcp.connect(new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"],
          env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } }));
      }
      const opened = browser ? await openBrowserPage(browser, origin, path(fresh), first.authorization) : undefined;
      const question = opened ? await browserAction(opened.page, origin, fresh.page, "MenuWrite") : await agent(operation(fresh, "MenuWrite"));
      assert.equal(question.page.interaction.state, "menu");
      assert.equal(question.page.interaction.defaultChoice, "2");
      assert.deepEqual(question.page.controls.map(item => item.caption), ["Cancel", "First", "Second", "東京"]);
      const replayQuestion = await client.read(path(fresh));
      assert.deepEqual(replayQuestion, question);
      if (adapter === "CMD") {
        const compact = await execute(process.execPath, ["build/client/cmd.mjs", "read", path(fresh)],
          { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
        assert.match(compact.stdout, /default=2 prompt="Choose explicitly <script>\."/);
        assert.ok(compact.stdout.includes('"東京"'));
      }
      if (opened) await opened.page.screenshot({ path: `${proof}/native-menu-question.png`, fullPage: true });
      const choice = index === 0 ? 0 : index === 1 ? 3 : 1;
      const answer = question.page.controls[choice].identity;
      const audit = BigInt(await sql('SELECT count(*) FROM ui_writes'));
      const result = opened ? await browserAction(opened.page, origin, question.page, answer) : await agent(operation(question, answer));
      assert.equal(field(result, "Value"), String(choice));
      assert.deepEqual(result.page.messages.map(item => item.text), [`Selected ${choice}.`]);
      assert.deepEqual(await client.read(path(fresh)), result);
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), String(choice));
      assert.equal(BigInt(await sql('SELECT count(*) FROM ui_writes')), audit + 1n);
      const replay = await post(replayQuestion, answer);
      assert.deepEqual(parsePage(replay.html), result.page);
      assert.equal(BigInt(await sql('SELECT count(*) FROM ui_writes')), audit + 1n);
      if (opened) await opened.page.screenshot({ path: `${proof}/native-menu-result.png`, fullPage: true });
    } finally { await mcp?.close(); await browser?.close(); }
  }
});

test("an unanswered native question times out without consent or implicit commit", async () => {
  const fresh = await client.read("/?page=50347&mode=Edit");
  const before = await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1');
  const question = await client.execute(path(fresh), operation(fresh, "ConfirmWrite"));
  assert.equal(question.page.interaction.state, "confirm");
  await new Promise(resolve => setTimeout(resolve, 5100));
  await assert.rejects(client.read(`/calls/${question.page.interaction.call}`), error =>
    error.code === "UiDialogCancelled" && error.outcome === "failed" &&
    error.command === operation(fresh, "ConfirmWrite").command);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), before);
  assert.equal(await sql(`SELECT answer IS NULL AND closed FROM agiru_client.page_dialogs WHERE handle='${question.page.interaction.dialog}'`), "t");
});

test("native question HTML byte refusal rolls back before publishing an unreachable dialog", async () => {
  const fresh = await client.read("/?page=50347&mode=Edit");
  const before = await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1');
  const selected = operation(fresh, "OversizedQuestion");
  await assert.rejects(client.execute(path(fresh), selected), error =>
    error.code === "PageHtmlLimit" && error.outcome === "failed" && error.command === selected.command);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), before);
  assert.equal(await sql(`SELECT count(*) FROM agiru_client.page_dialogs WHERE command_id='${selected.command}'`), "0");
});

test("linked card New runs AL initialization and saves exactly one new row across CMD MCP and htmx", async () => {
  const population = await sql('SELECT count(*) FROM "Navigation Row"');
  for (const [index, adapter] of ["CMD", "MCP", "web"].entries()) {
    const id = String(1001 + index);
    const browser = adapter === "web" ? await launchBrowser() : undefined;
    let mcp, web;
    try {
      const fresh = await client.read("/?page=50341");
      assert.equal(operation(fresh, "$agiru.new").enabled, true,
        "linked Card InsertAllowed, not the noneditable List's false property, supplies New");
      if (adapter === "web") {
        web = await openBrowserPage(browser, origin, path(fresh), first.authorization);
        await assertBrowserPage(web.page, fresh.page);
      }
      if (adapter === "MCP") {
        const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
        const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
        const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
        mcp = new Client({ name: "agiru-new-card-gate", version: "1" });
        await mcp.connect(new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"],
          env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } }));
      }
      const invoke = async (view, name, text) => {
        if (adapter === "web") return text === undefined
          ? browserAction(web.page, origin, view.page, name)
          : browserSet(web.page, origin, view.page, name, text);
        const { enabled: _enabled, ...selected } = operation(view, name);
        const input = { path: path(view), ...selected, ...(text === undefined ? {} : { text }) };
        if (adapter === "CMD") {
          const reply = await execute(process.execPath, ["build/client/cmd.mjs", "--json", "execute", JSON.stringify(input)],
            { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` } });
          assert.equal(reply.stderr, "");
          return JSON.parse(reply.stdout);
        }
        const reply = await mcp.callTool({ name: "agiru_execute", arguments: input });
        assert.notEqual(reply.isError, true, JSON.stringify(reply.structuredContent));
        return reply.structuredContent;
      };
      const create = commandEnvelope(fresh.page, operation(fresh, "$agiru.new"));
      let created = await invoke(fresh, "$agiru.new");
      assert.equal(created.page.page, "50340");
      assert.equal(created.page.handle, fresh.page.handle);
      assert.equal(field(created, "ID"), "0", "New must not select or copy an existing key");
      assert.equal(field(created, "Value"), "314", "original OnNewRecord supplies initialization");
      assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), population, "opening is not an insertion");
      const replay = await fetch(origin + create.path, { method: "POST", headers: {
        ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" },
        body: new URLSearchParams(create.fields) });
      const replayed = await finish(replay);
      assert.equal(replayed.response.status, 200);
      assert.deepEqual(parsePage(replayed.html), created.page, "New replay must not reopen AL");
      created = await invoke(created, "ID", id);
      created = await invoke(created, "Value", "62");
      created = await invoke(created, "$agiru.save");
      assert.equal(await sql(`SELECT "Value"::text || '|' || "SystemCreatedBy"::text
        FROM "Navigation Row" WHERE "ID"=${id}`), "62|00000000-0000-0000-0000-000000000001");
      assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), String(BigInt(population) + 1n));
      assert.deepEqual(await client.read(path(created)), created);
      const back = await invoke(created, "$agiru.back");
      assert.equal(back.page.page, "50341");
      assert.equal(field(back, "ID"), field(fresh, "ID"), "creation must not mutate the retained list's selected key");
      if (adapter === "web") await assertBrowserPage(web.page, back.page);
    } finally {
      await web?.page.close();
      await browser?.close();
      await mcp?.close();
      await sql(`DELETE FROM "Navigation Row" WHERE "ID"=${id}`);
    }
  }
  const blank = await client.read("/?page=50341");
  const created = await client.execute(path(blank), operation(blank, "$agiru.new"));
  await client.execute(path(created), operation(created, "$agiru.back"));
  assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), population, "unedited New must not insert a blank record");
});

test("linked card InsertAllowed false refuses New and direct Create before AL insertion", async () => {
  const population = await sql('SELECT count(*) FROM "Navigation Row"');
  const blocked = await client.read("/?page=50344");
  assert.ok(!blocked.page.controls.some(control => control.identity === "$agiru.new"));
  const forged = commandEnvelope(blocked.page, operation(blocked, "$agiru.card"));
  const body = new URLSearchParams(forged.fields);
  body.set("control", "$agiru.new");
  const denied = await fetch(origin + forged.path, { method: "POST", headers: {
    ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" }, body });
  const refusal = await finish(denied);
  assert.equal(refusal.response.status, 400);
  assert.equal(parseFailure(refusal.html).code, "PageHostUnsupported");
  await assert.rejects(client.read("/?page=50343&mode=Create"), error =>
    error.code === "PageHostUnsupported" && error.outcome === "refused");
  assert.equal(await sql('SELECT count(*) FROM "Navigation Row"'), population);
});

test("CMD MCP and Chromium explicitly select the original filtered modal and resume the same AL transaction", { timeout: 60000 }, async () => {
  for (const adapter of ["CMD", "MCP", "Web"]) {
    await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1; UPDATE "Navigation Row" SET "Value"=22 WHERE "ID"=2');
    const fresh = await client.read("/?page=50347&mode=Edit");
    const driver = await modalAdapter(adapter, fresh);
    const root = operation(fresh, "ModalPick");
    const writes = BigInt(await sql('SELECT count(*) FROM ui_writes'));
    try {
      let modal = await driver.action(fresh, "ModalPick");
      assert.equal(modal.page.profile, "4");
      assert.equal(modal.page.page, "50341");
      assert.equal(modal.page.handle, fresh.page.handle);
      assert.equal(modal.page.interaction.state, "modal");
      assert.equal(modal.page.interaction.originCommand, root.command);
      assert.equal(modal.page.window.limit, String(nativeApplication ? rowLimit : 40));
      assert.deepEqual(modal.page.rows.map(row => row.controls.find(cell => cell.identity === "ID").scalar.value), ["2"]);
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11", "modal suspension must not commit the pending write");
      assert.equal(BigInt(await sql('SELECT count(*) FROM ui_writes')), writes);
      assert.deepEqual(await client.read(path(modal)), modal);
      assert.match(renderAscii(modal.page), /modal=.*explicit close required/);
      const row = modal.page.rows[0].select.control;
      const before = await client.read(path(modal));
      modal = await driver.action(modal, row);
      assert.equal(modal.page.revision, "1");
      assert.equal(modal.page.interaction.dialog, before.page.interaction.dialog);
      const replay = await post(before, row);
      assert.equal(replay.response.status, 200);
      assert.deepEqual(parsePage(replay.html), modal.page);
      assert.equal(await sql(`SELECT revision FROM agiru_client.page_modals WHERE handle='${modal.page.interaction.dialog}'`), "1");
      await driver.screenshot("native-filtered-modal");
      const final = await driver.action(modal, "$agiru.modal_ok");
      assert.equal(final.page.page, "50347");
      assert.equal(final.page.interaction, undefined);
      assert.equal(field(final, "Value"), "22", "GetRecord must expose the selected original AL object");
      assert.deepEqual(final.page.messages.map(item => item.text), ["Selected row 2."]);
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "22");
      assert.equal(BigInt(await sql('SELECT count(*) FROM ui_writes')), writes + 2n);
      assert.equal(await sql(`SELECT active FROM agiru_client.page_modals WHERE handle='${modal.page.interaction.dialog}'`), "f");
      assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id='${root.command}'`), "complete");
      assert.deepEqual(await client.read(path(final)), final);
    } finally { await driver.close(); }
  }
});

test("CMD MCP and Chromium edit exact original modal variables without independently committing the caller", async () => {
  for (const adapter of ["CMD", "MCP", "Web"]) {
    await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
    const fresh = await client.read("/?page=50347&mode=Edit");
    const driver = await modalAdapter(adapter, fresh);
    try {
      let modal = await driver.action(fresh, "ModalNested");
      modal = await driver.action(modal, "OwnerMarker", "123");
      modal = await driver.action(modal, "ExactAmount", "1.23456789012345678901");
      assert.equal(field(modal, "OwnerMarker"), "123");
      assert.equal(field(modal, "ExactAmount"), "1.23456789012345678901");
      const before = await client.read(path(modal));
      modal = await driver.action(modal, "Notify");
      assert.deepEqual(modal.page.messages.map(message => message.text), ['Modal <script> Grün']);
      const replay = await post(before, "Notify");
      assert.equal(replay.response.status, 200);
      assert.deepEqual(parsePage(replay.html), modal.page, "modal receipts must retain exact message identities and data");
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11");
      const final = await driver.action(modal, "$agiru.modal_ok");
      assert.equal(field(final, "Value"), "123");
      assert.deepEqual(final.page.messages, modal.page.messages);
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "123");
    } finally { await driver.close(); }
  }
});

test("CMD MCP and Chromium refuse invalid typed modal input and retain the caller for explicit correction", { timeout: 60000 }, async () => {
  const diagnostics = new Map();
  const values = result => result.page.controls.filter(control => control.scalar)
    .map(control => ({ identity: control.identity, scalar: control.scalar }));
  const families = [
    [["OwnerMarker", "invalid-integer", "122"], ["OwnerMarker", "2147483648", "123"]],
    [["ExactAmount", "invalid-decimal", "1.23456789012345678901"]],
    [["ExactInteger", "9223372036854775808", "9223372036854775806"],
      ["ExactInteger", "-9223372036854775809", "-9223372036854775807"]],
    [["ArrayValue", "1.5", "17"]],
    [["Choice", "absent-member", "0"], ["Choice", "2147483648", "1"], ["Choice", "1-2", "0"]],
  ];
  for (const adapter of ["CMD", "MCP", "Web"]) {
    await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
    const fresh = await client.read("/?page=50347&mode=Edit");
    const driver = await modalAdapter(adapter, fresh);
    try {
      let current = fresh;
      for (const inputs of families) {
        const stored = field(current, "Value");
        let modal = await driver.action(current, "ModalNested");
        const writes = await sql('SELECT count(*) FROM ui_writes');
        for (const [identity, invalid, corrected] of inputs) {
          const before = await client.read(path(modal));
          assert.deepEqual(before, modal, "a fresh native read retains the adapter's exact state");
          const command = operation(before, identity).command;
          const failure = await driver.failure(before, identity, invalid);
          assert.equal(failure.outcome, "failed", `${adapter} must refuse invalid ${identity}`);
          assert.equal(failure.command, command);
          assert.ok(failure.message.length > 0);
          const diagnostic = { error: failure.error, message: failure.message, outcome: failure.outcome };
          const key = `${identity}:${invalid}`;
          if (adapter === "CMD") diagnostics.set(key, diagnostic);
          else assert.deepEqual(diagnostic, diagnostics.get(key), "all clients retain the same AL diagnostic");
          modal = await client.read(path(before));
          assert.equal(modal.page.interaction.state, "modal");
          assert.equal(modal.page.interaction.dialog, before.page.interaction.dialog);
          assert.notEqual(modal.page.revision, before.page.revision);
          assert.deepEqual(values(modal), values(before), "failed conversion must not alter any original AL variable");
          assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), stored);
          assert.equal(await sql('SELECT count(*) FROM ui_writes'), writes, "invalid input must not commit caller writes");
          assert.equal(await sql(`SELECT outcome FROM agiru_client.page_modal_commands WHERE command_id='${command}'`), "failed");
          const replay = await post(before, identity, invalid);
          assert.equal(replay.response.status, 500);
          const repeated = parseFailure(replay.html);
          assert.deepEqual({ error: repeated.code, message: repeated.message, outcome: repeated.outcome, command: repeated.command }, failure,
            "replaying the rejected command must retain its receipt, not apply another input");
          assert.deepEqual(await client.read(path(modal)), modal);
          modal = await driver.action(modal, identity, corrected);
          assert.equal(field(modal, identity), corrected);
          assert.equal(field(modal, "ValidationCount"), String(Number(field(before, "ValidationCount")) +
            (identity === "OwnerMarker" ? 1 : 0)), "valid input runs its AL validation exactly once");
        }
        await driver.screenshot(`native-invalid-variable-${inputs[0][0]}-corrected`);
        const selected = field(modal, "OwnerMarker");
        current = await driver.action(modal, "$agiru.modal_ok");
        assert.equal(field(current, "Value"), selected);
        assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), selected);
      }
    } finally { await driver.close(); }
  }
});

test("modal SQL ownership revisions CSRF replay and permissions fence every input without releasing the caller", async () => {
  await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
  const fresh = await client.read("/?page=50347&mode=Edit");
  const opened = await client.execute(path(fresh), operation(fresh, "ModalPick"));
  const choice = operation(opened, "$agiru.modal_ok");
  const envelope = commandEnvelope(opened.page, choice);
  const send = (changes = {}, headers = first, endpoint = envelope.path) => fetch(origin + endpoint, {
    method: "POST", headers: { ...headers, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams({ ...envelope.fields, ...changes }) });
  assert.equal((await send({}, second)).status, 410);
  assert.equal((await send({ csrf: "foreign" })).status, 403);
  assert.equal((await send({ revision: "999" })).status, 409);
  assert.equal((await send({}, first, "/modal-commands/not-the-modal")).status, 409);
  const parent = await post(fresh, "ModalPick", undefined, first, false);
  assert.equal(parent.response.status, 409, "the parent cannot accept input while its modal is active");
  if (nativeApplication) {
    await sql(`UPDATE "Tenant Permission" SET "Execute Permission"=0 WHERE "Role ID"='EDITOR' AND "Object Type"=8 AND "Object ID"=50341`);
    try { assert.equal((await send()).status, 403); }
    finally { await sql(`UPDATE "Tenant Permission" SET "Execute Permission"=1 WHERE "Role ID"='EDITOR' AND "Object Type"=8 AND "Object ID"=50341`); }
  }
  assert.equal(await sql(`SELECT count(*) FROM agiru_client.page_modal_commands WHERE modal_handle='${opened.page.interaction.dialog}'`), "0");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11");
  const selected = await client.execute(path(opened), operation(opened, opened.page.rows[0].select.control));
  const original = commandEnvelope(opened.page, operation(opened, opened.page.rows[0].select.control));
  const changed = await fetch(origin + original.path, { method: "POST", headers: {
    ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams({ ...original.fields, control: "$agiru.modal_cancel" }) });
  assert.equal(changed.status, 409, "an identical ID cannot be repurposed as cancellation");
  const cancelled = await post(selected, "$agiru.modal_cancel");
  assert.equal(cancelled.response.status, 500);
  assert.equal(parseFailure(cancelled.html).message, "explicit modal decline");
  assert.equal(parseFailure(cancelled.html).outcome, "failed");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11", "declined parent errors roll back earlier writes");
});

test("nested modal pages and questions preserve the original AL variables and expose only the active child", { timeout: 30000 }, async () => {
  await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
  const fresh = await client.read("/?page=50347&mode=Edit");
  const driver = await modalAdapter("Web", fresh);
  try {
    const parent = await driver.action(fresh, "ModalNested");
    assert.equal(parent.page.page, "50352");
    assert.equal(parent.page.unsupported, 1, "Action has no declared scalar transport and remains a counted gap");
    assert.equal(field(parent, "OwnerMarker"), "42");
    assert.equal(field(parent, "ExactAmount"), "1.2300");
    assert.equal(field(parent, "ExactInteger"), "9223372036854775807");
    assert.equal(field(parent, "OriginalText"), 'Grüezi <script> & "quoted"');
    assert.equal(field(parent, "ArrayValue"), "7");
    assert.deepEqual(parent.page.controls.find(control => control.identity === "Choice").scalar,
      { type: "Option", value: "1", domain: "page/50352/control/Choice", member: "After", undefined: false, closing: false });
    const child = await driver.action(parent, "PickChild");
    assert.equal(child.page.page, "50341");
    assert.notEqual(child.page.interaction.dialog, parent.page.interaction.dialog);
    const blocked = await post(parent, "$agiru.modal_ok", undefined, first, false);
    assert.equal(blocked.response.status, 409);
    let resumed = await driver.action(child, "$agiru.modal_ok");
    assert.equal(resumed.page.interaction.dialog, parent.page.interaction.dialog);
    assert.equal(field(resumed, "OwnerMarker"), "52");
    const question = await driver.action(resumed, "Ask");
    assert.equal(question.page.interaction.state, "confirm");
    assert.equal(question.page.interaction.defaultChoice, "0");
    assert.equal((await post(resumed, "$agiru.modal_cancel", undefined, first, false)).response.status, 409);
    resumed = await driver.action(question, question.page.controls[1].identity);
    assert.equal(resumed.page.interaction.dialog, parent.page.interaction.dialog);
    assert.equal(field(resumed, "OwnerMarker"), "62");
    const final = await driver.action(resumed, "$agiru.modal_ok");
    assert.equal(field(final, "Value"), "62");
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "62");
  } finally { await driver.close(); }
});

async function withDelayedModalClose(run) {
  await sql(`CREATE FUNCTION ui_close_delay() RETURNS trigger LANGUAGE plpgsql AS $body$
    BEGIN IF NEW."ID"=2 THEN PERFORM pg_sleep(0.3); END IF; RETURN NEW; END $body$;
    CREATE TRIGGER ui_close_delay BEFORE UPDATE ON "Navigation Row"
    FOR EACH ROW EXECUTE FUNCTION ui_close_delay()`);
  try { await run(); }
  finally { await sql('DROP TRIGGER ui_close_delay ON "Navigation Row"; DROP FUNCTION ui_close_delay()'); }
}

test("query-close veto and AL errors keep the modal open and require a new explicit close attempt", async () => withDelayedModalClose(async () => {
  await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
  const fresh = await client.read("/?page=50347&mode=Edit");
  let modal = await client.execute(path(fresh), operation(fresh, "ModalCloseRetry"));
  const identity = modal.page.interaction.dialog;
  for (const [attempt, message] of [[1, "the page refused to close (OnQueryClosePage)"], [2, "Fixture close error"]]) {
    const pending = await post(modal, "$agiru.modal_ok", undefined, first, false);
    assert.equal(pending.response.status, 200, "the delayed close must release the HTTP worker before AL finishes");
    const interaction = parsePage(pending.html).interaction;
    assert.equal(interaction.state, "working");
    assert.deepEqual(interaction.poll, { path: `/modal-commands/${identity}/${operation(modal, "$agiru.modal_ok").command}`, state: "pending" });
    const foreign = await fetch(origin + interaction.poll.path, { headers: second });
    assert.equal(foreign.status, 410, "a receipt address does not grant its foreign caller authority");
    const failure = await finish({ status: pending.response.status, text: async () => pending.html });
    assert.equal(failure.response.status, 500);
    const error = parseFailure(failure.html);
    assert.equal(error.message, message);
    assert.equal(error.command, operation(modal, "$agiru.modal_ok").command);
    assert.equal(error.outcome, "failed");
    const replay = await post(modal, "$agiru.modal_ok");
    assert.equal(replay.response.status, failure.response.status);
    assert.equal(replay.html, failure.html, "failed close replays its receipt without another AL attempt");
    modal = await client.read(path(modal));
    assert.equal(modal.page.interaction.dialog, identity);
    assert.equal(field(modal, "CloseAttempts"), String(attempt));
    assert.equal(field(modal, "ClosedCount"), "0");
    assert.equal(modal.page.interaction.poll.state, "failed");
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11");
  }
  const final = await client.execute(path(modal), operation(modal, "$agiru.modal_ok"));
  assert.equal(field(final, "Value"), "1");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "1");
}));

test("CMD MCP and Chromium retain delayed modal errors and allow an explicit retry without committing the caller", { timeout: 30000 }, async () => withDelayedModalClose(async () => {
  for (const adapter of ["CMD", "MCP", "Web"]) {
    await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
    const fresh = await client.read("/?page=50347&mode=Edit");
    const driver = await modalAdapter(adapter, fresh);
    try {
      let modal = await driver.action(fresh, "ModalCloseRetry");
      for (const [attempt, message] of [[1, "the page refused to close (OnQueryClosePage)"], [2, "Fixture close error"]]) {
        const command = operation(modal, "$agiru.modal_ok").command;
        const failure = await driver.failure(modal, "$agiru.modal_ok");
        assert.equal(failure.message, message);
        assert.equal(failure.command, command);
        assert.equal(failure.outcome, "failed");
        modal = await client.read(path(modal));
        assert.equal(field(modal, "CloseAttempts"), String(attempt));
        assert.equal(field(modal, "ClosedCount"), "0");
        assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11");
      }
      const final = await driver.action(modal, "$agiru.modal_ok");
      assert.equal(field(final, "Value"), "1");
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "1");
    } finally { await driver.close(); }
  }
}));

test("a later caller error rolls back modal writes but never undoes an earlier explicit Commit", async () => {
  for (const [action, stored, message] of [["ModalFail", "11", "failure after modal"],
    ["ModalCommitFail", "21", "failure after committed modal"]]) {
    await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
    const fresh = await client.read("/?page=50347&mode=Edit");
    const root = operation(fresh, action);
    const modal = await client.execute(path(fresh), root);
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), stored);
    await assert.rejects(client.execute(path(modal), operation(modal, "$agiru.modal_ok")), error =>
      error.message === message && error.command === root.command && error.outcome === "failed");
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), stored);
    assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands WHERE command_id='${root.command}'`), "failed");
    assert.equal(await sql(`SELECT active FROM agiru_client.page_modals WHERE handle='${modal.page.interaction.dialog}'`), "f");
  }
});

test("an unanswered native modal times out without choosing a row or committing caller writes", { timeout: 15000 }, async () => {
  await sql('UPDATE "Navigation Row" SET "Value"=11 WHERE "ID"=1');
  const fresh = await client.read("/?page=50347&mode=Edit");
  const modal = await client.execute(path(fresh), operation(fresh, "ModalPick"));
  const deadline = Date.now() + 10000;
  let response;
  do {
    await new Promise(resolve => setTimeout(resolve, 100));
    response = await fetch(origin + `/calls/${modal.page.interaction.call}`, { headers: first });
    if (response.status !== 200) break;
  } while (Date.now() < deadline);
  assert.equal(response.status, 500);
  assert.equal(parseFailure(await response.text()).code, "UiDialogCancelled");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "11");
});
