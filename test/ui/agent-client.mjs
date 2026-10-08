import assert from "node:assert/strict";
import { test, after } from "node:test";
import { readFile, mkdtemp, chmod, symlink, rm } from "node:fs/promises";
import { createServer } from "node:http";
import { execFile, spawn } from "node:child_process";
import { createRequire } from "node:module";
import { promisify } from "node:util";
import { fileURLToPath, pathToFileURL } from "node:url";
const modules = process.env.AGIRU_CLIENT_MODULES ? pathToFileURL(process.env.AGIRU_CLIENT_MODULES + "/") : new URL("../../build/client/", import.meta.url);
const { parsePage, parseFailure, commandEnvelope, limits } = await import(new URL("profile.mjs", modules));
const { renderAscii, quote } = await import(new URL("ascii.mjs", modules));
const { AgentClient, readAuth } = await import(new URL("http.mjs", modules));
const { operate } = await import(new URL("command.mjs", modules));
const execute = promisify(execFile);
const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
const html = await readFile(process.env.AGIRU_CLIENT_HTML, "utf8");
const page = parsePage(html);
const received = [];
let posts = 0;
const failureHtml = (command = "cmd_1_7", outcome = "failed") =>
  `<article data-agiru-error="1" data-code="UiWriteTransaction" data-command="${command}" data-outcome="${outcome}"><h1>Request failed</h1><p>Grüezi &lt;script&gt; 東京 &amp; blocked.</p></article>`;
let failureMode = "";
const server = createServer(async (request, response) => {
  received.push({ method: request.method, path: request.url, headers: request.headers });
  if (request.url === "/redirect") { response.writeHead(302, { Location: "http://127.0.0.1:1/foreign" }); response.end(); return; }
  if (request.url === "/timeout") return;
  if (request.method === "POST") {
    posts++;
    let body = "";
    for await (const chunk of request) body += chunk;
    received.at(-1).body = Object.fromEntries(new URLSearchParams(body));
    if (request.url === "/disconnect") { request.socket.destroy(); return; }
    if (failureMode) {
      response.writeHead(500, { "Content-Type": "text/html; charset=utf-8" });
      response.end(failureMode === "malformed" ? "<p>unqualified failure</p>" :
        failureHtml(failureMode === "mismatched" ? "other_command" : "cmd_1_7", failureMode === "unknown" ? "unknown" : "failed"));
      return;
    }
  }
  response.setHeader("Content-Type", "text/html; charset=utf-8");
  if (request.url === "/wrong-type") response.setHeader("Content-Type", "application/json");
  if (request.url === "/hx-redirect") response.setHeader("HX-Redirect", "//foreign/");
  if (request.url === "/refused") response.statusCode = 403;
  if (request.url === "/invalid-utf8") { response.end(Buffer.from([0xc0, 0xaf])); return; }
  if (request.url === "/big") { response.end("x".repeat(limits.bytes + 1)); return; }
  if (request.url === "/large") { response.end(html.replace("HTML &lt;fixture&gt;", "x".repeat(270000))); return; }
  if (request.url === "/?handle=page_uncertain" || request.url === "/disconnect") {
    response.end(html.replaceAll('page_1', 'page_uncertain').replaceAll('action="/commands"', 'action="/disconnect"').replaceAll('hx-post="/commands"', 'hx-post="/disconnect"'));
  } else response.end(html);
});
await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
const origin = `http://127.0.0.1:${server.address().port}`;
const client = new AgentClient(origin);
after(async () => { server.closeAllConnections(); await new Promise(resolve => server.close(resolve)); });

function change(before, afterValue) {
  assert.ok(html.includes(before), `Required mutation anchor: ${before}`);
  return html.replace(before, afterValue);
}
function refuses(value) { assert.throws(() => parsePage(value), error => error.code === "ProfileRefused"); }
function request(control, operation, text) {
  const item = page.controls.find(value => value.identity === control);
  return { path: `/?handle=${page.handle}`, page: page.handle, revision: page.revision,
    command: item.operation.command, control, operation, ...(text === undefined ? {} : { text }) };
}
function cmd(args) {
  const child = spawn(process.execPath, [fileURLToPath(new URL("cmd.mjs", modules)), ...args],
    { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: "" }, stdio: ["ignore", "pipe", "pipe"] });
  return new Promise((resolve, reject) => {
    let stdout = "", stderr = "";
    child.stdout.on("data", chunk => { stdout += chunk; });
    child.stderr.on("data", chunk => { stderr += chunk; });
    child.on("error", reject);
    child.on("close", code => resolve({ code, stdout, stderr }));
  });
}

