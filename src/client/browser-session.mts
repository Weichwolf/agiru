export class BrowserSession {
  readonly cookies = location.protocol === "https:";
  #authorization = "";
  #csrf = "";

  get ready(): boolean { return this.cookies ? this.#csrf !== "" : this.#authorization !== ""; }

  clear(): void { this.#authorization = ""; this.#csrf = ""; }

  headers(): Record<string, string> {
    if (!this.ready) throw new Error("Session required");
    return this.cookies ? { "X-Agiru-Client": "browser", "X-Agiru-CSRF": this.#csrf } :
      { Authorization: this.#authorization };
  }

  async #request(path: string, method: "GET" | "POST", headers: Record<string, string> = {}): Promise<Response> {
    if (!this.cookies) throw new Error("HTTPS required");
    const response = await fetch(path, { method, headers: { Accept: "application/json",
      "X-Agiru-Client": "browser", ...(method === "POST" ? { "Content-Type": "application/json" } : {}),
      ...headers }, credentials: "same-origin", redirect: "error", cache: "no-store",
      signal: AbortSignal.timeout(15000) });
    if (response.url !== new URL(path, location.origin).href || response.redirected) throw new Error("Session response refused");
    return response;
  }

  async #accept(response: Response): Promise<void> {
    if (response.status !== 200 || response.headers.get("Content-Type") !== "application/json; charset=utf-8" ||
        response.headers.get("Cache-Control") !== "no-store") throw new Error("Session response refused");
    const reader = response.body?.getReader();
    if (!reader) throw new Error("Session response refused");
    let text = "";
    const decoder = new TextDecoder("utf-8", { fatal: true });
    let bytes = 0;
    try {
      while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        bytes += value.length;
        if (bytes > 1024) throw new Error("Session response refused");
        text += decoder.decode(value, { stream: true });
      }
      text += decoder.decode();
      const result: unknown = JSON.parse(text);
      if (!result || typeof result !== "object" || Array.isArray(result) || Object.keys(result).length !== 1 ||
          !("csrf" in result) || typeof result.csrf !== "string" || !/^[a-f0-9]{64}$/.test(result.csrf)) {
        throw new Error("Session response refused");
      }
      this.#csrf = result.csrf;
    } finally { await reader.cancel(); }
  }

  async bootstrap(): Promise<boolean> {
    if (!this.cookies) return false;
    this.clear();
    const response = await this.#request("/session", "GET");
    if (response.status === 401) { await response.body?.cancel(); return false; }
    await this.#accept(response);
    return true;
  }

  async connect(secret: string): Promise<void> {
    if (!/^ag1_[a-f0-9]{64}$/.test(secret)) throw new Error("Development credential required");
    if (this.cookies) {
      await this.#accept(await this.#request("/session", "POST", { Authorization: `Bearer ${secret}` }));
    } else {
      if (!["127.0.0.1", "localhost", "[::1]"].includes(location.hostname) || location.protocol !== "http:") {
        throw new Error("Loopback development origin required");
      }
      this.#authorization = `Bearer ${secret}`;
    }
  }

  async logout(): Promise<void> {
    try {
      if (this.cookies) {
        const response = await this.#request("/session/logout", "POST", this.headers());
        await response.body?.cancel();
        if (response.status !== 200) throw new Error("Logout not established");
      }
    } finally { this.clear(); }
  }
}
