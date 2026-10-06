import assert from "node:assert/strict";
import { test, after } from "node:test";
import { spawn } from "node:child_process";
import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";
import { readFile } from "node:fs/promises";
import { connect } from "node:net";
import { AgentClient } from "../../build/client/http.mjs";
import { parsePage } from "../../build/client/profile.mjs";
const require = createRequire(new URL("../../src/client/package.json", import.meta.url));
const { Client } = await import(require.resolve("@modelcontextprotocol/sdk/client/index.js"));
const { StdioClientTransport } = await import(require.resolve("@modelcontextprotocol/sdk/client/stdio.js"));
const port = Number(process.env.AGIRU_DEV_HTTP_PORT ?? "8080");
const origin = `http://127.0.0.1:${port}`;
const page = parsePage(await readFile(process.env.AGIRU_CLIENT_HTML, "utf8"));
const native = spawn("make", ["--no-print-directory", "dev-exec",
  `COMMAND=/workspace/build/podman/gate_HttpServerGate --serve ${process.env.AGIRU_NATIVE_HTTP_HTML}`],
  { env: { ...process.env, AGIRU_DEV_INTERACTIVE: "1" }, stdio: ["pipe", "pipe", "pipe"] });
let output = "", diagnostic = "";
native.stdout.on("data", chunk => { output += chunk; });
native.stderr.on("data", chunk => { diagnostic += chunk; });
const closed = new Promise(resolve => native.on("close", code => resolve(code)));
const ready = new Promise((resolve, reject) => {
  const timer = setTimeout(() => reject(new Error("Native fixture did not start")), 15000);
  native.on("error", reject);
  native.once("close", code => { clearTimeout(timer); reject(new Error(`Native fixture exited ${code}: ${diagnostic}`)); });
  native.stdout.on("data", () => { if (output.includes("READY\n")) { clearTimeout(timer); resolve(); } });
});
await ready;
after(async () => {
  native.stdin.end("RQ");
  const timer = setTimeout(() => native.kill(), 15000);
  try {
    assert.equal(await closed, 0, diagnostic);
    assert.ok(diagnostic.includes("HTTP handler raised an unhandled exception"));
    assert.ok(!diagnostic.includes("cleanup failed"));
  }
  finally { clearTimeout(timer); }
});

function child(command, args, env = process.env) {
  const processValue = spawn(command, args, { env, stdio: ["ignore", "pipe", "pipe"] });
  return new Promise((resolve, reject) => {
    let stdout = "", stderr = "";
    processValue.stdout.on("data", chunk => { stdout += chunk; });
    processValue.stderr.on("data", chunk => { stderr += chunk; });
    processValue.on("error", reject);
    processValue.on("close", code => resolve({ code, stdout, stderr }));
  });
}
async function sql(expression) {
  const result = await child("make", ["--no-print-directory", "dev-exec",
    `COMMAND=psql -X -A -t -v ON_ERROR_STOP=1 postgresql://agiru:agiru@127.0.0.1:5432/agiru_gate -c '${expression.replaceAll("$", () => "$$")}'`]);
  assert.equal(result.code, 0, result.stderr);
  return result.stdout.trim();
}
function command(operation, control, text) {
  return { path: "/?page=50400&company=CRONUS%20CH", page: page.handle, revision: page.revision,
    command: page.controls.find(item => item.identity === control).operation.command,
    control, operation, ...(text === undefined ? {} : { text }) };
}
function raw(request) {
  return new Promise((resolve, reject) => {
    const socket = connect(port, "127.0.0.1");
    let response = "";
    const timer = setTimeout(() => { socket.destroy(); reject(new Error("Raw request timed out")); }, 5000);
    socket.on("connect", () => socket.write(request));
    socket.on("data", chunk => { response += chunk.toString("latin1"); });
    socket.on("error", error => { clearTimeout(timer); reject(error); });
    socket.on("close", () => { clearTimeout(timer); resolve(response); });
  });
}

test("external client reads actual C++-produced HTML through the native container listener", async () => {
  const result = await new AgentClient(origin).read("/?page=50400&company=CRONUS%20CH");
  assert.deepEqual(result.page, page);
  const response = await fetch(origin + "/");
  assert.equal(response.headers.get("cache-control"), "no-store");
  assert.equal(response.headers.get("x-content-type-options"), "nosniff");
});