test("C++-rendered profile: exact scalars, order, entity decoding and counted gaps", () => {
  assert.equal(page.page, "50400");
  assert.equal(page.revision, "9007199254740993");
  assert.equal(page.caption, "HTML <fixture>");
  assert.deepEqual(page.controls.map(control => control.identity), ["General", "Amount", "Big", "Computed", "Lines", "Post", "Disabled"]);
  assert.equal(page.controls[1].scalar.value, "1.2300");
  assert.equal(page.controls[2].scalar.value, "9223372036854775807");
  assert.equal(page.controls[1].display, 'Grüezi <script> & "quoted"\r\n');
  assert.equal(page.unsupported, 2);
  assert.equal(page.controls.at(-1).operation.enabled, false);
  assert.ok(Object.isFrozen(page) && Object.isFrozen(page.controls) && Object.isFrozen(page.controls[1].scalar));
  assert.ok(!JSON.stringify(page).includes("test-csrf"));
});

test("ASCII is deterministic, lossless, non-TTY and safe for Unicode/terminal framing", () => {
  const text = renderAscii(page);
  assert.equal(text, renderAscii(page));
  assert.ok(text.includes('Decimal="1.2300"') && text.includes('BigInteger="9223372036854775807"'));
  assert.ok(text.includes('Grüezi <script> & \\"quoted\\"\\r\\n'));
  assert.ok(text.includes('action "Disabled" "Caption Disabled" disabled'));
  assert.ok(!text.includes("test-csrf") && !text.includes("\x1b"));
  assert.equal(JSON.parse(quote("中文🙂\u0085\u202e\u2066\n\t\\")), "中文🙂\u0085\u202e\u2066\n\t\\");
  assert.ok(!/[\u0080-\u009f\u202a-\u202e\u2066-\u2069]/u.test(quote("\u0085\u202e\u2066")));
  const framing = "\u061c\u200e\u200f\u2028\u2029";
  assert.equal(JSON.parse(quote(framing)), framing);
  assert.ok(!/[\u061c\u200e\u200f\u2028\u2029]/u.test(quote(framing)));
  assert.throws(() => renderAscii(page, Buffer.byteLength(text) - 1), error => error.code === "OutputLimit");
  assert.equal(renderAscii(page, Buffer.byteLength(text)), text);
});

test("Enum/temporal flags remain explicit and independent of captions", () => {
  const enumHtml = change('data-type="Decimal" data-value="1.2300" data-domain="" data-member=""',
    'data-type="Enum" data-value="42" data-domain="table/12/field/6" data-member="Open"');
  const entry = parsePage(enumHtml).controls[1];
  assert.deepEqual(entry.scalar, { type: "Enum", value: "42", domain: "table/12/field/6", member: "Open", undefined: false, closing: false });
  const temporal = parsePage(change('data-type="Decimal" data-value="1.2300"', 'data-type="Date" data-value="2026-10-06"')
    .replace('data-closing="false"', 'data-closing="true"'));
  assert.ok(renderAscii(temporal).includes('Date="2026-10-06" closing'));
  const undefinedValue = parsePage(change('data-undefined="false"', 'data-undefined="true"'));
  assert.equal(undefinedValue.controls[1].scalar.undefined, true);
});

