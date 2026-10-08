import assert from "node:assert/strict";
import { test, beforeEach, afterEach, after } from "node:test";
import { createServer } from "node:http";
import { readFile } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient } from "../../build/client/http.mjs";
import { parsePage, commandEnvelope } from "../../build/client/profile.mjs";
import { assertBrowserPage } from "./browser-client.mjs";
import { questionHtml } from "./dialog-fixture.mjs";

const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
const { chromium } = require("playwright-core");
const original = await readFile(process.env.AGIRU_CLIENT_HTML, "utf8");
const bearer = `Bearer ag1_${"a".repeat(64)}`;
const requests = [];
let responseHtml = original;
let responseHeaders = {};
let responseStatus = 200;
let responseDelay = 0;
let workingPolls = -1, workingCommand = "", workingFailure = false;
const workingHtml = () => `<article data-agiru-profile="3" data-view="interaction" data-page="50400" data-handle="page_1" data-revision="9007199254740993" data-state="working" data-call="call_web" data-origin-command="${workingCommand}"><h1>Working</h1><output data-unsupported-count="0"></output></article>`;
const assets = new Map(await Promise.all(["index.html", "web.js", "web.css"].map(async name =>
  [name, await readFile(new URL(`../../build/web/${name}`, import.meta.url))])));
if (process.env.AGIRU_WEB_SCRIPT) assets.set("web.js", await readFile(process.env.AGIRU_WEB_SCRIPT));
const server = createServer(async (request, response) => {
  if (request.url.startsWith("/assets/")) {
    const name = request.url.slice(8);
    const value = assets.get(name);
    response.writeHead(value ? 200 : 404, { "Content-Type": name.endsWith(".js") ? "text/javascript" :
      name.endsWith(".css") ? "text/css" : "text/html" });
    response.end(value);
    return;
  }
  if (request.url === "/favicon.ico") { response.writeHead(404); response.end(); return; }
  let body = "";
  for await (const chunk of request) body += chunk;
  requests.push({ method: request.method, path: request.url, headers: request.headers,
    fields: Object.fromEntries(new URLSearchParams(body)) });
  if (request.headers.authorization !== bearer) { response.writeHead(401); response.end(); return; }
  if (workingPolls >= 0 && request.method === "POST") {
    workingCommand = requests.at(-1).fields.command;
    response.writeHead(200, { "Content-Type": "text/html; charset=utf-8" }); response.end(workingHtml()); return;
  }
  if (request.url === "/calls/call_web") {
    response.writeHead(workingFailure ? 500 : 200, { "Content-Type": "text/html; charset=utf-8" });
    response.end(workingFailure ? `<article data-agiru-error="1" data-code="AlError" data-command="${workingCommand}" data-outcome="failed"><h1>Request failed</h1><p>Delayed failure</p></article>` :
      workingPolls-- > 0 ? workingHtml() : original); return;
  }
  if (responseDelay) await new Promise(resolve => setTimeout(resolve, responseDelay));
  response.writeHead(responseStatus, { "Content-Type": "text/html; charset=utf-8", ...responseHeaders });
  response.end(responseHtml);
});
await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
const origin = `http://127.0.0.1:${server.address().port}`;
const browser = await chromium.launch({ executablePath: process.env.AGIRU_CHROMIUM ?? "/usr/bin/chromium",
  headless: true, args: ["--no-sandbox"] });
beforeEach(() => { responseHtml = original; responseHeaders = {}; responseStatus = 200; responseDelay = 0;
  workingPolls = -1; workingCommand = ""; workingFailure = false; });
afterEach(async () => { for (const context of browser.contexts()) await context.close(); });
after(async () => {
  await browser.close();
  server.closeAllConnections();
  await new Promise(resolve => server.close(resolve));
});

async function open(target = "/?page=50400") {
  const page = await browser.newPage();
  page.setDefaultTimeout(10000);
  await page.goto(`${origin}/assets/index.html`);
  await page.locator("#target").fill(target);
  await page.locator("#credential").fill(bearer.slice(7));
  await page.locator("#connection button").click();
  return page;
}
async function ready(page) {
  await page.locator("#workspace article").waitFor();
  await page.waitForFunction(() => !document.querySelector(".htmx-request"));
}
const control = (page, name) => page.locator(`section[data-control="${name}"]`);