test("nginx is the sole published listener; backend is loopback-only and static assets bypass ERP", async () => {
  const listeners = await child("make", ["--no-print-directory", "dev-exec", "COMMAND=ss -H -l -t -n"]);
  assert.equal(listeners.code, 0, listeners.stderr);
  assert.match(listeners.stdout, /127\.0\.0\.1:18080\s/);
  assert.ok(!listeners.stdout.includes("0.0.0.0:18080"));
  const published = await child("podman", ["port", process.env.AGIRU_DEV_CONTAINER ?? "agiru-dev"]);
  assert.equal(published.code, 0, published.stderr);
  assert.equal(published.stdout.trim(), `8080/tcp -> 127.0.0.1:${port}`);
  const before = await sql("SELECT count(*) FROM agiru_http_transport_fixture");
  const asset = await fetch(origin + "/assets/http-fixture.html");
  assert.equal(asset.status, 200);
  assert.match(asset.headers.get("content-type"), /^text\/html/);
  assert.equal(await asset.text(), await readFile(process.env.AGIRU_CLIENT_HTML, "utf8"));
  assert.equal((await fetch(origin + "/assets/missing-file")).status, 404);
  assert.equal(await sql("SELECT count(*) FROM agiru_http_transport_fixture"), before);
});

test("proxy preserves raw BC URI and replaces forged forwarding authority", async () => {
  const target = "/echo-target?company=CRONUS%20CH&page=21&filter=A%2FB%2B%25&filter=%E4%B8%AD%E6%96%87";
  assert.equal(await (await fetch(origin + target)).text(), target);
  const response = await fetch(origin + "/proxy-headers", { headers: {
    "X-Forwarded-Proto": "https", "X-Forwarded-Host": "attacker.invalid",
    "X-Forwarded-For": "attacker, another", "X-Real-IP": "attacker",
    "Forwarded": "for=attacker;proto=https", "Authorization": "Bearer transport-fixture",
    "Cookie": "fixture=preserved"
  } });
  assert.equal(response.status, 200);
  const headers = Object.fromEntries((await response.text()).trimEnd().split("\n").map(line => {
    const split = line.indexOf("="); return [line.slice(0, split), line.slice(split + 1)];
  }));
  assert.equal(headers.Host, new URL(origin).host);
  assert.equal(headers["X-Forwarded-Host"], new URL(origin).host);
  assert.equal(headers["X-Forwarded-Proto"], "http");
  assert.equal(headers.Forwarded, "");
  assert.ok(headers["X-Forwarded-For"] && !/[a-z,]/i.test(headers["X-Forwarded-For"]));
  assert.equal(headers["X-Real-IP"], headers["X-Forwarded-For"]);
  assert.equal(headers.Authorization, "Bearer transport-fixture");
  assert.equal(headers.Cookie, "fixture=preserved");
});

test("ambiguous HTTP framing is rejected before any ERP handler SQL effect", async () => {
  const before = await sql("SELECT count(*) FROM agiru_http_transport_fixture");
  const ambiguous = [
    "Content-Length: 1\r\nContent-Length: 2\r\n",
    "Content-Length: 4\r\nTransfer-Encoding: chunked\r\n",
    "Host: another\r\nContent-Length: 0\r\n"
  ];
  for (const headers of ambiguous) {
    assert.match(await raw(`POST /framing HTTP/1.1\r\nHost: localhost\r\n${headers}Connection: close\r\n\r\n0\r\n\r\n`), /^HTTP\/1.1 400 /);
  }
  assert.equal(await sql("SELECT count(*) FROM agiru_http_transport_fixture"), before);
});

test("shell CMD and MCP each submit one exact command; independent PostgreSQL records raw URI/body", async () => {
  const initial = Number(await sql("SELECT count(*) FROM agiru_http_transport_fixture WHERE method=$$POST$$"));
  const request = command("set", "Amount", "中文 + & = 1.2300\n🙂");
  const cmd = await child(process.execPath, [fileURLToPath(new URL("../../build/client/cmd.mjs", import.meta.url)),
    "--json", "execute", JSON.stringify(request)], { ...process.env, AGIRU_ORIGIN: origin, AGIRU_AUTH_FILE: "" });
  assert.equal(cmd.code, 0, cmd.stderr); assert.deepEqual(JSON.parse(cmd.stdout).page, page);
  const transport = new StdioClientTransport({ command: process.execPath,
    args: [fileURLToPath(new URL("../../build/client/mcp.mjs", import.meta.url))], env: { AGIRU_ORIGIN: origin } });
  const mcp = new Client({ name: "agiru-native-fixture", version: "1" });
  try {
    await mcp.connect(transport);
    const result = await mcp.callTool({ name: "agiru_execute", arguments: command("action", "Post") });
    assert.deepEqual(result.structuredContent.page, page);
  } finally { await mcp.close(); }
  assert.equal(Number(await sql("SELECT count(*) FROM agiru_http_transport_fixture WHERE method=$$POST$$")), initial + 2);
  const rows = JSON.parse(await sql("SELECT json_agg(t) FROM agiru_http_transport_fixture t WHERE method=$$POST$$"));
  const values = rows.map(row => Object.fromEntries(new URLSearchParams(row.body)));
  assert.equal(values[0].text, request.text);
  assert.equal(values[0].revision, "9007199254740993");
  assert.equal(values[0].csrf, "test-csrf");
  assert.equal(values[1].operation, "action");
  assert.ok(rows.every(row => row.target === "/commands"));
  assert.ok(await sql("SELECT count(*) FROM agiru_http_transport_fixture WHERE target=$$/?page=50400&company=CRONUS%20CH$$") !== "0");
});

