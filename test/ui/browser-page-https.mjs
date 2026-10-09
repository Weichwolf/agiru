import assert from "node:assert/strict";
import { after, test } from "node:test";
import { execFile, spawn } from "node:child_process";
import { readFile, writeFile, unlink } from "node:fs/promises";
import { createRequire } from "node:module";
import { promisify } from "node:util";
import { AgentClient } from "../../build/client/http.mjs";
import { parsePage, commandEnvelope } from "../../build/client/profile.mjs";
import { ServerConfigs } from "./server-config.mjs";
import { assertBrowserPage, browserAction, browserSet } from "./browser-client.mjs";

const execute = promisify(execFile);
const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
const { chromium } = require("playwright-core");
const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
const container = process.env.AGIRU_BROWSER_TLS_CONTAINER;
const proof = process.env.AGIRU_BROWSER_TLS_PROOF;
assert.match(container, /^agiru-browser-https-[0-9]+$/);
assert.match(proof, /^\/tmp\/agiru-browser-https\.[A-Za-z0-9]+$/);
const address = (await readFile(`${proof}/port`, "utf8")).trim();
assert.match(address, /^127\.0\.0\.1:[0-9]+$/);
const origin = `https://localhost:${address.split(":")[1]}`;
const native = "/run/agiru/browser-pages";
const configs = new ServerConfigs(container, native, proof);
const seed = spawn("podman", ["exec", "--interactive", "--user", "agiru", container,
  `${native}/host`, `${native}/auth.json`, origin, "--seed-only"], { stdio: ["pipe", "pipe", "pipe"] });
let seedOutput = "", diagnostics = "", service, servicePid, serviceClosed, browser, database;
const seedClosed = new Promise(resolve => seed.once("close", resolve));
seed.stdout.on("data", chunk => { seedOutput += chunk; });
seed.stderr.on("data", chunk => { diagnostics += chunk; });
await new Promise((resolve, reject) => {
  const timer = setTimeout(() => reject(new Error("Page seed did not start")), 15000);
  seed.once("error", reject);
  seed.once("close", code => { clearTimeout(timer); reject(new Error(`Page seed exited ${code}`)); });
  seed.stdout.on("data", () => { if (seedOutput.includes("READY\n")) { clearTimeout(timer); resolve(); } });
});
database = seedOutput.match(/^DATABASE (agiru_owned_gate_[0-9]+_page_host)$/m)?.[1];
assert.ok(database);
const auth = JSON.parse((await execute("podman", ["exec", "--user", "agiru", container,
  "sed", "-n", "1p", `${native}/auth.json`])).stdout);
assert.match(auth.authorization, /^Bearer ag1_[a-f0-9]{64}$/);
await writeFile(`${proof}/auth.json`, JSON.stringify(auth), { flag: "wx", mode: 0o600 });
after(async () => {
  await browser?.close();
  if (servicePid) {
    await execute("podman", ["exec", "--user", "agiru", container, "kill", "-TERM", servicePid]);
    assert.equal(await serviceClosed, 0, "native service drains before removing the seed");
  }
  await configs.clean();
  seed.stdin.end("Q");
  const timer = setTimeout(() => seed.kill(), 15000);
  try {
    assert.equal(await seedClosed, 0, "seed removes its owned database");
    assert.ok(!diagnostics.includes("cleanup failed"));
    for (const suffix of ["", ".second", ".peer"]) {
      await execute("podman", ["exec", "--user", "agiru", container, "unlink", `${native}/auth.json${suffix}`]);
    }
    await unlink(`${proof}/auth.json`);
  } finally { clearTimeout(timer); }
});
const config = await configs.write({ database: `postgresql://agiru:agiru@127.0.0.1:5432/${database}`,
  company: "Fixture + Company", origin, browser_sessions: { enabled: true }, http: { workers: 2 },
  pages: { dialog_timeout_seconds: 30, response_wait_ms: 1 } });
