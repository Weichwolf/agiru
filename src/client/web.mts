import htmx from "htmx.org";
import { checkResponseProfile, commandEnvelope, parsePage, type Page } from "./profile.mjs";

type Request = {
  verb: string; path: string; elt: HTMLElement; headers: Record<string, string>;
  parameters: Record<string, string>;
};
type Response = {
  xhr: XMLHttpRequest; requestConfig: Request;
};

const root = document.querySelector<HTMLElement>("#workspace")!;
const status = document.querySelector<HTMLElement>("#status")!;
const login = document.querySelector<HTMLFormElement>("#connection")!;
const secret = document.querySelector<HTMLInputElement>("#credential")!;
let authorization = "";
let current: Page | undefined;
let active = false;
const candidates = new WeakMap<XMLHttpRequest, Page>();

htmx.config.allowEval = false;
htmx.config.allowScriptTags = false;
htmx.config.selfRequestsOnly = true;
htmx.config.historyCacheSize = 0;
htmx.config.timeout = 15000;
htmx.config.includeIndicatorStyles = false;

function path(value: string): string {
  if (!value.startsWith("/") || value.startsWith("//") || /[\\\x00-\x20\x7f]/.test(value)) throw new Error();
  const url = new URL(value, location.origin);
  if (url.origin !== location.origin || url.username || url.password || url.hash) throw new Error();
  return url.pathname + url.search;
}

function notice(response?: Response): void {
  const command = response?.requestConfig.parameters.command;
  status.textContent = response?.requestConfig.verb === "post"
    ? `Write outcome uncertain; reconcile command ${command ?? "(unknown)"} before retrying.`
    : "Page request refused; no successful outcome established.";
}

document.addEventListener("htmx:configRequest", event => {
  const request = (event as CustomEvent<Request>).detail;
  try {
    request.path = path(request.path);
    if (!authorization) throw new Error();
    request.headers.Authorization = authorization;
    request.headers.Accept = "text/html";
    if (request.verb === "get") {
      if (request.elt !== root) throw new Error();
      return;
    }
    if (request.verb !== "post" || !current || !(request.elt instanceof HTMLFormElement)) throw new Error();
    const form = request.elt;
    const selected = current.controls.find(control => control.operation?.command === request.parameters.command)?.operation;
    if (!selected) throw new Error();
    const envelope = commandEnvelope(current, selected, selected.operation === "set" ? request.parameters.text : undefined);
    if (request.path !== envelope.path || form.closest("article")?.dataset.handle !== current.handle ||
        form.closest("article")?.dataset.revision !== current.revision) throw new Error();
    const supplied = Object.entries(request.parameters);
    if (supplied.length !== Object.keys(envelope.fields).length ||
        supplied.some(([name, value]) => envelope.fields[name] !== value)) throw new Error();
  } catch {
    event.preventDefault();
    status.textContent = "Command refused; refresh and choose an advertised command.";
  }
});

document.addEventListener("htmx:beforeRequest", event => {
  if (active) { event.preventDefault(); return; }
  active = true;
  status.textContent = "Working…";
});

document.addEventListener("htmx:beforeOnLoad", event => {
  const response = (event as CustomEvent<Response>).detail;
  try {
    const xhr = response.xhr;
    if (xhr.status < 200 || xhr.status >= 300 ||
        xhr.responseURL !== new URL(response.requestConfig.path, location.origin).href) throw new Error();
    checkResponseProfile(xhr.getResponseHeader("Content-Type") ?? "", header => xhr.getResponseHeader(header) !== null);
    candidates.set(xhr, parsePage(xhr.responseText));
  } catch {
    event.preventDefault();
    notice(response);
  }
});

document.addEventListener("htmx:afterSwap", event => {
  const response = (event as CustomEvent<Response>).detail;
  const accepted = candidates.get(response.xhr);
  if (!accepted) { notice(response); return; }
  current = accepted;
  const url = new URL(location.href);
  url.search = new URLSearchParams({ handle: current.handle }).toString();
  history.replaceState(null, "", url);
  status.textContent = current.unsupported ? `${current.unsupported} unsupported controls remain visible.` : "Ready";
});

for (const name of ["htmx:sendError", "htmx:timeout", "htmx:responseError", "htmx:onLoadError"]) {
  document.addEventListener(name, event => {
    active = false;
    notice((event as CustomEvent<Response>).detail);
  });
}
document.addEventListener("htmx:afterRequest", () => { active = false; });

login.addEventListener("submit", event => {
  event.preventDefault();
  if (active) return;
  if (location.protocol !== "https:" && !["127.0.0.1", "localhost", "[::1]"].includes(location.hostname)) {
    status.textContent = "Use HTTPS or a loopback development origin.";
    return;
  }
  if (!/^ag1_[a-f0-9]{64}$/.test(secret.value)) {
    status.textContent = "A valid privately issued development credential is required.";
    return;
  }
  authorization = `Bearer ${secret.value}`;
  secret.value = "";
  login.hidden = true;
  try {
    const target = document.querySelector<HTMLInputElement>("#target")!.value;
    void htmx.ajax("get", path(target), { source: root, target: root, swap: "innerHTML" }).catch(() => notice());
  } catch { status.textContent = "Only same-origin root-relative page paths are allowed."; }
});

const target = document.querySelector<HTMLInputElement>("#target")!;
target.value = location.search ? `/${location.search}` : "/?page=22";