const mutations = [
  ["unsupported profile", 'data-agiru-profile="1"', 'data-agiru-profile="2"'],
  ["wrong view", 'data-view="current-row"', 'data-view="list"'],
  ["duplicate attribute", 'data-revision="9007199254740993"', 'data-revision="9007199254740993" data-revision="1"'],
  ["fractional revision", 'data-revision="9007199254740993"', 'data-revision="1.0"'],
  ["foreign command origin", 'action="/commands"', 'action="//foreign/commands"'],
  ["htmx mismatch", 'hx-post="/commands"', 'hx-post="/other"'],
  ["missing operation", 'name="operation"', 'name="unknown"'],
  ["wrong page", 'name="page" value="page_1"', 'name="page" value="page_2"'],
  ["wrong revision", 'name="revision" value="9007199254740993"', 'name="revision" value="1"'],
  ["wrong control", 'name="control" value="Amount"', 'name="control" value="Caption Amount"'],
  ["empty CSRF", 'name="csrf" value="test-csrf"', 'name="csrf" value=""'],
  ["unknown scalar", 'data-type="Decimal"', 'data-type="Double"'],
  ["rounded scalar grammar", 'data-value="1.2300"', 'data-value="1,2300"'],
  ["invalid flags", 'data-undefined="false"', 'data-undefined="yes"'],
  ["missing count", 'data-unsupported-count="2"', 'data-unsupported-count="1"'],
  ["duplicate controls", 'data-control="Big"', 'data-control="Amount"'],
  ["duplicate command", 'value="cmd_1_7"', 'value="cmd_1_2"'],
  ["script", "</h1>", "<script>alert(1)</script></h1>"],
  ["injected handler", '<button type="submit">Set', '<button type="submit" onclick="alert(1)">Set'],
  ["htmx override on button", '<button type="submit">Set', '<button type="submit" hx-post="/other">Set'],
];
for (const [name, before, afterValue] of mutations) test(`profile negative control: ${name}`, () => refuses(change(before, afterValue)));

test("HTML budget, malformed nesting, comments, invalid text and extra roots refuse", () => {
  refuses(html + " ".repeat(limits.bytes));
  refuses(html + html);
  refuses(html.replace("</form>", ""));
  refuses(html.replace("</h1>", "<!--hidden--></h1>"));
  refuses(html.replace("</h1>", "\ud800</h1>"));
  refuses(html.replace("</h1>", "\x1b</h1>"));
  refuses(html.replace("</article>", ""));
  refuses(html.replace("</h1>", "&#x80;</h1>"));
  refuses(html.replace("</h1>", "&#27;</h1>"));
  const literal = parsePage(html.replace("</h1>", "\u0085\ufdd0\u202e</h1>"));
  assert.ok(literal.caption.endsWith("\u0085\ufdd0\u202e"));
  assert.ok(!/[\u0080-\u009f\u202a-\u202e]/u.test(renderAscii(literal)));
});

test("only advertised enabled commands submit; private fields retained without inference", () => {
  const selected = page.controls[1].operation;
  const envelope = commandEnvelope(page, selected, "2.3400");
  assert.deepEqual({ ...envelope.fields }, { page: "page_1", revision: "9007199254740993", command: "cmd_1_2",
    csrf: "test-csrf", operation: "set", control: "Amount", text: "2.3400" });
  assert.throws(() => commandEnvelope(page, page.controls.at(-1).operation), error => error.code === "CommandDisabled");
  assert.throws(() => commandEnvelope(page, { ...selected, command: "invented" }, "3"));
  assert.throws(() => commandEnvelope(page, selected));
  assert.throws(() => commandEnvelope(page, page.controls.at(-2).operation, "auto-confirm"));
});

test("actual HTTP read and shell CMD match the shared lossless/ASCII library", async () => {
  const result = await operate(client, "read", { path: "/?page=50400&company=CRONUS%20CH" });
  assert.deepEqual(result.page, page);
  const json = await cmd(["--json", "read", "/?page=50400&company=CRONUS%20CH"]);
  assert.equal(json.code, 0); assert.equal(json.stderr, "");
  assert.deepEqual(JSON.parse(json.stdout), result);
  const ascii = await cmd(["read", "/?page=50400&company=CRONUS%20CH"]);
  assert.equal(ascii.stdout, renderAscii(page)); assert.equal(ascii.stderr, "");
});

test("CMD set performs exactly one explicit form POST with unchanged text and tokens", async () => {
  const before = posts;
  const value = "中文 + & = 1.2300\n🙂";
  const result = await cmd(["--json", "execute", JSON.stringify(request("Amount", "set", value))]);
  assert.equal(result.code, 0); assert.equal(posts, before + 1);
  assert.deepEqual(JSON.parse(result.stdout).page, page);
  const sent = received.findLast(item => item.method === "POST");
  assert.equal(sent.path, "/commands");
  assert.equal(sent.body.text, value); assert.equal(sent.body.csrf, "test-csrf");
  assert.equal(sent.body.command, "cmd_1_2"); assert.equal(sent.body.revision, page.revision);
  assert.equal(sent.headers.origin, origin); assert.equal(sent.headers["hx-request"], "true");
});