test("htmx follows a working call with GET only and preserves the native page contract", async () => {
  const page = await open();
  await ready(page);
  workingPolls = 2;
  const before = requests.length;
  await control(page, "Post").locator("button").click();
  await page.waitForFunction(() => document.querySelector("#workspace article")?.dataset.agiruProfile === "3");
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("2 unsupported"));
  const calls = requests.slice(before);
  assert.deepEqual(calls.map(item => [item.method, item.path]),
    [["POST", "/commands"], ["GET", "/calls/call_web"], ["GET", "/calls/call_web"], ["GET", "/calls/call_web"]]);
  assert.equal(workingCommand, parsePage(original).controls.find(item => item.identity === "Post").operation.command);
  await assertBrowserPage(page, parsePage(original));
  assert.match(page.url(), /\?handle=page_1$/);
});

test("htmx renders explicit questions and messages without automatic defaults or polling", async () => {
  responseHtml = questionHtml();
  const model = parsePage(responseHtml);
  const page = await open("/?handle=page_1");
  await ready(page);
  await page.waitForFunction(() => document.querySelector("#status").textContent === "Explicit answer required.");
  await assertBrowserPage(page, model);
  const before = requests.length;
  await page.waitForTimeout(200);
  assert.equal(requests.length, before, "an unanswered question must not generate a request");
  assert.equal(await page.locator("#workspace script").count(), 0);
  responseHtml = original;
  const answer = model.controls[0];
  await control(page, answer.identity).locator("button").click();
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("2 unsupported"));
  assert.equal(requests.length, before + 1);
  assert.equal(requests.at(-1).path, "/answers");
  assert.deepEqual(requests.at(-1).fields, { ...commandEnvelope(model, answer.operation).fields });
});

test("htmx retains the original command when an asynchronous native action fails", async () => {
  const page = await open();
  await ready(page);
  workingPolls = 1; workingFailure = true;
  const before = requests.length;
  await control(page, "Post").locator("button").click();
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Server error AlError"));
  assert.equal(await page.locator("#status").textContent(),
    `Server error AlError: Delayed failure outcome=failed command=${workingCommand}; prior explicit commits may persist.`);
  assert.deepEqual(requests.slice(before).map(item => item.method), ["POST", "GET"]);
  await assertBrowserPage(page, parsePage(original));
});

test("actual browser renders original native HTML with exact values, counted gaps and no persisted token", async () => {
  const page = await open();
  await ready(page);
  const agent = await new AgentClient(origin, { authorization: bearer }).read("/?page=50400");
  await assertBrowserPage(page, agent.page);
  assert.equal(await page.locator("article").getAttribute("data-revision"), agent.page.revision);
  assert.equal(await control(page, "Amount").locator("input[name=text]").getAttribute("data-value"), "1.2300");
  assert.equal(await control(page, "Big").locator("output").getAttribute("data-value"), "9223372036854775807");
  assert.equal(await page.locator("article h1").textContent(), "HTML <fixture>");
  assert.equal(await page.locator("aside[role=alert]").count(), agent.page.unsupported);
  assert.equal(await control(page, "Disabled").locator("button").isDisabled(), true);
  assert.match(page.url(), /\?handle=page_1$/);
  assert.deepEqual(await page.evaluate(() => ({ local: localStorage.length, session: Object.keys(sessionStorage),
    credential: document.querySelector("#credential").value })),
    { local: 0, session: ["htmx-current-path-for-history"], credential: "" });
  assert.equal(await page.evaluate(() => Object.entries(sessionStorage).some(([key, value]) =>
    key.includes("ag1_") || value.includes("ag1_"))), false);
  await page.screenshot({ path: `${process.env.AGIRU_WEB_PROOF}/native-profile.png`, fullPage: true });
  await page.close();
});

test("real htmx field/action POST preserves the exact agent command envelope without rounding", async () => {
  const page = await open();
  await ready(page);
  const model = parsePage(original);
  const amount = model.controls.find(value => value.identity === "Amount").operation;
  const text = "0.1234567890123456789012345678";
  const before = requests.length;
  await control(page, "Amount").locator("input[name=text]").fill(text);
  await control(page, "Amount").locator("button").click();
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("2 unsupported"));
  assert.equal(requests.length, before + 1);
  const sent = requests.at(-1);
  assert.equal(sent.method, "POST");
  assert.deepEqual(sent.fields, { ...commandEnvelope(model, amount, text).fields });
  assert.equal(sent.headers.origin, origin);
  assert.equal(sent.headers["hx-request"], "true");
  const post = model.controls.find(value => value.identity === "Post").operation;
  await control(page, "Post").locator("button").click();
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("2 unsupported"));
  assert.deepEqual(requests.at(-1).fields, { ...commandEnvelope(model, post).fields });
  await page.close();
});