service = spawn("podman", ["exec", "--user", "agiru", container, `${native}/agiru`, "serve", "--config", config],
  { stdio: ["ignore", "pipe", "pipe"] });
serviceClosed = new Promise(resolve => service.once("close", resolve));
await new Promise((resolve, reject) => {
  let output = "";
  const timer = setTimeout(() => reject(new Error("Native page service did not start")), 15000);
  service.once("error", reject);
  service.once("close", code => { clearTimeout(timer); reject(new Error(`Native page service exited ${code}`)); });
  service.stderr.on("data", chunk => { diagnostics += chunk; });
  service.stdout.on("data", chunk => {
    output += chunk;
    const ready = output.match(/^READY ([1-9][0-9]*) 18080$/m);
    if (ready) { servicePid = ready[1]; clearTimeout(timer); resolve(); }
  });
});
const agent = new AgentClient(origin, auth);
const clientEnv = { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json` };
async function sql(statement) {
  return (await execute("podman", ["exec", "--user", "postgres", container, "psql", "-XAt",
    "-v", "ON_ERROR_STOP=1", database, "-c", statement])).stdout.trim();
}
async function launch() {
  return chromium.launch({ executablePath: `${process.cwd()}/test/ui/trusted-chromium.sh`,
    headless: true, args: ["--no-sandbox"], env: { ...process.env, AGIRU_BROWSER_TRUST_PROFILE: `${proof}/browser-home` } });
}
const path = page => `/?handle=${page.handle}`;
const field = (page, name) => page.controls.find(control => control.identity === name)?.scalar?.value;
function operation(page, name) {
  const { enabled: _enabled, ...value } = page.controls.find(control => control.identity === name).operation;
  return value;
}
const responses = new WeakMap();
function track(page) {
  page.on("response", response => {
    if (response.status() === 200 && response.request().headers()["hx-request"] === "true") responses.set(page, response);
  });
}
async function model(page) {
  await page.waitForFunction(() => /^(Ready|[0-9]+ unsupported|Explicit answer required)/
    .test(document.querySelector("#status").textContent) && !document.querySelector(".htmx-request"));
  return parsePage(await responses.get(page).text());
}
async function open(page, target, credential = "") {
  await page.goto(`${origin}/assets/index.html`);
  await page.waitForFunction(() => !document.querySelector("#credential").disabled);
  await page.locator("#target").fill(target);
  if (credential) await page.locator("#credential").fill(credential);
  await page.locator("#connection button").click();
  await page.locator("#workspace article").waitFor();
  await page.waitForFunction(() => !document.querySelector(".htmx-request"));
  return model(page);
}
function semantics(page) {
  const control = value => ({ identity: value.identity, kind: value.kind, caption: value.caption,
    ...(value.scalar ? { scalar: value.scalar } : {}), ...(value.operation ? { enabled: value.operation.enabled } : {}) });
  const controls = page.controls.map(value => {
    const row = page.rows?.findIndex(row => row.select.control === value.identity) ?? -1;
    return control(row < 0 ? value : { ...value, identity: `$agiru.row-index.${row}` });
  });
  return { page: page.page, caption: page.caption, controls,
    unsupported: page.unsupported, ...(page.rows ? { rows: page.rows.map(row => ({ caption: row.caption,
      selected: row.selected, controls: row.controls.map(control) })), window: page.window } : {}) };
}
let context, page, current;
const requests = [];

test("Chromium rejects the same Caddy certificate before private CA installation", async () => {
  const untrusted = await launch();
  try {
    const tab = await untrusted.newPage();
    await assert.rejects(tab.goto(`${origin}/assets/index.html`), /ERR_CERT_AUTHORITY_INVALID/);
  } finally { await untrusted.close(); }
  await execute("certutil", ["-A", "-d", `sql:${proof}/browser-home/.pki/nssdb`, "-n", "agiru-test-ca",
    "-t", "C,,", "-i", `${proof}/ca.crt`]);
  browser = await launch();
  context = await browser.newContext();
  page = await context.newPage();
  track(page);
  page.setDefaultTimeout(15000);
  page.on("request", request => { if (!request.url().includes("/assets/")) requests.push(request); });
});

test("real HTTPS shell bootstrap executes no AL page before explicit Open", async () => {
  await page.goto(`${origin}/assets/index.html`);
  await page.waitForFunction(() => document.querySelector("#status").textContent === "Development credential required.");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_contexts"), "0");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.browser_sessions"), "0");
  assert.equal(await sql("SELECT count(*) FROM ui_writes"), "0");
});

test("actual htmx exchanges once and sends protected cookies plus CSRF without retained bearer", async () => {
  current = await open(page, "/?page=50341", auth.authorization.slice(7));
  await assertBrowserPage(page, current);
  const cookies = await context.cookies(origin);
  assert.equal(cookies.length, 1);
  assert.equal(cookies[0].name, "__Host-agiru");
  assert.equal(cookies[0].httpOnly, true); assert.equal(cookies[0].secure, true);
  assert.equal(cookies[0].sameSite, "Strict"); assert.equal(cookies[0].path, "/");
  assert.deepEqual(await page.evaluate(() => ({ cookie: document.cookie, secret: document.querySelector("#credential").value,
    persisted: [...Object.values(localStorage), ...Object.values(sessionStorage)].some(value => /ag[b1]|ag1_/.test(value)) })),
    { cookie: "", secret: "", persisted: false });
  const protectedRequests = requests.filter(request => request.headers()["hx-request"] === "true");
  assert.ok(protectedRequests.length > 0);
  for (const request of protectedRequests) {
    assert.equal(request.headers().authorization, undefined);
    assert.equal(request.headers()["x-agiru-client"], "browser");
    assert.match(request.headers()["x-agiru-csrf"], /^[a-f0-9]{64}$/);
  }
  assert.equal(requests.filter(request => request.headers().authorization).length, 1);
  assert.equal(await sql("SELECT count(*) FROM agiru_client.browser_sessions"), "1");
  await page.screenshot({ path: `${proof}/https-cookie-list.png`, fullPage: true });
});

test("external CMD MCP and cookie htmx list retain identical exact values with independent handles", async () => {
  const cmd = JSON.parse((await execute(process.execPath, ["build/client/cmd.mjs", "--json", "read", "/?page=50341"],
    { env: clientEnv })).stdout);
  assert.deepEqual(semantics(cmd.page), semantics(current));
  const mcp = new Client({ name: "agiru-cookie-parity", version: "1" });
  try {
    await mcp.connect(new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"], env: clientEnv }));
    const reply = await mcp.callTool({ name: "agiru_read", arguments: { path: "/?page=50341" } });
    assert.notEqual(reply.isError, true);
    assert.deepEqual(semantics(reply.structuredContent.page), semantics(current));
  } finally { await mcp.close(); }
  assert.notEqual(cmd.page.handle, current.handle);
  await assert.rejects(agent.read(path(current)), /expired|refused|missing|HTTP/i);
  assert.equal(await sql("SELECT count(*) FROM ui_writes"), "0");
});

test("cookie-backed Validate Save and identical replay have independently verified SQL effects", async () => {
  let next = await browserAction(page, origin, current, "$agiru.card");
  next = await browserSet(page, origin, next.page, "Value", "731");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "731");
  const envelope = commandEnvelope(next.page, operation(next.page, "$agiru.save"));
  next = await browserAction(page, origin, next.page, "$agiru.save");
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "731");
  const before = await sql("SELECT count(*) FROM ui_writes");
  const activity = await sql("SELECT last_activity_at::text FROM agiru_client.browser_sessions");
  const headers = requests.find(request => request.headers()["x-agiru-csrf"])?.headers();
  let replay = await page.evaluate(async ({ envelope, csrf }) => {
    const reply = await fetch(envelope.path, { method: "POST", headers: { "X-Agiru-Client": "browser",
      "X-Agiru-CSRF": csrf, "Content-Type": "application/x-www-form-urlencoded" }, body: new URLSearchParams(envelope.fields) });
    return { status: reply.status, html: await reply.text() };
  }, { envelope, csrf: headers["x-agiru-csrf"] });
  const deadline = Date.now() + 15000;
  while (replay.status === 200 && parsePage(replay.html).interaction?.state === "working") {
    assert.ok(Date.now() < deadline, "replayed receipt finishes using GET only, never another POST");
    const call = parsePage(replay.html).interaction.call;
    replay = await page.evaluate(async ({ call, csrf }) => {
      const reply = await fetch(`/calls/${call}`, { headers: { "X-Agiru-Client": "browser", "X-Agiru-CSRF": csrf } });
      return { status: reply.status, html: await reply.text() };
    }, { call, csrf: headers["x-agiru-csrf"] });
  }
  assert.equal(replay.status, 200);
  assert.deepEqual(parsePage(replay.html), next.page);
  assert.equal(await sql("SELECT count(*) FROM ui_writes"), before);
  assert.equal(await sql("SELECT last_activity_at::text FROM agiru_client.browser_sessions"), activity);
  current = next.page;
});

test("a second tab passively shares authentication but opens independent AL state", async () => {
  const before = await sql("SELECT last_activity_at::text FROM agiru_client.browser_sessions");
  const tab = await context.newPage();
  track(tab);
  try {
    await tab.goto(`${origin}/assets/index.html`);
    await tab.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Browser session ready"));
    assert.equal(await sql("SELECT last_activity_at::text FROM agiru_client.browser_sessions"), before);
    await tab.locator("#target").fill("/?page=50340&mode=Edit");
    await tab.locator("#connection button").click();
    await tab.locator("#workspace article").waitFor();
    const independent = await model(tab);
    assert.notEqual(independent.handle, current.handle);
    await browserSet(tab, origin, independent, "LoadedValue", "999");
    assert.equal(field(await model(page), "Value"), "731");
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "731");
    await page.locator("#logout").click();
    await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Signed out"));
    assert.deepEqual(await context.cookies(origin), []);
    await tab.locator('[data-control="$agiru.save"] button').click();
    await tab.waitForFunction(() => !document.querySelector("#workspace article"));
    assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), "731");
    assert.equal(await sql("SELECT count(*) FROM agiru_client.browser_sessions WHERE revoked_at IS NULL"), "0");
  } finally { await tab.close(); }
});

test("malformed HTTPS session grants never fall back to bearer ERP requests", async () => {
  const isolated = await browser.newContext();
  const tab = await isolated.newPage();
  const sent = [];
  tab.on("request", request => sent.push(request));
  const before = await sql("SELECT count(*) FROM agiru_client.page_contexts");
  try {
    await tab.goto(`${origin}/assets/index.html`);
    await tab.waitForFunction(() => document.querySelector("#status").textContent === "Development credential required.");
    await tab.route(`${origin}/session`, route => {
      assert.equal(route.request().method(), "POST");
      return route.fulfill({ status: 200, headers: { "Content-Type": "application/json; charset=utf-8",
        "Cache-Control": "no-store" }, body: '{"csrf":"invalid"}' });
    });
    await tab.locator("#target").fill("/?page=50340&mode=Edit");
    await tab.locator("#credential").fill(auth.authorization.slice(7));
    await tab.locator("#connection button").click();
    await tab.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Connection refused"));
    assert.equal(sent.filter(request => request.headers().authorization).length, 1);
    assert.equal(sent.filter(request => request.headers()["hx-request"]).length, 0);
    assert.equal(await tab.locator("#credential").inputValue(), "");
    assert.deepEqual(await isolated.cookies(origin), []);
    assert.equal(await sql("SELECT count(*) FROM agiru_client.page_contexts"), before);
  } finally { await isolated.close(); }
});

async function cancelled(interaction, pageHandle) {
  const deadline = Date.now() + 5000;
  while (await sql(`SELECT outcome FROM agiru_client.page_commands WHERE handle='${pageHandle}' AND command_id='${interaction.originCommand}'`) !== "failed") {
    assert.ok(Date.now() < deadline, "active cancellation must finish before the thirty-second dialog timeout");
    await new Promise(resolve => setTimeout(resolve, 50));
  }
  assert.equal(await sql(`SELECT invalidated FROM agiru_client.page_contexts WHERE handle='${pageHandle}'`), "t");
  if (interaction.state === "modal") {
    assert.equal(await sql(`SELECT active FROM agiru_client.page_modals WHERE handle='${interaction.dialog}'`), "f");
  } else {
    assert.equal(await sql(`SELECT closed::text||':'||(answer IS NULL)::text FROM agiru_client.page_dialogs WHERE handle='${interaction.dialog}'`), "true:true");
  }
}

for (const [action, state, committed] of [["ConfirmWrite", "confirm", false],
  ["CommittedConfirm", "confirm", true], ["ModalNested", "modal", false]]) {
  test(`HTTPS browser logout cancels suspended ${action} and preserves only earlier commits`, async () => {
    const isolated = await browser.newContext();
    const tab = await isolated.newPage();
    track(tab);
    try {
      const fresh = await open(tab, "/?page=50347&mode=Edit", auth.authorization.slice(7));
      const before = BigInt(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'));
      const writes = BigInt(await sql("SELECT count(*) FROM ui_writes"));
      const question = await browserAction(tab, origin, fresh, action);
      assert.equal(question.page.interaction.state, state);
      const expected = String(before + (committed ? 10n : 0n));
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), expected);
      await tab.locator("#logout").click();
      await tab.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Signed out"));
      await cancelled(question.page.interaction, fresh.handle);
      assert.deepEqual(await isolated.cookies(origin), []);
      assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), expected);
      assert.equal(await sql("SELECT count(*) FROM ui_writes"), String(writes + (committed ? 1n : 0n)));
      const peer = await agent.read("/?page=50347&mode=Edit");
      assert.equal(field(peer.page, "Value"), expected, "same-user agent authority is not revoked by browser logout");
    } finally { await isolated.close(); }
  });
}

test("expiry at an unanswered AL question retains the original command without consent or retry", async () => {
  current = await open(page, "/?page=50347&mode=Edit", auth.authorization.slice(7));
  const before = await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1');
  const writes = await sql("SELECT count(*) FROM ui_writes");
  const question = await browserAction(page, origin, current, "MenuWrite");
  assert.equal(question.page.interaction.state, "menu");
  const original = question.page.interaction.originCommand;
  const choice = question.page.controls[1];
  assert.notEqual(choice.operation.command, original);
  assert.equal(await sql(`SELECT closed FROM agiru_client.page_dialogs WHERE handle='${question.page.interaction.dialog}'`), "f");
  await sql("UPDATE agiru_client.browser_sessions SET idle_expires_at=clock_timestamp()-interval '1 second' WHERE revoked_at IS NULL");
  const count = requests.length;
  const refused = page.waitForResponse(response => response.url() === `${origin}/answers`);
  const escaped = await page.evaluate(value => CSS.escape(value), choice.identity);
  await page.locator(`[data-control="${escaped}"] button`).click();
  assert.equal((await refused).status(), 401);
  await page.waitForFunction(command => document.querySelector("#status").textContent.includes(`reconcile command ${command}`), original);
  assert.equal(await page.locator("#workspace article").count(), 0);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), before);
  await cancelled(question.page.interaction, current.handle);
  assert.equal(await sql("SELECT count(*) FROM ui_writes"), writes);
  assert.equal(await sql('SELECT "Value" FROM "Navigation Row" WHERE "ID"=1'), before);
  assert.equal(requests.slice(count).filter(request => request.method() === "POST").length, 1);
});
