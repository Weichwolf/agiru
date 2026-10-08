import assert from "node:assert/strict";
import { after, test } from "node:test";
import { execFile, spawn } from "node:child_process";
import { readFile } from "node:fs/promises";
import { request as httpsRequest, Agent } from "node:https";
import { promisify } from "node:util";

const execute = promisify(execFile);
const container = process.env.AGIRU_BROWSER_TLS_CONTAINER;
const proof = process.env.AGIRU_BROWSER_TLS_PROOF;
assert.match(container, /^agiru-browser-https-[0-9]+$/);
const address = (await readFile(`${proof}/port`, "utf8")).trim();
assert.match(address, /^127\.0\.0\.1:[0-9]+$/);
const origin = `https://localhost:${address.split(":")[1]}`;
const agent = new Agent({ ca: await readFile(`${proof}/ca.crt`), keepAlive: false });
const authPath = "/run/agiru/browser-https-auth.json";
const native = spawn("podman", ["exec", "--interactive", "--user", "agiru", container,
  "/workspace/build/podman/gate_BrowserHttpGate", "--serve", authPath, origin],
  { stdio: ["pipe", "pipe", "pipe"] });
let output = "", diagnostic = "";
native.stdout.on("data", chunk => { output += chunk; });
native.stderr.on("data", chunk => { diagnostic += chunk; });
const closed = new Promise(resolve => native.once("close", resolve));
await new Promise((resolve, reject) => {
  const timer = setTimeout(() => reject(new Error("Native cookie fixture did not start")), 15000);
  native.once("error", reject);
  native.once("close", code => { clearTimeout(timer); reject(new Error(`Native fixture exited ${code}`)); });
  native.stdout.on("data", () => { if (output.includes("READY\n")) { clearTimeout(timer); resolve(); } });
});
const auth = JSON.parse((await execute("podman", ["exec", "--user", "agiru", container, "sed", "-n", "1p", authPath])).stdout);
assert.ok(/^Bearer ag1_[a-f0-9]{64}$/.test(auth.authorization), "private fixture source is canonical");
const database = (await execute("podman", ["exec", "--user", "postgres", container, "psql", "-XAt",
  "-v", "ON_ERROR_STOP=1", "agiru_gate", "-c",
  "SELECT datname FROM pg_database WHERE datname LIKE 'agiru_owned_gate_%_browser_http'"])).stdout.trim();
assert.match(database, /^agiru_owned_gate_[0-9]+_browser_http$/);

after(async () => {
  native.stdin.end("Q");
  const timer = setTimeout(() => native.kill(), 15000);
  try {
    assert.equal(await closed, 0, "native fixture must cleanly terminate its owned database");
    const removed = await execute("podman", ["exec", "--user", "postgres", container, "psql", "-XAt",
      "-v", "ON_ERROR_STOP=1", "agiru_gate", "-c", `SELECT count(*) FROM pg_database WHERE datname='${database}'`]);
    assert.equal(removed.stdout.trim(), "0");
    assert.ok(!diagnostic.includes("cleanup failed"));
    await execute("podman", ["exec", "--user", "agiru", container, "unlink", authPath]);
  } finally { clearTimeout(timer); agent.destroy(); }
});

async function sql(statement) {
  const result = await execute("podman", ["exec", "--user", "postgres", container, "psql", "-XAt",
    "-v", "ON_ERROR_STOP=1", database, "-c", statement]);
  return result.stdout.trim();
}

function headers(overrides = {}) {
  return { Accept: "application/json", "Content-Type": "application/json", Origin: origin,
    "Sec-Fetch-Site": "same-origin", "Sec-Fetch-Mode": "cors", "Sec-Fetch-Dest": "empty",
    "X-Agiru-Client": "browser", ...overrides };
}

function request(path, method = "GET", supplied = {}, trusted = true) {
  return new Promise((resolve, reject) => {
    const outgoing = httpsRequest(new URL(path, origin), { method, headers: supplied,
      agent: trusted ? agent : undefined, family: 4, timeout: 5000 }, response => {
      let body = "";
      response.on("data", chunk => { body += chunk; });
      response.once("end", () => resolve({ status: response.statusCode, headers: response.headers, body }));
    });
    outgoing.once("error", reject);
    outgoing.once("timeout", () => outgoing.destroy(new Error("TLS request timed out")));
    outgoing.end();
  });
}

function grant(response) {
  assert.equal(response.status, 200);
  assert.equal(response.headers["cache-control"], "no-store");
  assert.equal(response.headers["x-content-type-options"], "nosniff");
  assert.equal(response.headers["set-cookie"]?.length, 1);
  const cookie = response.headers["set-cookie"][0];
  assert.ok(/^__Host-agiru=agb1_[a-f0-9]{64}; Secure; HttpOnly; SameSite=Strict; Path=\/$/.test(cookie),
    "only one exact protected host cookie is permitted");
  const csrf = JSON.parse(response.body).csrf;
  assert.ok(/^[a-f0-9]{64}$/.test(csrf), "CSRF bootstrap value is canonical");
  assert.ok(!response.body.includes(cookie.split(";")[0].split("=")[1]), "cookie secret never appears in response body");
  assert.ok(!response.body.includes(auth.authorization.slice(7)), "source bearer never appears in response body");
  return { cookie: cookie.split(";")[0], csrf };
}