test("htmx presents qualified command errors as untrusted text and never replaces the retained page or retries", async () => {
  const page = await open();
  await ready(page);
  const command = parsePage(original).controls.find(control => control.identity === "Post").operation.command;
  const before = requests.length;
  responseStatus = 500;
  responseHtml = `<article data-agiru-error="1" data-code="UiWriteTransaction" data-command="${command}" data-outcome="failed"><h1>Request failed</h1><p>Grüezi &lt;script&gt; 東京 &amp; blocked.</p></article>`;
  await control(page, "Post").locator("button").click();
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("Server error"));
  assert.equal(await page.locator("#status").textContent(),
    `Server error UiWriteTransaction: Grüezi <script> 東京 & blocked. outcome=failed command=${command}; prior explicit commits may persist.`);
  assert.equal(requests.length, before + 1);
  await assertBrowserPage(page, parsePage(original));
  assert.equal(await page.locator("#status script").count(), 0);
  assert.equal(await page.evaluate(() => window.injected), undefined);
});

for (const [name, mutate] of [
  ["script", html => html.replace("</h1>", "<script>window.injected=true</script></h1>")],
  ["handler", html => html.replace('<button type="submit">Set', '<button type="submit" onclick="window.injected=true">Set')],
  ["foreign command", html => html.replaceAll('/commands', '//foreign.invalid/commands')],
  ["hidden omitted gap", html => html.replace('data-unsupported-count="2"', 'data-unsupported-count="0"')],
]) test(`browser refuses untrusted native fragment before htmx execution: ${name}`, async () => {
  responseHtml = mutate(original);
  const page = await open();
  await page.waitForFunction(() => document.querySelector("#status").textContent.includes("refused"));
  assert.equal(await page.locator("article").count(), 0);
  assert.equal(await page.evaluate(() => window.injected), undefined);
  await page.close();
  responseHtml = original;
});

test("undeclared htmx response effects refuse before redirect/trigger processing", async () => {
  responseHeaders = { "HX-Redirect": "https://foreign.invalid/" };
  const page = await open();
  await page.waitForFunction(() => document.querySelector("#status").textContent.includes("refused"));
  assert.ok(page.url().startsWith(origin));
  assert.equal(await page.locator("article").count(), 0);
  await page.close();
  responseHeaders = {};
});

test("altered hidden envelope is refused locally without an HTTP write", async () => {
  const page = await open();
  await ready(page);
  const before = requests.length;
  await control(page, "Amount").locator('input[name="csrf"]').evaluate(input => { input.value = "tampered"; });
  await control(page, "Amount").locator("button").click();
  assert.match(await page.locator("#status").textContent(), /Command refused/);
  assert.equal(requests.length, before);
  await page.close();
});

test("uncertain writes keep the old page and command identity; never retry automatically", async () => {
  const page = await open();
  await ready(page);
  const before = requests.length;
  responseStatus = 500;
  await control(page, "Post").locator("button").click();
  await page.waitForFunction(() => document.querySelector("#status").textContent.includes("uncertain"));
  assert.match(await page.locator("#status").textContent(), /cmd_1_7/);
  assert.equal(await page.locator("article").count(), 1);
  assert.equal(requests.length, before + 1);
  await page.close();
  responseStatus = 200;
});

test("shared admission prevents two simultaneous forms from writing against the same revision", async () => {
  const page = await open();
  await ready(page);
  const before = requests.length;
  responseDelay = 200;
  await page.evaluate(() => {
    document.querySelector('section[data-control="Post"] button').click();
    document.querySelector('section[data-control="Amount"] button').click();
  });
  await page.waitForFunction(() => document.querySelector("#status").textContent.startsWith("2 unsupported"));
  assert.equal(requests.length, before + 1);
  await page.close();
  responseDelay = 0;
});
