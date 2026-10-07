import assert from "node:assert/strict";
import { execFile } from "node:child_process";
import { chmod, readFile, unlink, writeFile } from "node:fs/promises";
import { promisify } from "node:util";

const execute = promisify(execFile);
const defaults = JSON.parse(await readFile(new URL("../../deploy/dev/agiru.json", import.meta.url), "utf8"));

export class ServerConfigs {
  #files = [];
  constructor(container, native, proof) {
    this.container = container;
    this.native = native;
    this.proof = proof;
  }
  async write(overrides, name = "server.json") {
    assert.match(name, /^[a-z][a-z0-9-]*\.json$/);
    const config = { ...defaults, ...overrides };
    for (const section of ["http", "pages", "transactions"]) {
      config[section] = { ...defaults[section], ...overrides[section] };
    }
    const host = `${this.proof}/${name}`;
    const native = `${this.native}/${name}`;
    await writeFile(host, JSON.stringify(config), { flag: "wx", mode: 0o600 });
    this.#files.push({ host, native });
    await execute("podman", ["cp", host, `${this.container}:${native}`]);
    await execute("podman", ["exec", "--user", "0", this.container, "chown", "agiru:agiru", native]);
    await execute("podman", ["exec", "--user", "agiru", this.container, "chmod", "600", native]);
    await chmod(host, 0o600);
    return native;
  }
  async clean() {
    for (const { host, native } of this.#files) {
      await execute("podman", ["exec", "--user", "agiru", this.container, "unlink", native]);
      await unlink(host);
    }
    this.#files = [];
  }
}