test("stale/disabled/unknown actions and invalid schemas never POST", async () => {
  const before = posts;
  await assert.rejects(operate(client, "execute", { ...request("Post", "action"), revision: "1" }), error => error.code === "StalePage");
  await assert.rejects(operate(client, "execute", request("Disabled", "action")), error => error.code === "CommandDisabled");
  await assert.rejects(operate(client, "execute", { ...request("Post", "action"), control: "forged" }));
  await assert.rejects(operate(client, "execute", { ...request("Post", "action"), command: "forged" }));
  await assert.rejects(operate(client, "execute", { ...request("Post", "action"), text: "yes" }));
  await assert.rejects(operate(client, "execute", { ...request("Amount", "set", "2"), unknown: true }));
  await assert.rejects(operate(client, "execute", request("Amount", "set")));
  assert.equal(posts, before);
});

test("execute refuses opening or mismatched paths before any HTTP request", async () => {
  const before = received.length;
  for (const path of ["/?page=50400", "/?page=50400&company=CRONUS%20CH", "/",
    "/?handle=other", "/?handle=page_1&handle=page_1", "/?handle=page_1&page=50400",
    "/other?handle=page_1", "/?handle=page_1#fragment", "//foreign/?handle=page_1"]) {
    await assert.rejects(operate(client, "execute", { ...request("Post", "action"), path }),
      error => ["CommandRefused", "PathRefused"].includes(error.code));
  }
  const shell = await cmd(["execute", JSON.stringify({ ...request("Post", "action"), path: "/?page=50400" })]);
  assert.equal(shell.code, 2);
  assert.equal(JSON.parse(shell.stderr).error, "CommandRefused");
  assert.equal(received.length, before, "do not run OnOpenPage while checking an existing command");
});

test("HTTP refuses redirects, foreign paths, malformed UTF-8, status and body/type budgets", async () => {
  for (const path of ["/redirect", "/hx-redirect", "/wrong-type", "/invalid-utf8", "/big", "/refused",
    "//foreign/", "https://foreign/", "/\\foreign", "/#fragment"]) await assert.rejects(client.read(path));
  for (const value of ["http://remote.example", "file:///tmp", "https://user:secret@example.org", "https://example.org/path"]) {
    assert.throws(() => new AgentClient(value), error => error.code === "OriginRefused");
  }
  await assert.rejects(new AgentClient(origin, {}, 20).read("/timeout"), error => error.code === "TransportFailure");
});

test("uncertain write disconnect is never retried; command receipt identity retained", async () => {
  const before = posts;
  await assert.rejects(operate(client, "execute", { ...request("Post", "action"), path: "/?handle=page_uncertain", page: "page_uncertain" }),
    error => error.code === "WriteUncertain" && error.command === "cmd_1_7");
  assert.equal(posts, before + 1);
  const shell = await cmd(["execute", JSON.stringify({ ...request("Post", "action"), path: "/?handle=page_uncertain", page: "page_uncertain" })]);
  assert.equal(shell.code, 3); assert.equal(shell.stdout, "");
  assert.equal(JSON.parse(shell.stderr).command, "cmd_1_7");
  assert.equal(posts, before + 2);
});

