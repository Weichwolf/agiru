import assert from "node:assert/strict";
import { test, before, after } from "node:test";
import { spawn, execFile } from "node:child_process";
import { promisify } from "node:util";
import { chmod, writeFile } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient, readAuth } from "../../build/client/http.mjs";
import { parsePage, parseFailure } from "../../build/client/profile.mjs";
import { ServerConfigs } from "./server-config.mjs";
import { assertBrowserPage, browserAction, browserSet, finishedResponse, launchBrowser, openBrowserPage } from "./browser-client.mjs";

const execute = promisify(execFile);
const container = process.env.AGIRU_DEV_CONTAINER ?? "agiru-dev";
const origin = `http://127.0.0.1:${process.env.AGIRU_DEV_HTTP_PORT ?? "8080"}`;
const native = process.env.AGIRU_ERP_NATIVE;
const proof = process.env.AGIRU_ERP_PROOF;
const database = process.env.AGIRU_ERP_DATABASE;
const company = process.env.AGIRU_ERP_COMPANY;
assert.match(database ?? "", /^agiru_erp_gate_[A-Za-z0-9_]+$/);
assert.match(native ?? "", /^\/tmp\/agiru-erp-fixture\.[A-Za-z0-9]+$/);
assert.match(proof ?? "", /^\/tmp\/agiru-erp-fixture\.[A-Za-z0-9]+$/);
assert.ok(company);
const dsn = `postgresql://agiru:agiru@127.0.0.1:5432/${database}`;
const configurations = new ServerConfigs(container, native, proof);
const path = result => `/?handle=${result.page.handle}`;
const field = (result, name) => result.page.controls.find(control => control.identity === name)?.scalar?.value;
const operation = (result, name, text) => {
  const command = result.page.controls.find(control => control.identity === name)?.operation;
  assert.ok(command?.enabled, `native ERP page must advertise enabled ${name}`);
  return { path: path(result), page: result.page.handle, revision: result.page.revision,
    command: command.command, control: command.control, operation: command.operation,
    ...(text === undefined ? {} : { text }) };
};
const quoted = value => `'${value.replaceAll("'", "''")}'`;
async function sql(statement) {
  return (await execute("podman", ["exec", "--user", "agiru", container,
    "psql", "-XAt", "-v", "ON_ERROR_STOP=1", "-d", database, "-c", statement])).stdout.trim();
}
async function customer(number) {
  return JSON.parse(await sql(`SELECT json_build_object('name',"Name",'search',"Search Name",
    'modifier',"SystemModifiedBy"::text,'version',"timestamp"::text,
    'creator',"SystemCreatedBy"::text,'address',"Address",'country',"Country/Region Code",
    'credit',"Credit Limit (LCY)"::text)
    FROM "Customer" WHERE "No."=${quoted(number)}`));
}
async function ledgerSnapshot() {
  const tables = ["Cust. Ledger Entry", "G/L Entry", "Item Ledger Entry", "Value Entry"];
  return sql(`SELECT ${tables.map(name => `(SELECT count(*)::text || ':' ||
    COALESCE(md5(string_agg(md5(to_jsonb(entry)::text), '' ORDER BY "Entry No.")), 'empty')
    FROM "${name}" entry)`).join(",")}`);
}
async function customerSnapshot(number) {
  return sql(`SELECT to_jsonb(customer)::text FROM "Customer" customer WHERE "No."=${quoted(number)}`);
}
async function cmd(name, value, denied = false) {
  return execute(process.execPath, ["build/client/cmd.mjs", "--json", name,
    typeof value === "string" ? value : JSON.stringify(value)], {
    env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json${denied ? ".denied" : ""}` },
    timeout: 30000, maxBuffer: 4194304,
  });
}
async function mcp(name, args, denied = false) {
  const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
  const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
  const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
  const transport = new StdioClientTransport({ command: process.execPath, args: ["build/client/mcp.mjs"],
    env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: `${proof}/auth.json${denied ? ".denied" : ""}` } });
  const client = new Client({ name: "agiru-native-erp-gate", version: "1" });
  await client.connect(transport);
  try { return await client.callTool({ name: `agiru_${name}`, arguments: args }); }
  finally { await client.close(); }
}

async function workflowDriver(adapter, initial) {
  const web = adapter === "Web" ? await openBrowserPage(browser, origin, path(initial), first.authorization) : undefined;
  if (web) await assertBrowserPage(web.page, initial.page);
  return {
    async action(current, identity, text) {
      const request = operation(current, identity, text);
      if (adapter === "CMD") return JSON.parse((await cmd("execute", request)).stdout);
      if (adapter === "MCP") {
        const reply = await mcp("execute", request);
        assert.ok(!reply.isError, JSON.stringify(reply.structuredContent));
        return reply.structuredContent;
      }
      return text === undefined ? browserAction(web.page, origin, current.page, identity)
        : browserSet(web.page, origin, current.page, identity, text);
    },
    async attemptedSet(current, identity, text) {
      const request = operation(current, identity, text);
      if (adapter === "CMD") {
        try { return { request, result: JSON.parse((await cmd("execute", request)).stdout) }; }
        catch (error) {
          assert.equal(error.code, 2);
          assert.equal(error.stdout, "");
          return { request, failure: JSON.parse(error.stderr) };
        }
      }
      if (adapter === "MCP") {
        const reply = await mcp("execute", request);
        if (!reply.isError) return { request, result: reply.structuredContent };
        assert.deepEqual(JSON.parse(reply.content[0].text), reply.structuredContent);
        return { request, failure: reply.structuredContent };
      }
      assert.equal(current.page.interaction, undefined, "this attempt must use an original root card");
      const escaped = await web.page.evaluate(value => CSS.escape(value), identity);
      const control = web.page.locator(`[data-control="${escaped}"]`);
      const received = web.page.waitForResponse(response => response.url() === `${origin}/commands` &&
        response.request().method() === "POST");
      await control.locator("input[name=text]").fill(text);
      await control.locator("button").click();
      const response = await finishedResponse(web.page, origin, await received);
      if (response.status() === 200) {
        return { request, result: { page: parsePage(await response.text()), status: response.status() } };
      }
      const failure = parseFailure(await response.text());
      await web.page.waitForFunction(message => document.querySelector("#status")?.textContent.includes(message),
        failure.message);
      return { request, failure: { error: failure.code, message: failure.message,
        outcome: failure.outcome, command: failure.command } };
    },
    async screenshot(name) {
      if (web) await web.page.screenshot({ path: `${proof}/${name}.png`, fullPage: true });
    },
    async close() { await web?.page.close(); },
  };
}

async function reopenCustomer(adapter, number) {
  let list = JSON.parse((await cmd("read", target)).stdout);
  const driver = await workflowDriver(adapter, list);
  const blocks = Math.ceil(Number(await sql('SELECT count(*) FROM "Customer"')) / 40) + 1;
  try {
    for (let block = 0; block < blocks; ++block) {
      assert.equal(list.page.window.limit, "40");
      const row = list.page.rows.find(row => row.controls.find(control => control.identity === "No.")?.scalar?.value === number);
      if (row) {
        list = await driver.action(list, row.select.control);
        const card = await driver.action(list, "$agiru.card");
        await driver.screenshot(`customer-${adapter.toLowerCase()}-reopened`);
        return card;
      }
      if (!list.page.window.more) break;
      list = await driver.action(list, "$agiru.next");
    }
    assert.fail(`independent Customer List did not expose newly created ${number}`);
  } finally { await driver.close(); }
}

let server, closed, pid, first, denied, client, list, card, saved, selected, original, browser;
let output = "", diagnostic = "";
before(async () => {
  const config = await configurations.write({ database: dsn, company, origin });
  for (const suffix of ["", ".denied"]) {
    await execute("podman", ["cp", `${container}:${native}/auth.json${suffix}`, `${proof}/auth.json${suffix}`]);
    await chmod(`${proof}/auth.json${suffix}`, 0o600);
  }
  first = await readAuth(`${proof}/auth.json`);
  denied = await readAuth(`${proof}/auth.json.denied`);
  client = new AgentClient(origin, first, 30000);
  browser = await launchBrowser();
  server = spawn("podman", ["exec", "--user", "agiru", container,
    "env", `LD_LIBRARY_PATH=${native}/binaries`,
    ...(process.env.AGIRU_ERP_PRELOAD ? [`LD_PRELOAD=${process.env.AGIRU_ERP_PRELOAD}`] : []), "sh", "-c",
    'printf "START %s\\n" "$$"; exec "$@"', "agiru-erp-native",
    `${native}/binaries/agiru`, "serve", "--config", config],
    { stdio: ["ignore", "pipe", "pipe"] });
  closed = new Promise(resolve => server.once("close", resolve));
  server.stderr.on("data", chunk => { diagnostic += chunk; });
  await new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error(`native ERP server startup timed out: ${diagnostic}`)), 30000);
    server.once("error", error => { clearTimeout(timer); reject(error); });
    server.once("close", code => { clearTimeout(timer); reject(new Error(`native ERP server exited ${code}: ${diagnostic}`)); });
    server.stdout.on("data", chunk => {
      output += chunk;
      pid = output.match(/^START ([1-9][0-9]*)$/m)?.[1] ?? pid;
      const ready = output.match(/^READY ([1-9][0-9]*) 18080$/m);
      if (ready) { clearTimeout(timer); assert.equal(ready[1], pid); resolve(); }
    });
  });
}, { timeout: 35000 });
after(async () => {
  await browser?.close();
  if (pid && server.exitCode === null) {
    await execute("podman", ["exec", "--user", "agiru", container, "kill", "-TERM", pid]);
    const timer = setTimeout(() => server.kill(), 15000);
    try { assert.equal(await closed, 0, "native ERP server must drain and shut down cleanly"); }
    finally { clearTimeout(timer); }
  }
  await configurations.clean();
  await writeFile(`${proof}/server.log`, output + diagnostic);
  for (const auth of [first, denied]) {
    if (auth) assert.ok(!(output + diagnostic).includes(auth.authorization.slice(7)), "no credentials in server logs");
  }
});
const target = `/?page=22&company=${encodeURIComponent(company)}`;

test("original Customer List denies the unassigned user over web, CMD and MCP without SQL effects", { timeout: 30000 }, async () => {
  const response = await fetch(origin + target, { headers: denied });
  assert.equal(response.status, 403);
  const html = await response.text();
  await writeFile(`${proof}/denied.html`, html);
  assert.ok(!html.includes('data-control="No."'));
  let error;
  try { await cmd("read", target, true); } catch (caught) { error = caught; }
  assert.ok(error, "actual shell CMD must refuse an unassigned user");
  assert.equal(error.code, 2);
  const failure = JSON.parse(error.stderr);
  assert.equal(failure.error, "Permission");
  assert.equal(failure.outcome, "refused");
  const reply = await mcp("read", { path: target }, true);
  assert.equal(reply.isError, true);
  assert.deepEqual(reply.structuredContent, failure);
  const web = await openBrowserPage(browser, origin, target, denied.authorization);
  assert.equal(web.response.status(), 403);
  assert.equal(await web.page.locator("#status").textContent(),
    `Server error ${failure.error}: ${failure.message} outcome=${failure.outcome}`);
  assert.equal(await web.page.locator("#workspace [data-control]").count(), 0);
  await web.page.screenshot({ path: `${proof}/customer-denied.png`, fullPage: true });
  await web.page.close();
  assert.equal(await sql('SELECT count(*) FROM "Customer"'), "68");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_contexts"), "0");
});

test("original Customer List opens over Caddy and retains identical web/CMD/MCP typed state", { timeout: 30000 }, async () => {
  const response = await fetch(origin + target, { headers: first });
  const html = await response.text();
  await writeFile(`${proof}/customer-list.html`, html);
  assert.equal(response.status, 200, html);
  const initial = { page: parsePage(html), status: response.status };
  list = initial.page.interaction?.state === "working"
    ? await client.read(`/calls/${initial.page.interaction.call}`) : initial;
  if (initial.page.interaction) {
    const completed = await fetch(origin + path(list), { headers: first });
    assert.equal(completed.status, 200);
    const finalHtml = await completed.text();
    assert.deepEqual(parsePage(finalHtml), list.page);
    await writeFile(`${proof}/customer-list.html`, finalHtml);
  }
  assert.equal(list.page.page, "22");
  assert.equal(list.page.profile, "2");
  assert.equal(list.page.view, "list");
  assert.deepEqual(list.page.window, { limit: "40", more: true, direction: "forward" });
  const expectedRows = JSON.parse(await sql(`SELECT json_agg(row_to_json(c)) FROM
    (SELECT "No." AS number,"Name" AS name FROM "Customer" ORDER BY "No." LIMIT 40) c`));
  assert.equal(expectedRows.length, 40);
  assert.deepEqual(list.page.rows.map(row => {
    const number = row.controls.find(control => control.identity === "No.")?.scalar;
    const name = row.controls.find(control => control.identity === "Name")?.scalar;
    assert.equal(number?.type, "Code");
    assert.equal(name?.type, "Text");
    return { number: number.value, name: name.value };
  }), expectedRows, "the original Customer window must match independent PostgreSQL key order and values");
  assert.equal(list.page.rows.filter(row => row.selected).length, 1);
  assert.equal(list.page.rows[0].selected, true);
  selected = field(list, "No.");
  assert.equal(selected, expectedRows[0].number);
  assert.ok(selected, "original customer key must be exposed exactly");
  original = await customer(selected);
  assert.equal(field(list, "Name"), original.name);
  const shell = await cmd("read", path(list));
  assert.equal(shell.stderr, "");
  assert.deepEqual(JSON.parse(shell.stdout), list);
  const reply = await mcp("read", { path: path(list) });
  assert.ok(!reply.isError);
  assert.deepEqual(reply.structuredContent, list);
  const web = await openBrowserPage(browser, origin, path(list), first.authorization);
  assert.equal(web.response.status(), 200);
  await assertBrowserPage(web.page, list.page);
  await web.page.screenshot({ path: `${proof}/customer-list-window.png`, fullPage: true });
  await web.page.close();
  await writeFile(`${proof}/customer-list.json`, JSON.stringify(list));
});

test("CMD discovers and opens the original Customer Card with the exact selected SQL key", { timeout: 30000 }, async () => {
  assert.ok(list, "Customer List prerequisite failed; card was not executed");
  card = JSON.parse((await cmd("execute", operation(list, "$agiru.card"))).stdout);
  assert.equal(card.page.page, "21");
  assert.equal(await sql('SELECT "Manual Nos." FROM "No. Series" WHERE "Code" IN ' +
    '(SELECT "Customer Nos." FROM "Sales & Receivables Setup")'), "t", "seed requires the interactive No. field");
  assert.equal(field(card, "No."), selected,
    "Customer Card interactive OnOpenPage must expose the exact SQL key");
  assert.equal(field(card, "Name"), original.name);
  assert.deepEqual(await client.read(path(card)), card);
});

test("CMD validates and saves an original Customer Name with Unicode and independently attributed SQL effects", { timeout: 30000 }, async () => {
  assert.ok(card, "Customer Card prerequisite failed; Validate/Save was not executed");
  const name = "AGIRU CMD QUALIFY Ω 雪";
  saved = JSON.parse((await cmd("execute", operation(card, "Name", name))).stdout);
  assert.equal(field(saved, "Name"), name);
  const row = await customer(selected);
  assert.equal(row.name, name);
  assert.equal(row.modifier, "00000000-0000-0000-0000-000000000001");
  assert.ok(BigInt(row.version) > BigInt(original.version));
  if (!original.search || original.search === original.name.toUpperCase()) assert.equal(row.search, name);
  assert.deepEqual(await client.read(path(saved)), saved);
});

test("MCP validates and saves the same original Customer through the same command runtime", { timeout: 30000 }, async () => {
  assert.ok(saved, "CMD Save prerequisite failed; MCP Save was not executed");
  const name = "AGIRU MCP QUALIFY Ω 雪";
  const reply = await mcp("execute", operation(saved, "Name", name));
  assert.ok(!reply.isError, JSON.stringify(reply.structuredContent));
  saved = reply.structuredContent;
  assert.equal(field(saved, "Name"), name);
  const row = await customer(selected);
  assert.equal(row.name, name);
  assert.equal(row.modifier, "00000000-0000-0000-0000-000000000001");
  assert.deepEqual(await client.read(path(saved)), saved);
});

test("actual Chromium and htmx validate and save the same Customer without agent-only business logic", { timeout: 30000 }, async () => {
  assert.ok(saved, "agent Save prerequisite failed; HTML form was not executed");
  const name = "AGIRU WEB QUALIFY Ω 雪";
  const web = await openBrowserPage(browser, origin, path(saved), first.authorization);
  assert.equal(web.response.status(), 200);
  await assertBrowserPage(web.page, saved.page);
  saved = await browserSet(web.page, origin, saved.page, "Name", name);
  await web.page.screenshot({ path: `${proof}/customer-card-saved.png`, fullPage: true });
  await web.page.close();
  assert.equal(field(saved, "Name"), name);
  const row = await customer(selected);
  assert.equal(row.name, name);
  assert.equal(row.modifier, "00000000-0000-0000-0000-000000000001");
  assert.deepEqual(await client.read(path(saved)), saved);
});

test("Customer edits retain full row population and durable command receipts", async () => {
  assert.ok(saved, "Customer edits prerequisite failed; acceptance remains incomplete");
  assert.equal(await sql('SELECT count(*) FROM "Customer"'), "68");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_commands WHERE outcome='failed'"), "0");
  assert.ok(BigInt(await sql("SELECT count(*) FROM agiru_client.page_commands WHERE outcome='complete'")) >= 4n);
  assert.equal(await sql("SELECT count(*) FROM pg_stat_activity WHERE datname=current_database()"), "1");
});

test("original Customer New exposes the explicit template selection before creating a customer", { timeout: 30000 }, async () => {
  assert.equal(await sql('SELECT count(*) FROM "Customer Templ."'), "3",
    "the unchanged reference seed requires an explicit choice, not a single-template shortcut");
  assert.equal(await sql('SELECT "Default Nos." FROM "No. Series" WHERE "Code" IN ' +
    '(SELECT "Customer Nos." FROM "Sales & Receivables Setup")'), "t");
  const before = await sql('SELECT count(*) FROM "Customer"');
  try {
    const fresh = JSON.parse((await cmd("read", target)).stdout);
    const selected = JSON.parse((await cmd("execute", operation(fresh, "$agiru.new"))).stdout);
    assert.equal(selected.page.page, "1380", "the active modal must be the original template list");
    const templates = JSON.parse(await sql(`SELECT json_agg(row_to_json(t)) FROM
      (SELECT "Code" AS code,"Description" AS description FROM "Customer Templ." ORDER BY "Code" LIMIT 40) t`));
    assert.deepEqual(selected.page.rows.map(row => ({
      code: row.controls.find(control => control.identity === "Code")?.scalar?.value,
      description: row.controls.find(control => control.identity === "Description")?.scalar?.value,
    })), templates);
    await writeFile(`${proof}/customer-template-selection.json`, JSON.stringify(selected));
  } finally {
    assert.equal(await sql('SELECT count(*) FROM "Customer"'), before,
      "no automatic template choice or customer insertion before the explicit answer");
  }
});

for (const adapter of ["CMD", "MCP", "Web"]) {
  test(`${adapter} creates an original Customer from an explicitly selected template, edits and independently reopens it`,
    { timeout: 120000 }, async () => {
      const population = Number(await sql('SELECT count(*) FROM "Customer"'));
      const ledgers = await ledgerSnapshot();
      const fresh = JSON.parse((await cmd("read", target)).stdout);
      const driver = await workflowDriver(adapter, fresh);
      let created;
      const steps = [];
      const inputs = { Name: `AGIRU ${adapter} CUSTOMER Ω 雪`, Address: "AGIRU Testweg 6",
        "Country/Region Code": "CH", "Credit Limit (LCY)": "1234.56" };
      try {
        let modal = await driver.action(fresh, "$agiru.new");
        assert.equal(modal.page.page, "1380");
        assert.equal(modal.page.interaction.state, "modal");
        assert.equal(modal.page.window.limit, "40");
        assert.equal(await sql('SELECT count(*) FROM "Customer"'), String(population));
        assert.equal(modal.page.rows.length, 3, "unchanged seed must retain all three template choices");
        const templateRow = modal.page.rows[1];
        const templateCode = templateRow.controls.find(control => control.identity === "Code")?.scalar?.value;
        assert.ok(templateCode);
        assert.equal(templateRow.selected, false, "explicitly select a non-default template");
        modal = await driver.action(modal, templateRow.select.control);
        assert.equal(field(modal, "Code"), templateCode);
        assert.equal(await sql('SELECT count(*) FROM "Customer"'), String(population));
        created = await driver.action(modal, "$agiru.modal_ok");
        assert.equal(created.page.page, "21", "resume the original Customer Card, not a replacement mask");
        assert.equal(created.page.interaction, undefined);
        const number = field(created, "No.");
        assert.ok(number, "original AL numbering must assign a new customer key");
        assert.equal(await sql('SELECT count(*) FROM "Customer"'), String(population + 1));
        assert.equal((await customer(number)).creator, "00000000-0000-0000-0000-000000000001");
        const inherited = ["Gen. Bus. Posting Group", "VAT Bus. Posting Group", "Customer Posting Group",
          "Payment Method Code", "Currency Code"];
        for (const name of inherited) {
          assert.equal(await sql(`SELECT "${name}" FROM "Customer" WHERE "No."=${quoted(number)}`),
            await sql(`SELECT "${name}" FROM "Customer Templ." WHERE "Code"=${quoted(templateCode)}`),
            `original AL must transfer ${name} from the explicitly selected template`);
        }
        for (const [name, value] of Object.entries(inputs)) {
          created = await driver.action(created, name, value);
          steps.push({ control: name, input: value, page: created.page, stored: await customer(number) });
          await writeFile(`${proof}/customer-${adapter.toLowerCase()}-steps.json`, JSON.stringify({
            template: templateCode, number, steps }));
          assert.equal(field(created, name), value);
        }
        const stored = await customer(number);
        assert.equal(stored.name, inputs.Name);
        assert.equal(stored.address, inputs.Address);
        assert.equal(stored.country, inputs["Country/Region Code"]);
        assert.equal(await sql(`SELECT "Credit Limit (LCY)"=${quoted(inputs["Credit Limit (LCY)"])}::numeric
          FROM "Customer" WHERE "No."=${quoted(number)}`), "t");
        assert.equal(stored.modifier, "00000000-0000-0000-0000-000000000001");
        assert.equal(await sql(`SELECT count(*) FROM "Cust. Ledger Entry" WHERE "Customer No."=${quoted(number)}`), "0");
        assert.equal(await ledgerSnapshot(), ledgers,
          "master-data entry must not insert, delete or change any of the four ledger populations");
        await driver.screenshot(`customer-${adapter.toLowerCase()}-created`);
        await writeFile(`${proof}/customer-${adapter.toLowerCase()}-created.json`, JSON.stringify({
          template: templateCode, number, inputs, stored, page: created.page, ledgers }));
      } finally { await driver.close(); }
      const reopened = await reopenCustomer(adapter, field(created, "No."));
      assert.notEqual(reopened.page.handle, created.page.handle, "reopen through an independent original list/card instance");
      assert.equal(field(reopened, "No."), field(created, "No."));
      for (const [name, value] of Object.entries(inputs)) {
        if (name !== "Credit Limit (LCY)") assert.equal(field(reopened, name), value);
      }
      assert.equal(field(reopened, "Credit Limit (LCY)"), (await customer(field(created, "No."))).credit,
        "fresh exact Decimal transport must retain the declared SQL storage scale, not binary floating point");
      assert.equal(field(reopened, "Balance (LCY)"), "0");
    });
}

for (const adapter of ["CMD", "MCP", "Web"]) {
  test(`${adapter} refuses an original Customer edit from a stale page without overwriting a peer's committed changes`,
    { timeout: 120000 }, async () => {
      assert.ok(selected, "original Customer List prerequisite failed; concurrency was not executed");
      const stale = await reopenCustomer("CMD", selected);
      const driver = await workflowDriver(adapter, stale);
      try {
        const peer = await reopenCustomer("CMD", selected);
        assert.notEqual(peer.page.handle, stale.page.handle);
        const observed = await customer(selected);
        const peerName = `AGIRU ${adapter} COMMITTED PEER Ω 雪`;
        const committed = JSON.parse((await cmd("execute", operation(peer, "Name", peerName))).stdout);
        assert.equal(field(committed, "Name"), peerName);
        const current = await customer(selected);
        assert.equal(current.name, peerName);
        assert.ok(BigInt(current.version) > BigInt(observed.version));
        const unchanged = await customerSnapshot(selected);
        const ledgers = await ledgerSnapshot();
        const attempt = await driver.attemptedSet(stale, "Credit Limit (LCY)", "8765.43");
        await writeFile(`${proof}/customer-${adapter.toLowerCase()}-stale-write.json`, JSON.stringify({
          stale: stale.page, committed: committed.page, before: current, attempt,
          stored: await customer(selected) }));
        assert.equal(await customerSnapshot(selected), unchanged,
          "a stale page must not overwrite any committed field, audit value or rowversion");
        assert.equal(await ledgerSnapshot(), ledgers, "a rejected master-data write must leave all ledgers unchanged");
        assert.equal(attempt.result, undefined, "a stale edit must not return a successful page");
        assert.ok(attempt.failure?.message, "a stale edit must return an explicit native diagnostic");
        assert.equal(attempt.failure.outcome, "failed");
        assert.equal(attempt.failure.command, attempt.request.command);
        assert.notEqual(attempt.failure.error, "ServerFailure");
        assert.notEqual(attempt.failure.error, "WriteUncertain");
        assert.equal(await sql(`SELECT outcome FROM agiru_client.page_commands
          WHERE handle=${quoted(stale.page.handle)} AND command_id=${quoted(attempt.request.command)}`), "failed");
        await driver.screenshot(`customer-${adapter.toLowerCase()}-stale-write-refused`);
      } finally { await driver.close(); }
    });
}