let current, original;
function browser(overrides = {}) {
  assert.ok(current, "a successful prior exchange is required");
  return headers({ Cookie: current.cookie, "X-Agiru-CSRF": current.csrf, ...overrides });
}

test("TLS certificate validation fails without the private test CA", async () => {
  await assert.rejects(request("/session", "GET", headers(), false), error =>
    ["UNABLE_TO_VERIFY_LEAF_SIGNATURE", "UNABLE_TO_GET_ISSUER_CERT_LOCALLY", "SELF_SIGNED_CERT_IN_CHAIN"].includes(error.code));
});

test("Caddy overwrites forged forwarding authority and exchanges the source over trusted TLS", async () => {
  current = grant(await request("/session", "POST", headers({ Authorization: auth.authorization,
    "X-Forwarded-Proto": "http", "X-Forwarded-Host": "forged.invalid", Forwarded: "proto=http;host=forged.invalid" })));
  original = current;
  assert.equal(await sql("SELECT count(*) FROM agiru_client.browser_sessions"), "1");
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "0");
  assert.equal(await sql("SELECT bool_and(length(digest)=64 AND length(csrf_digest)=64) FROM agiru_client.browser_sessions"), "t");
});

test("session-bound CSRF permits the protected SQL probe on the private native listener", async () => {
  const response = await request("/protected", "GET", browser());
  assert.equal(response.status, 200);
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "1");
});

test("cross-site missing-CSRF navigation and prefetch denials have no independent SQL effects", async () => {
  for (const overrides of [{ "X-Agiru-CSRF": "" }, { "X-Agiru-CSRF": "forged" },
    { "Sec-Fetch-Site": "cross-site" }, { "Sec-Fetch-Site": "same-site" }, { "Sec-Fetch-Site": "" },
    { "Sec-Fetch-Mode": "navigate", "Sec-Fetch-Dest": "document" },
    { Purpose: "prefetch" }, { Origin: "https://elsewhere.invalid" }]) {
    assert.equal((await request("/protected", "GET", browser(overrides))).status, 403);
  }
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "1");
});

test("cookie-bearer ambiguity and repeated cookies cannot switch the authenticated client", async () => {
  assert.equal((await request("/protected", "GET", browser({ Authorization: auth.authorization }))).status, 401);
  assert.equal((await request("/protected", "GET", browser({ Cookie: `${current.cookie}; ${current.cookie}` }))).status, 401);
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "1");
});

test("passive CSRF bootstrap neither renews idle expiry nor creates SQL probe effects", async () => {
  const before = await sql("SELECT last_activity_at::text||'|'||idle_expires_at::text FROM agiru_client.browser_sessions WHERE revoked_at IS NULL");
  const response = await request("/session", "GET", browser({ "X-Agiru-CSRF": "", Origin: "" }));
  assert.equal(response.status, 200);
  assert.ok(JSON.parse(response.body).csrf === current.csrf, "bootstrap recovers the same synchronizer");
  assert.equal(response.headers["set-cookie"], undefined);
  assert.equal(await sql("SELECT last_activity_at::text||'|'||idle_expires_at::text FROM agiru_client.browser_sessions WHERE revoked_at IS NULL"), before);
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "1");
});

test("rotation durably replaces identity without extending absolute expiry or granting old-cookie reuse", async () => {
  current = grant(await request("/session/rotate", "POST", browser()));
  assert.ok(current.cookie !== original.cookie && current.csrf !== original.csrf, "both secrets rotate");
  assert.equal((await request("/protected", "GET", headers({ Cookie: original.cookie, "X-Agiru-CSRF": original.csrf }))).status, 401);
  assert.equal(await sql("SELECT count(*)::text||'|'||count(revoked_at)::text||'|'||count(DISTINCT expires_at)::text FROM agiru_client.browser_sessions"), "2|1|1");
  assert.equal((await request("/protected", "GET", browser())).status, 200);
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "2");
});

test("logout deletes the protected cookie and revoked SQL identity cannot write again", async () => {
  const response = await request("/session/logout", "POST", browser());
  assert.equal(response.status, 200);
  assert.ok(response.headers["set-cookie"]?.[0] === "__Host-agiru=; Secure; HttpOnly; SameSite=Strict; Path=/; Max-Age=0");
  assert.equal((await request("/protected", "GET", browser())).status, 401);
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "2");
});

test("an expired cookie is not adopted during fresh exchange and source expiry remains authoritative", async () => {
  current = grant(await request("/session", "POST", headers({ Cookie: current.cookie, Authorization: auth.authorization })));
  await sql("UPDATE agiru_client.credentials SET issued_at=statement_timestamp()-interval '2 hours',expires_at=statement_timestamp()-interval '1 hour'");
  assert.equal((await request("/protected", "GET", browser())).status, 401);
  assert.equal(await sql("SELECT count(*) FROM cookie_probe"), "2");
});