test("typed server errors retain exact diagnostics and explicit outcomes; malformed or foreign command outcomes remain uncertain", async () => {
  const decoded = { code: "UiWriteTransaction", message: "Grüezi <script> 東京 & blocked.", command: "cmd_1_7", outcome: "failed" };
  assert.deepEqual(parseFailure(failureHtml()), decoded);
  for (const invalid of [failureHtml().replace('data-outcome="failed"', 'data-outcome="complete"'),
    failureHtml().replace('data-command="cmd_1_7"', 'data-command=""'), failureHtml() + failureHtml(),
    failureHtml().replace("</p>", "<script>sideEffect()</script></p>"),
    failureHtml().replace('data-code="UiWriteTransaction"', 'data-code="UiWriteTransaction" data-code="other"')]) {
    assert.throws(() => parseFailure(invalid), error => error.code === "ProfileRefused");
  }
  const before = posts;
  try {
    for (const mode of ["failed", "unknown", "malformed", "mismatched"]) {
      failureMode = mode;
      await assert.rejects(operate(client, "execute", request("Post", "action")), error =>
        mode === "failed" || mode === "unknown"
          ? error.code === decoded.code && error.message === decoded.message && error.command === decoded.command && error.outcome === mode
          : error.code === "WriteUncertain" && error.command === decoded.command);
      const shell = await cmd(["execute", JSON.stringify(request("Post", "action"))]);
      assert.equal(shell.code, mode === "failed" ? 2 : 3);
      assert.equal(shell.stdout, "");
      const value = JSON.parse(shell.stderr);
      assert.equal(value.command, decoded.command);
      if (mode === "failed" || mode === "unknown") assert.deepEqual(value,
        { error: decoded.code, message: decoded.message, command: decoded.command, outcome: mode });
      else assert.equal(value.error, "WriteUncertain");
    }
    assert.equal(posts, before + 8, "never retry any failed or uncertain submission automatically");
  } finally { failureMode = ""; }
});

test("MCP real stdio initialize/discover/read/set/action matches CMD and form effects", async () => {
  const transport = new StdioClientTransport({ command: process.execPath,
    args: [fileURLToPath(new URL("mcp.mjs", modules))],
    env: { AGIRU_ORIGIN: origin }, stderr: "pipe", maxBufferSize: 4194304 });
  const mcp = new Client({ name: "agiru-fixture-test", version: "1" });
  let diagnostic = "";
  transport.stderr.on("data", value => { diagnostic += value; });
  try {
    await mcp.connect(transport);
    const tools = await mcp.listTools();
    for (const tool of tools.tools) {
      assert.deepEqual(tool.annotations, { readOnlyHint: false, destructiveHint: true, idempotentHint: false },
        `${tool.name}: page opens can write in AL triggers; do not advertise safe automatic retries`);
    }
    assert.deepEqual(tools.tools.map(tool => tool.name), ["agiru_read", "agiru_execute"]);
    const result = await mcp.callTool({ name: "agiru_read", arguments: { path: "/?page=50400" } });
    assert.deepEqual(result.structuredContent, { status: 200, page });
    assert.equal(result.content[0].text, renderAscii(page));
    assert.equal(result.isError, undefined);
    const large = await mcp.callTool({ name: "agiru_read", arguments: { path: "/large" } });
    assert.equal(large.structuredContent.presentation.code, "OutputLimit");
    assert.equal(large.structuredContent.page.caption.length, 270000);
    assert.ok(large.content[0].text.includes("ASCII output budget exceeded"));
    const before = posts;
    for (const input of [request("Amount", "set", "中文🙂"), request("Post", "action")]) {
      const called = await mcp.callTool({ name: "agiru_execute", arguments: input });
      assert.deepEqual(called.structuredContent, { status: 200, page });
    }
    assert.equal(posts, before + 2);
    const invalid = await mcp.callTool({ name: "agiru_execute", arguments: { ...request("Post", "action"), autoConfirm: true } });
    assert.equal(invalid.isError, true); assert.equal(posts, before + 2);
    const disabled = await mcp.callTool({ name: "agiru_execute", arguments: request("Disabled", "action") });
    assert.equal(disabled.isError, true); assert.equal(disabled.structuredContent.error, "CommandDisabled");
    const beforeOpen = received.length;
    const reopened = await mcp.callTool({ name: "agiru_execute", arguments: { ...request("Post", "action"), path: "/?page=50400" } });
    assert.equal(reopened.isError, true); assert.equal(reopened.structuredContent.error, "CommandRefused");
    assert.equal(received.length, beforeOpen);
    const uncertain = await mcp.callTool({ name: "agiru_execute", arguments: { ...request("Post", "action"), path: "/?handle=page_uncertain", page: "page_uncertain" } });
    assert.equal(uncertain.isError, true); assert.equal(uncertain.structuredContent.error, "WriteUncertain");
    assert.equal(posts, before + 3); assert.equal(diagnostic, "");
  } finally { await mcp.close(); }
});

