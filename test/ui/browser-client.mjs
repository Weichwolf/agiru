import assert from "node:assert/strict";
import { createRequire } from "node:module";
import { parsePage } from "../../build/client/profile.mjs";

const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
const { chromium } = require("playwright-core");

export function launchBrowser() {
  return chromium.launch({ executablePath: process.env.AGIRU_CHROMIUM ?? "/usr/bin/chromium",
    headless: true, args: ["--no-sandbox"] });
}

async function settled(page) {
  await page.waitForFunction(() => /^(Ready|Explicit answer required|[0-9]+ unsupported|Page request refused|Write outcome uncertain|Server error)/
    .test(document.querySelector("#status").textContent));
  await page.waitForFunction(() => !document.querySelector(".htmx-request"));
}

export async function openBrowserPage(browser, origin, target, authorization) {
  assert.match(authorization, /^Bearer ag1_[a-f0-9]{64}$/);
  const page = await browser.newPage();
  page.setDefaultTimeout(15000);
  await page.goto(`${origin}/assets/index.html`);
  await page.locator("#target").fill(target);
  await page.locator("#credential").fill(authorization.slice(7));
  const received = page.waitForResponse(response => response.url() === new URL(target, origin).href &&
    response.request().headers()["hx-request"] === "true");
  await page.locator("#connection button").click();
  const response = await received;
  await settled(page);
  return { page, response };
}

export async function assertBrowserPage(page, model) {
  const rendered = await page.evaluate(() => {
    const article = document.querySelector("#workspace article");
    if (!article) return null;
    const readControl = node => {
      const identity = node.dataset.control;
      const kind = node.dataset.kind ?? (node.tagName === "ASIDE" ? "unsupported" : "label");
      const value = node.querySelector(":scope > form > input[name=text], :scope > output[data-type]");
      const heading = node.querySelector(":scope > h2, :scope > h3, :scope > form > button");
      const button = node.querySelector(":scope > form > button");
      const command = node.querySelector(':scope > form > input[name="command"]');
      return { identity, kind, caption: heading?.textContent ?? node.textContent,
        ...(value ? { scalar: { type: value.dataset.type, value: value.dataset.value, domain: value.dataset.domain,
          member: value.dataset.member, undefined: value.dataset.undefined === "true", closing: value.dataset.closing === "true" } } : {}),
        ...(command ? { command: command.value, enabled: !button.disabled } : {}) };
    };
    const rows = [...article.querySelectorAll('[data-kind="row"]')].map(row => {
      const entries = [...row.querySelectorAll("[data-control]")].map(readControl);
      const action = entries.at(-1);
      return { handle: row.dataset.row, selected: row.dataset.selected === "true",
        caption: row.querySelector(":scope > h2").textContent, controls: entries.slice(0, -1),
        select: { control: action.identity, command: action.command, enabled: action.enabled } };
    });
    const controls = [...article.querySelectorAll("[data-control]")]
      .filter(node => !node.closest('[data-kind="rows"]')).map(readControl);
    for (const row of rows) controls.push({ identity: row.select.control, kind: "action", caption: "Select row",
      command: row.select.command, enabled: row.select.enabled });
    return {
      profile: article.dataset.agiruProfile, view: article.dataset.view, page: article.dataset.page,
      handle: article.dataset.handle, revision: article.dataset.revision,
      caption: article.querySelector(":scope > h1").textContent,
      controls,
      ...(article.dataset.agiruProfile === "3" ? { interaction: { state: article.dataset.state,
        call: article.dataset.call, originCommand: article.dataset.originCommand,
        ...(article.dataset.dialog ? { dialog: article.dataset.dialog, defaultChoice: article.dataset.default,
          prompt: article.querySelector('[data-prompt]').textContent } : {}) } } : {}),
      ...([...article.querySelectorAll('[data-message]')].length ? { messages:
        [...article.querySelectorAll('[data-message]')].map(node => ({ handle: node.dataset.message, text: node.textContent })) } : {}),
      ...(article.dataset.agiruProfile === "2" ? { rows, window: { limit: article.dataset.limit,
        more: article.dataset.more === "true", direction: article.dataset.direction } } : {}),
      unsupported: article.querySelector(":scope > output[data-unsupported-count]").dataset.unsupportedCount,
    };
  });
  const expectedControl = control => ({ identity: control.identity, kind: control.kind, caption: control.caption,
    ...(control.scalar ? { scalar: control.scalar } : {}),
    ...(control.operation ? { command: control.operation.command, enabled: control.operation.enabled } : {}) });
  const controls = model.controls.map(expectedControl);
  const rows = model.rows?.map(row => ({ handle: row.handle, selected: row.selected, caption: row.caption,
    controls: row.controls.map(expectedControl), select: { control: row.select.control,
      command: row.select.command, enabled: row.select.enabled } }));
  assert.deepEqual(rendered, { profile: model.profile, view: model.view, page: model.page, handle: model.handle,
    revision: model.revision, caption: model.caption, controls, unsupported: String(model.unsupported),
    ...(model.interaction ? { interaction: model.interaction } : {}),
    ...(model.messages?.length ? { messages: model.messages } : {}),
    ...(rows ? { rows, window: model.window } : {}) });
}

export async function browserAction(page, origin, model, identity) {
  const escaped = await page.evaluate(value => CSS.escape(value), identity);
  const endpoint = model.interaction?.dialog ? "/answers" : "/commands";
  const received = page.waitForResponse(response => response.url() === `${origin}${endpoint}` &&
    response.request().method() === "POST");
  await page.locator(`[data-control="${escaped}"] button`).click();
  const response = await finishedResponse(page, origin, await received);
  assert.equal(response.status(), 200, await response.text());
  await page.waitForFunction(previous => {
    const article = document.querySelector("#workspace article");
    return article?.dataset.revision !== previous.revision ||
      (article?.dataset.agiruProfile === "3" && article.dataset.dialog !== previous.interaction?.dialog);
  }, model);
  await settled(page);
  const result = { page: parsePage(await response.text()), status: response.status() };
  await assertBrowserPage(page, result.page);
  return result;
}

export async function browserSet(page, origin, model, identity, text) {
  const escaped = await page.evaluate(value => CSS.escape(value), identity);
  const control = page.locator(`[data-control="${escaped}"]`);
  const received = page.waitForResponse(response => response.url() === `${origin}/commands` &&
    response.request().method() === "POST");
  await control.locator("input[name=text]").fill(text);
  await control.locator("button").click();
  const response = await finishedResponse(page, origin, await received);
  assert.equal(response.status(), 200, await response.text());
  await page.waitForFunction(previous => document.querySelector("#workspace article")?.dataset.revision !== previous,
    model.revision);
  await settled(page);
  const result = { page: parsePage(await response.text()), status: response.status() };
  await assertBrowserPage(page, result.page);
  return result;
}

export async function finishedResponse(page, origin, response) {
  if (response.status() !== 200) return response;
  const interaction = parsePage(await response.text()).interaction;
  if (!interaction || interaction.state !== "working") return response;
  return page.waitForResponse(async next => next.url() === `${origin}/calls/${interaction.call}` &&
    (next.status() !== 200 || parsePage(await next.text()).interaction?.state !== "working"));
}
