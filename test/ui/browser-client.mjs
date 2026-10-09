import assert from "node:assert/strict";
import { createRequire } from "node:module";
import { parsePage, parseFailure } from "../../build/client/profile.mjs";

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
      const value = node.querySelector(":scope > form > input[name=text], :scope > form > select[name=text], :scope > output[data-type]");
      const heading = node.querySelector(":scope > h2, :scope > h3, :scope > form > button");
      const button = node.querySelector(":scope > form > button");
      const command = node.querySelector(':scope > form > input[name="command"]');
      return { identity, kind, caption: heading?.textContent ?? node.textContent,
        ...(value ? { scalar: { type: value.dataset.type, value: value.dataset.value, domain: value.dataset.domain,
          member: value.dataset.member, undefined: value.dataset.undefined === "true", closing: value.dataset.closing === "true" } } : {}),
        ...(value?.tagName === "SELECT" ? { choices: [...value.options].filter(option => !option.disabled).map(option => ({
          value: option.value, member: option.dataset.member, caption: option.textContent })) } : {}),
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
      ...(["3", "4"].includes(article.dataset.agiruProfile) ? { interaction: { state: article.dataset.state,
        call: article.dataset.call, originCommand: article.dataset.originCommand,
        ...(article.dataset.poll ? { poll: { path: article.dataset.poll, state: article.dataset.pollState } } : {}),
        ...(article.dataset.state === "modal" ? { dialog: article.dataset.dialog } : article.dataset.dialog ? { dialog: article.dataset.dialog, defaultChoice: article.dataset.default,
          prompt: article.querySelector('[data-prompt]').textContent } : {}) } } : {}),
      ...([...article.querySelectorAll('[data-message]')].length ? { messages:
        [...article.querySelectorAll('[data-message]')].map(node => ({ handle: node.dataset.message, text: node.textContent })) } : {}),
      ...(article.dataset.view === "list" ? { rows, window: { limit: article.dataset.limit,
        more: article.dataset.more === "true", direction: article.dataset.direction } } : {}),
      unsupported: article.querySelector(":scope > output[data-unsupported-count]").dataset.unsupportedCount,
    };
  });
  const expectedControl = control => ({ identity: control.identity, kind: control.kind, caption: control.caption,
    ...(control.scalar ? { scalar: control.scalar } : {}),
    ...(control.choices ? { choices: control.choices } : {}),
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

async function changed(page, model) {
  await page.waitForFunction(previous => {
    const article = document.querySelector("#workspace article");
    return article?.dataset.revision !== previous.revision ||
      (article?.dataset.state !== previous.interaction?.state || article?.dataset.dialog !== previous.interaction?.dialog);
  }, model);
  await settled(page);
}

export async function browserAction(page, origin, model, identity) {
  const escaped = await page.evaluate(value => CSS.escape(value), identity);
  const endpoint = model.interaction?.state === "modal" ? `/modal-commands/${model.interaction.dialog}` :
    model.interaction?.dialog ? "/answers" : "/commands";
  const received = page.waitForResponse(response => response.url() === `${origin}${endpoint}` &&
    response.request().method() === "POST");
  await page.locator(`[data-control="${escaped}"] button`).click();
  const response = await finishedResponse(page, origin, await received);
  assert.equal(response.status(), 200, await response.text());
  await changed(page, model);
  const result = { page: parsePage(await response.text()), status: response.status() };
  await assertBrowserPage(page, result.page);
  return result;
}

export async function browserSet(page, origin, model, identity, text) {
  const escaped = await page.evaluate(value => CSS.escape(value), identity);
  const control = page.locator(`[data-control="${escaped}"]`);
  const endpoint = model.interaction?.state === "modal" ? `/modal-commands/${model.interaction.dialog}` : "/commands";
  const received = page.waitForResponse(response => response.url() === `${origin}${endpoint}` &&
    response.request().method() === "POST");
  if (await control.locator("select[name=text]").count()) {
    const choice = model.controls.find(control => control.identity === identity)?.choices?.find(choice =>
      choice.value === text || choice.member === text);
    assert.ok(choice, "the browser must explicitly choose a server-declared enum value");
    await control.locator("select[name=text]").selectOption(choice.value);
  } else await control.locator("input[name=text]").fill(text);
  await control.locator("button").click();
  const response = await finishedResponse(page, origin, await received);
  assert.equal(response.status(), 200, await response.text());
  await changed(page, model);
  const result = { page: parsePage(await response.text()), status: response.status() };
  await assertBrowserPage(page, result.page);
  return result;
}

export async function finishedResponse(page, origin, response) {
  if (response.status() !== 200) return response;
  const interaction = parsePage(await response.text()).interaction;
  if (!interaction || (interaction.state !== "working" && interaction.poll?.state !== "failed")) return response;
  const next = await page.waitForResponse(reply => reply.url() === origin +
    (interaction.poll?.path ?? `/calls/${interaction.call}`));
  return finishedResponse(page, origin, next);
}

export async function browserFailure(page, origin, model, identity, text) {
  const escaped = await page.evaluate(value => CSS.escape(value), identity);
  const endpoint = `/modal-commands/${model.interaction.dialog}`;
  const received = page.waitForResponse(response => response.url() === origin + endpoint &&
    response.request().method() === "POST");
  if (text !== undefined) await page.locator(`[data-control="${escaped}"] input[name=text]`).fill(text);
  await page.locator(`[data-control="${escaped}"] button`).click();
  const response = await finishedResponse(page, origin, await received);
  assert.equal(response.status(), 500);
  const failure = parseFailure(await response.text());
  await page.waitForFunction(expected => document.querySelector("#status")?.textContent.includes(expected),
    failure.message);
  await page.waitForFunction(previous => document.querySelector("#workspace article")?.dataset.revision !== previous,
    model.revision);
  return { error: failure.code, message: failure.message, outcome: failure.outcome, command: failure.command };
}