test("MCP refuses invalid UTF-8 and oversized stdio input without HTTP writes", async () => {
  const before = posts;
  for (const bytes of [Buffer.from([0xc0, 0xaf, 0x0a]), Buffer.from([0xf0, 0x9f]), Buffer.alloc(1048577, 0x20)]) {
    const child = spawn(process.execPath, [fileURLToPath(new URL("mcp.mjs", modules))],
      { env: { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: "" }, stdio: ["pipe", "pipe", "pipe"] });
    let stderr = "", stdout = "";
    child.stderr.on("data", chunk => { stderr += chunk; });
    child.stdout.on("data", chunk => { stdout += chunk; });
    const exited = new Promise((resolve, reject) => {
      const timer = setTimeout(() => { child.kill(); reject(new Error("MCP refusal did not terminate")); }, 4000);
      child.on("close", code => { clearTimeout(timer); resolve(code); });
      child.on("error", reject);
    });
    child.stdin.end(bytes);
    assert.equal(await exited, 2); assert.equal(stderr, "MCP input refused\n"); assert.equal(stdout, "");
  }
  assert.equal(posts, before);
});

test("output budgets disclose refused ASCII while preserving full CMD/MCP structured values", async () => {
  const output = await cmd(["--json", "read", "/large"]);
  assert.equal(output.code, 0);
  const result = JSON.parse(output.stdout);
  assert.deepEqual(result.presentation, { ascii: "refused", code: "OutputLimit" });
  assert.equal(result.page.caption.length, 270000);
  const ascii = await cmd(["read", "/large"]);
  assert.equal(ascii.code, 0);
  assert.ok(ascii.stdout.includes("ASCII output budget exceeded"));
  assert.ok(Buffer.byteLength(ascii.stdout) < 262144);
});

test("CLI refuses malformed/unknown arguments with stable stderr-only errors", async () => {
  for (const args of [["execute", "{"], ["unknown", "{}"], ["read"], ["--json", "read", "/", "unexpected"]]) {
    const result = await cmd(args);
    assert.equal(result.code, 2); assert.equal(result.stdout, "");
    assert.ok(JSON.parse(result.stderr).error.endsWith("Refused"));
  }
});

test("private auth files reject FIFOs without waiting for a writer", async () => {
  const dir = await mkdtemp("/tmp/agiru-agent-auth.");
  const fifo = `${dir}/auth.fifo`;
  try {
    await execute("mkfifo", ["--mode=600", fifo]);
    const source = `import { readAuth } from ${JSON.stringify(new URL("http.mjs", modules).href)};
      try { await readAuth(process.argv[1]); process.exitCode = 1; }
      catch (error) { if (error.code !== "AuthFile") throw error; console.log(error.code); }`;
    const result = await execute(process.execPath, ["--input-type=module", "--eval", source, fifo],
      { timeout: 3000, killSignal: "SIGKILL" });
    assert.equal(result.stdout, "AuthFile\n");
    assert.equal(result.stderr, "");
  } finally { await rm(dir, { recursive: true, force: true }); }
});

test("private auth files reject loose modes, symlinks and unknown headers", async () => {
  const dir = await mkdtemp("/tmp/agiru-agent-auth.");
  try {
    const { writeFile } = await import("node:fs/promises");
    const file = `${dir}/auth.json`;
    await writeFile(file, JSON.stringify({ authorization: "Bearer fixture-token", cookie: "session=fixture" }), { mode: 0o600 });
    assert.deepEqual(await readAuth(file), { authorization: "Bearer fixture-token", cookie: "session=fixture" });
    await chmod(file, 0o644);
    await assert.rejects(readAuth(file), error => error.code === "AuthFile");
    await chmod(file, 0o600); await symlink(file, `${dir}/link`);
    await assert.rejects(readAuth(`${dir}/link`), error => error.code === "AuthFile");
    await writeFile(file, JSON.stringify({ "x-override": "unsafe" }));
    await assert.rejects(readAuth(file), error => error.code === "AuthFile");
  } finally { await rm(dir, { recursive: true }); }
});
