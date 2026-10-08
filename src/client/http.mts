import { constants } from "node:fs";
import { open } from "node:fs/promises";
import { ClientError, ServerError } from "./errors.mjs";
import { checkResponseProfile, commandEnvelope, limits, parseFailure, parsePage, type Page } from "./profile.mjs";

export type Command = Readonly<{
  page: string; revision: string; command: string; control: string; operation: "set" | "action"; text?: string;
}>;
export type Result = Readonly<{ page: Page; status: number }>;
export type Headers = Readonly<{ authorization?: string; cookie?: string }>;

export async function readAuth(path: string): Promise<Headers> {
  let file;
  try {
    file = await open(path, constants.O_RDONLY | constants.O_NOFOLLOW | constants.O_NONBLOCK);
    const stat = await file.stat();
    if (!stat.isFile() || stat.uid !== process.getuid?.() || (stat.mode & 0o077) !== 0 || stat.size > 8192) {
      throw new ClientError("AuthFile", "Authentication file must be owned, private and at most 8 KiB");
    }
    const bytes = Buffer.alloc(8193);
    const { bytesRead } = await file.read(bytes, 0, bytes.length, 0);
    if (bytesRead > 8192) throw new ClientError("AuthFile", "Authentication file exceeds its bound");
    const value: unknown = JSON.parse(new TextDecoder("utf-8", { fatal: true }).decode(bytes.subarray(0, bytesRead)));
    if (value === null || typeof value !== "object" || Array.isArray(value)) throw new Error();
    const record = value as Record<string, unknown>;
    if (Object.keys(record).some(key => !["authorization", "cookie"].includes(key)) ||
      Object.values(record).some(item => typeof item !== "string" || !/^[\x20-\x7e]+$/.test(item))) throw new Error();
    return Object.freeze({ ...record }) as Headers;
  } catch (error) {
    if (error instanceof ClientError) throw error;
    throw new ClientError("AuthFile", "Cannot load private authentication file");
  } finally { await file?.close(); }
}

export class AgentClient {
  readonly #origin: URL;
  readonly #headers: Headers;
  readonly #timeout: number;

  constructor(origin: string, headers: Headers = {}, timeout = 15000) {
    try {
      const parsed = new URL(origin);
      if (!["http:", "https:"].includes(parsed.protocol) || parsed.username || parsed.password ||
        parsed.pathname !== "/" || parsed.search || parsed.hash) throw new Error();
      if (parsed.protocol === "http:" && !["127.0.0.1", "localhost", "[::1]"].includes(parsed.hostname)) throw new Error();
      this.#origin = parsed;
    } catch { throw new ClientError("OriginRefused", "Use an HTTPS origin or a loopback development HTTP origin"); }
    if (!Number.isSafeInteger(timeout) || timeout < 1 || timeout > 120000) {
      throw new ClientError("TimeoutRefused", "Timeout must be between 1 and 120000 milliseconds");
    }
    this.#headers = Object.freeze({ ...headers });
    this.#timeout = timeout;
  }

