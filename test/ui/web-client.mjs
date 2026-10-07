import assert from "node:assert/strict";
import { test, beforeEach, afterEach, after } from "node:test";
import { createServer } from "node:http";
import { readFile } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient } from "../../build/client/http.mjs";
import { parsePage, commandEnvelope } from "../../build/client/profile.mjs";

const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
const { chromium } = require("playwright-core");
const original = await readFile(process.env.AGIRU_CLIENT_HTML, "utf8");
const bearer = `Bearer ag1_${"a".repeat(64)}`;
const requests = [];
let responseHtml = original;
let responseHeaders = {};
let responseStatus = 200;
let responseDelay = 0;
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
  if (responseDelay) await new Promise(resolve => setTimeout(resolve, responseDelay));
  response.writeHead(responseStatus, { "Content-Type": "text/html; charset=utf-8", ...responseHeaders });
  response.end(responseHtml);
});
await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
const origin = `http://127.0.0.1:${server.address().port}`;
const browser = await chromium.launch({ executablePath: process.env.AGIRU_CHROMIUM ?? "/usr/bin/chromium",
  headless: true, args: ["--no-sandbox"] });
beforeEach(() => { responseHtml = original; responseHeaders = {}; responseStatus = 200; responseDelay = 0; });
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

test("actual browser renders original native HTML with exact values, counted gaps and no persisted token", async () => {
  const page = await open();
  await ready(page);
  const agent = await new AgentClient(origin, { authorization: bearer }).read("/?page=50400");
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
