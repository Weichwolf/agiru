import assert from "node:assert/strict";
import { test, before, after } from "node:test";
import { spawn, execFile } from "node:child_process";
import { promisify } from "node:util";
import { chmod, writeFile } from "node:fs/promises";
import { createRequire } from "node:module";
import { AgentClient, readAuth } from "../../build/client/http.mjs";
import { commandEnvelope, parsePage } from "../../build/client/profile.mjs";
import { ServerConfigs } from "./server-config.mjs";

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
    'modifier',"SystemModifiedBy"::text,'version',"timestamp"::text)
    FROM "Customer" WHERE "No."=${quoted(number)}`));
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

let server, closed, pid, first, denied, client, list, card, saved, selected, original;
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
  const reply = await mcp("read", { path: target }, true);
  assert.equal(reply.isError, true);
  assert.deepEqual(reply.structuredContent, JSON.parse(error.stderr));
  assert.equal(await sql('SELECT count(*) FROM "Customer"'), "68");
  assert.equal(await sql("SELECT count(*) FROM agiru_client.page_contexts"), "0");
});

test("original Customer List opens over Caddy and retains identical web/CMD/MCP typed state", { timeout: 30000 }, async () => {
  const response = await fetch(origin + target, { headers: first });
  const html = await response.text();
  await writeFile(`${proof}/customer-list.html`, html);
  assert.equal(response.status, 200, html);
  list = { page: parsePage(html), status: response.status };
  assert.equal(list.page.page, "22");
  selected = field(list, "No.");
  assert.ok(selected, "original customer key must be exposed exactly");
  original = await customer(selected);
  assert.equal(field(list, "Name"), original.name);
  const shell = await cmd("read", path(list));
  assert.equal(shell.stderr, "");
  assert.deepEqual(JSON.parse(shell.stdout), list);
  const reply = await mcp("read", { path: path(list) });
  assert.ok(!reply.isError);
  assert.deepEqual(reply.structuredContent, list);
  await writeFile(`${proof}/customer-list.json`, JSON.stringify(list));
});

test("CMD discovers and opens the original Customer Card with the exact selected SQL key", { timeout: 30000 }, async () => {
  assert.ok(list, "Customer List prerequisite failed; card was not executed");
  card = JSON.parse((await cmd("execute", operation(list, "$agiru.card"))).stdout);
  assert.equal(card.page.page, "21");
  assert.equal(field(card, "No."), selected);
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

test("the actual HTML form validates and saves the same Customer without agent-only business logic", { timeout: 30000 }, async () => {
  assert.ok(saved, "agent Save prerequisite failed; HTML form was not executed");
  const name = "AGIRU WEB QUALIFY Ω 雪";
  const command = operation(saved, "Name", name);
  const envelope = commandEnvelope(saved.page, { ...command, enabled: true }, name);
  const response = await fetch(origin + envelope.path, { method: "POST", headers: {
    ...first, Origin: origin, "Content-Type": "application/x-www-form-urlencoded", "HX-Request": "true" },
    body: new URLSearchParams(envelope.fields), signal: AbortSignal.timeout(30000) });
  const html = await response.text();
  await writeFile(`${proof}/customer-card-saved.html`, html);
  assert.equal(response.status, 200, html);
  saved = { page: parsePage(html), status: response.status };
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