test("binary transport preserves NUL/non-UTF8 bytes without treating them as AL scalars", async () => {
  const bytes = Buffer.from([0, 1, 27, 127, 128, 255]);
  const response = await fetch(origin + "/binary", { method: "POST", body: bytes });
  assert.equal(response.status, 200);
  assert.deepEqual(Buffer.from(await response.arrayBuffer()), bytes);
});

test("native body/response/header bounds refuse explicitly", async () => {
  const oversized = await fetch(origin + "/", { method: "POST", body: "x".repeat(4097) });
  assert.equal(oversized.status, 413);
  for (const path of ["/oversized-response", "/injected-header"]) {
    const result = await fetch(origin + path);
    assert.equal(result.status, 500); assert.equal(await result.text(), "HttpResponseRefused");
    assert.equal(result.headers.get("injected"), null);
  }
  const response = await raw("GET / HTTP/1.1\r\nHost: localhost\r\nAuthorization: first\r\nauthorization: second\r\nConnection: close\r\n\r\n");
  assert.match(response, /^HTTP\/1.1 400 /);
  const many = Array.from({ length: 65 }, (_, index) => `X-Test-${index}: value\r\n`).join("");
  assert.match(await raw(`GET / HTTP/1.1\r\nHost: localhost\r\n${many}Connection: close\r\n\r\n`), /^HTTP\/1.1 431 /);
});

test("unexpected handler errors produce counted diagnostics, not successful no-ops", async () => {
  const response = await fetch(origin + "/throw");
  assert.equal(response.status, 500); assert.equal(await response.text(), "HttpHandlerFailure");
});

async function buffered() {
  return new Promise((resolve, reject) => {
    const start = output.length;
    const timer = setTimeout(() => { native.stdout.off("data", inspect); reject(new Error("Missing body-byte receipt")); }, 3000);
    function inspect() {
      const found = output.slice(start).match(/BYTES ([0-9]+)\n/);
      if (found) { clearTimeout(timer); native.stdout.off("data", inspect); resolve(Number(found[1])); }
    }
    native.stdout.on("data", inspect);
    native.stdin.write("B");
  });
}

test("aggregate upload quota refuses another body and releases ownership after disconnect", async () => {
  const sockets = [connect(port, "127.0.0.1"), connect(port, "127.0.0.1")];
  try {
    await Promise.all(sockets.map(socket => new Promise((resolve, reject) => {
      socket.on("error", reject);
      socket.on("connect", () => socket.write("POST /binary HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4096\r\n\r\n" + "x".repeat(4095), resolve));
    })));
    const deadline = Date.now() + 3000;
    while (await buffered() !== 8190) {
      assert.ok(Date.now() < deadline, "stalled uploads must own exactly 8190 bytes");
      await new Promise(resolve => setTimeout(resolve, 10));
    }
    const rejected = await fetch(origin + "/binary", { method: "POST", body: "xyz" });
    assert.equal(rejected.status, 413);
  } finally { for (const socket of sockets) socket.destroy(); }
  const deadline = Date.now() + 3000;
  while (await buffered() !== 0) {
    assert.ok(Date.now() < deadline, "disconnect must release body quota");
    await new Promise(resolve => setTimeout(resolve, 10));
  }
});

test("idle browser/client think time retains no handler PostgreSQL connection", async () => {
  assert.equal(await sql("SELECT count(*) FROM pg_stat_activity WHERE datname=current_database() AND pid<>pg_backend_pid()"), "0");
});

test("two blocking jobs do not block network admission; bounded overflow gets 503 and shutdown drains", async () => {
  const held = [fetch(origin + "/hold"), fetch(origin + "/hold")];
  await new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error("Workers did not enter deterministic hold barrier")), 5000);
    const inspect = () => {
      if ((output.match(/HOLD\n/g) ?? []).length === 2) { clearTimeout(timer); resolve(); }
    };
    native.stdout.on("data", inspect);
    inspect();
  });
  const contenders = [fetch(origin + "/queued"), fetch(origin + "/queued")];
  let timer;
  try {
    const candidates = contenders.map(async promise => {
      const response = await promise;
      assert.equal(response.status, 503); return response;
    });
    candidates.push(new Promise((_, reject) => { timer = setTimeout(() => reject(new Error("Overflow was not refused while workers were blocked")), 5000); }));
    const rejected = await Promise.race(candidates);
    assert.equal(await rejected.text(), "HttpExecutorBusy");
  } finally { clearTimeout(timer); native.stdin.write("R"); }
  assert.ok((await Promise.all(held)).every(response => response.status === 200));
  const results = await Promise.all(contenders);
  assert.deepEqual(results.map(response => response.status).sort(), [200, 503]);
});