  #url(path: string): URL {
    try {
      if (!path.startsWith("/") || path.startsWith("//") || /[\\\x00-\x20\x7f]/.test(path)) throw new Error();
      const url = new URL(path, this.#origin);
      if (url.origin !== this.#origin.origin || url.hash || url.username || url.password) throw new Error();
      return url;
    } catch { throw new ClientError("PathRefused", "Only same-origin root-relative paths are allowed"); }
  }

  async #request(path: string, fields?: Readonly<Record<string, string>>, command?: string, timeout = this.#timeout,
                 originCommand?: string): Promise<Result> {
    const url = this.#url(path);
    const body = fields ? new URLSearchParams(fields).toString() : undefined;
    if (body && Buffer.byteLength(body) > limits.bytes) throw new ClientError("RequestLimit", "Command byte budget exceeded");
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), timeout);
    let reader: ReadableStreamDefaultReader<Uint8Array> | undefined;
    try {
      const response = await fetch(url, { method: body === undefined ? "GET" : "POST", redirect: "manual",
        headers: { ...this.#headers, Accept: "text/html", "HX-Request": "true",
          ...(body !== undefined ? { "Content-Type": "application/x-www-form-urlencoded", Origin: this.#origin.origin } : {}) },
        ...(body !== undefined ? { body } : {}), signal: controller.signal });
      reader = response.body?.getReader();
      if (response.status >= 300 && response.status < 400) throw new ClientError("RedirectRefused", "Redirects require explicit navigation");
      checkResponseProfile(response.headers.get("content-type") ?? "", header => response.headers.has(header));
      if (!reader) throw new ClientError("ResponseRefused", "Missing response body");
      const chunks: Uint8Array[] = [];
      let size = 0;
      for (;;) {
        const next = await reader.read();
        if (next.done) break;
        size += next.value.byteLength;
        if (size > limits.bytes) throw new ClientError("ResponseLimit", "Response byte budget exceeded");
        chunks.push(next.value);
      }
      const html = new TextDecoder("utf-8", { fatal: true }).decode(Buffer.concat(chunks, size));
      if (!response.ok) {
        const failure = parseFailure(html);
        const expected = failure.outcome === "refused" ? command : originCommand ?? command;
        if (command && failure.command !== expected && !(path.startsWith("/modal-commands/") && failure.command === command)) {
          throw new ClientError("ResponseRefused", "Error command identity does not match the submitted command");
        }
        throw new ServerError(failure.code, failure.message, failure.outcome, failure.command || undefined);
      }
      const page = parsePage(html);
      const result = Object.freeze({ page, status: response.status });
      if (Buffer.byteLength(JSON.stringify(result)) > 4194304) {
        throw new ClientError("ResponseLimit", "Structured output byte budget exceeded");
      }
      return result;
    } catch (error) {
      if (error instanceof ServerError) throw error;
      if (command) throw new ClientError("WriteUncertain", "Write outcome is uncertain; reconcile the command receipt before retrying", command);
      if (error instanceof ClientError) throw error;
      throw new ClientError("TransportFailure", "HTTP read failed or returned invalid UTF-8");
    } finally {
      clearTimeout(timer);
      await reader?.cancel().catch(() => {});
    }
  }

  async #follow(result: Result, deadline: number, command?: string, handle = result.page.handle,
                resolveFailure = false): Promise<Result> {
    const call = result.page.interaction?.call;
    for (;;) {
      const interaction = result.page.interaction;
      if (result.page.handle !== handle || (interaction && (interaction.call !== call ||
          interaction.originCommand !== (command ?? "")))) {
        throw new ClientError(command ? "WriteUncertain" : "ResponseRefused", "Operation identity changed while polling", command);
      }
      if (!interaction || (interaction.state !== "working" &&
          !(resolveFailure && interaction.poll?.state === "failed"))) return result;
      const remaining = deadline - Date.now();
      if (remaining <= 0) throw new ClientError(command ? "WriteUncertain" : "OperationPending",
        `AL operation is still pending; read /calls/${call} to reconcile without replaying it`, command);
      await new Promise(resolve => setTimeout(resolve, Math.min(50, remaining)));
      const poll = interaction.poll?.path ?? `/calls/${call}`;
      const input = interaction.poll ? poll.split("/").at(-1) : command;
      result = await this.#request(poll, undefined, input, Math.max(1, deadline - Date.now()), command);
    }
  }

  async read(path: string): Promise<Result> {
    const deadline = Date.now() + this.#timeout;
    const result = await this.#request(path);
    return this.#follow(result, deadline, result.page.interaction?.originCommand || undefined);
  }

  async execute(path: string, requested: Command): Promise<Result> {
    if (!/^[A-Za-z0-9_-]{1,128}$/.test(requested.page) || !/^[0-9]{1,128}$/.test(requested.revision) ||
      !/^[A-Za-z0-9_-]{1,128}$/.test(requested.command) || !requested.control ||
      !["set", "action"].includes(requested.operation) ||
      (requested.text !== undefined && (!requested.text.isWellFormed() || Buffer.byteLength(requested.text) > limits.bytes))) {
      throw new ClientError("CommandRefused", "Invalid explicit command context");
    }
    const location = this.#url(path);
    const retained = location.pathname === "/" && location.searchParams.size === 1 &&
      location.searchParams.get("handle") === requested.page;
    if (!retained) throw new ClientError("CommandRefused", "Execute requires the retained /?handle=<page> path; never reopen a page to submit a command");
    const current = await this.read(path);
    if (current.page.handle !== requested.page || current.page.revision !== requested.revision) {
      throw new ClientError("StalePage", "Page handle or revision changed; read and choose a new command explicitly");
    }
    const envelope = commandEnvelope(current.page, { ...requested, enabled: true }, requested.text);
    const originCommand = current.page.interaction?.originCommand ?? requested.command;
    const deadline = Date.now() + this.#timeout;
    const submit = async (): Promise<Result> => {
      return this.#request(envelope.path, envelope.fields, requested.command, this.#timeout, originCommand);
    };
    return this.#follow(await submit(), deadline, originCommand, requested.page, true);
  }
}

export async function configuredClient(): Promise<AgentClient> {
  const origin = process.env.AGIRU_ORIGIN ?? "http://127.0.0.1:8080";
  const auth = process.env.AGIRU_AUTH_FILE ? await readAuth(process.env.AGIRU_AUTH_FILE) : {};
  return new AgentClient(origin, auth);
}
