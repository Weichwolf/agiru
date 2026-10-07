import assert from "node:assert/strict";
import { test } from "node:test";
import { execFile, spawn } from "node:child_process";
import { readFile, writeFile } from "node:fs/promises";
import { promisify } from "node:util";

const execute = promisify(execFile);
const container = process.env.AGIRU_DEV_CONTAINER ?? "agiru-dev";
const command = async args => (await execute("podman", ["exec", "--user", "agiru", container, ...args])).stdout;

test("actual Caddy serves local assets and browser documents while preserving native HX routing", async () => {
  await execute("bash", ["scripts/dev_container.sh", "web"]);
  const temporary = (await command(["mktemp", "-d", "/tmp/agiru-web-edge.XXXXXX"])).trim();
  assert.match(temporary, /^\/tmp\/agiru-web-edge\.[A-Za-z0-9]+$/);
  let pid, exited = false, diagnostic = "";
  const server = spawn("podman", ["exec", "--user", "agiru",
    "--env", "AGIRU_HTTP_ADDRESS=http://127.0.0.1:18081",
    "--env", `XDG_CONFIG_HOME=${temporary}/config`, "--env", `XDG_DATA_HOME=${temporary}/data`,
    container, "sh", "-c", 'printf "CADDY_PID %s\\n" "$$"; exec caddy run --config /workspace/deploy/dev/Caddyfile --adapter caddyfile'],
    { stdio: ["ignore", "pipe", "pipe"] });
  const closed = new Promise(resolve => server.once("close", code => { exited = true; resolve(code); }));
  try {
    await new Promise((resolve, reject) => {
      const timer = setTimeout(() => reject(new Error("Private Caddy did not start")), 10000);
      server.once("error", error => { clearTimeout(timer); reject(error); });
      server.once("close", code => { clearTimeout(timer); reject(new Error(`Private Caddy exited: ${code}`)); });
      server.stdout.on("data", chunk => {
        const match = String(chunk).match(/^CADDY_PID ([1-9][0-9]*)$/m);
        if (match) pid = match[1];
      });
      server.stderr.on("data", chunk => {
        diagnostic = (diagnostic + chunk).slice(-16384);
        if (diagnostic.includes("serving initial configuration")) { clearTimeout(timer); resolve(); }
      });
    });
    assert.ok(pid);
    const get = async (path, headers = []) => command(["curl", "--fail", "--silent", "--show-error", "--max-time", "5",
      ...headers, `http://127.0.0.1:18081${path}`]);
    const document = await readFile("build/web/index.html", "utf8");
    assert.equal(await get("/?page=22&company=CRONUS", ["-H", "Accept: text/html"]), document);
    const headers = await get("/", ["-I", "-H", "Accept: text/html"]);
    assert.match(headers, /content-security-policy:.*script-src .self./i);
    assert.match(headers, /referrer-policy: no-referrer/i);
    assert.doesNotMatch(headers, /^server:/im);
    for (const name of ["web.js", "web.css", "htmx-LICENSE.txt", "parse5-LICENSE.txt", "entities-LICENSE.txt"]) {
      assert.equal(await get(`/assets/${name}`), await readFile(`build/web/${name}`, "utf8"));
    }
    const native = await command(["curl", "--silent", "--show-error", "--max-time", "5",
      "-H", "Accept: text/html", "-H", "HX-Request: true", "http://127.0.0.1:18081/?page=22"]);
    assert.notEqual(native, document, "HX requests must not return the browser shell");
    const port = process.env.AGIRU_DEV_HTTP_PORT ?? "8080";
    const publicBundle = await fetch(`http://127.0.0.1:${port}/assets/web.js`);
    assert.equal(publicBundle.status, 200);
    assert.equal(await publicBundle.text(), await readFile("build/web/web.js", "utf8"));
  } finally {
    try {
      if (pid && !exited) await command(["kill", "-TERM", pid]);
      assert.equal(await closed, 0, diagnostic);
    } finally {
      await command(["rm", "-r", "--", temporary]);
      await writeFile(`${process.env.AGIRU_WEB_PROOF}/caddy.log`, diagnostic);
    }
  }
});
