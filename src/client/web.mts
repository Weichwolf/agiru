import htmx from "htmx.org";
import { checkResponseProfile, commandEnvelope, parseFailure, parsePage, type Page } from "./profile.mjs";

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
let retained: Readonly<{ page: Page; html: string }> | undefined;
let retainedModal: Readonly<{ page: Page; html: string }> | undefined;
let resolving = false;
let failureNotice = "";
let active = false;
let pollDeadline = 0;
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
  const command = response?.requestConfig.parameters.command || current?.interaction?.originCommand;
  status.textContent = response?.requestConfig.verb === "post" || command
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
      if (request.path.startsWith("/calls/") && request.path !== `/calls/${current?.interaction?.call}`) throw new Error();
      if (request.path.startsWith("/modal-commands/") && request.path !== current?.interaction?.poll?.path) throw new Error();
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
  if ((event as CustomEvent<Response>).detail.requestConfig.verb === "post") {
    resolving = true;
    failureNotice = "";
  }
  status.textContent = "Working…";
});

document.addEventListener("htmx:beforeOnLoad", event => {
  const response = (event as CustomEvent<Response>).detail;
  try {
    const xhr = response.xhr;
    if (xhr.status < 200 ||
        xhr.responseURL !== new URL(response.requestConfig.path, location.origin).href) throw new Error();
    checkResponseProfile(xhr.getResponseHeader("Content-Type") ?? "", header => xhr.getResponseHeader(header) !== null);
    if (xhr.status >= 300) {
      const failure = parseFailure(xhr.responseText);
      const command = response.requestConfig.verb === "post" ?
        (response.requestConfig.path === "/answers" && failure.outcome !== "refused" ? current?.interaction?.originCommand : response.requestConfig.parameters.command) :
        response.requestConfig.path.startsWith("/modal-commands/") ? response.requestConfig.path.split("/").at(-1) :
        response.requestConfig.path.startsWith("/calls/") ? current?.interaction?.originCommand : undefined;
      if (command && failure.command !== command &&
          !(response.requestConfig.path.startsWith("/modal-commands/") && failure.command === current?.interaction?.originCommand)) throw new Error();
      event.preventDefault();
      if (failure.outcome === "failed" && current?.interaction && retained?.page.handle === current.handle &&
          failure.command === current.interaction.originCommand) {
        root.innerHTML = retained.html;
        htmx.process(root);
        current = retained.page;
      } else if (failure.outcome === "failed" && current?.interaction && retainedModal?.page.handle === current.handle) {
        root.innerHTML = retainedModal.html;
        htmx.process(root);
        current = retainedModal.page;
        const handle = current.handle;
        setTimeout(() => {
          void htmx.ajax("get", `/?handle=${handle}`, { source: root, target: root, swap: "innerHTML" })
            .catch(() => notice());
        }, 50);
      }
      resolving = false;
      failureNotice = `Server error ${failure.code}: ${failure.message} outcome=${failure.outcome}` +
        (failure.command ? ` command=${failure.command}` : "") +
        (failure.outcome === "failed" ? "; prior explicit commits may persist." : "");
      status.textContent = failureNotice;
      return;
    }
    const page = parsePage(xhr.responseText);
    const interaction = page.interaction;
    if ((response.requestConfig.path.startsWith("/calls/") ||
         (response.requestConfig.verb === "get" && response.requestConfig.path.startsWith("/modal-commands/"))) &&
        (page.handle !== current?.handle || (interaction &&
          (interaction.call !== current?.interaction?.call || interaction.originCommand !== current?.interaction?.originCommand)))) throw new Error();
    if (response.requestConfig.verb === "post" && interaction &&
        (page.handle !== current?.handle || interaction.originCommand !==
          (response.requestConfig.path === "/answers" || response.requestConfig.path.startsWith("/modal-commands/")
            ? current?.interaction?.originCommand : response.requestConfig.parameters.command))) throw new Error();
    candidates.set(xhr, page);
  } catch {
    event.preventDefault();
    notice(response);
  }
});

document.addEventListener("htmx:afterSwap", event => {
  const response = (event as CustomEvent<Response>).detail;
  const accepted = candidates.get(response.xhr);
  if (!accepted) { notice(response); return; }
  if (accepted.interaction?.call !== current?.interaction?.call) pollDeadline = Date.now() + 15000;
  current = accepted;
  if (!current.interaction) retained = Object.freeze({ page: current, html: response.xhr.responseText });
  if (current.interaction?.state === "modal") retainedModal = Object.freeze({ page: current, html: response.xhr.responseText });
  const url = new URL(location.href);
  url.search = new URLSearchParams({ handle: current.handle }).toString();
  history.replaceState(null, "", url);
  if (current.interaction?.state === "working" || (resolving && current.interaction?.poll?.state === "failed")) {
    status.textContent = "Working…";
    setTimeout(() => {
      if (current !== accepted) return;
      if (Date.now() >= pollDeadline) {
        status.textContent = `AL operation pending; read /calls/${accepted.interaction!.call} before retrying.`;
        return;
      }
      void htmx.ajax("get", accepted.interaction!.poll?.path ?? `/calls/${accepted.interaction!.call}`, { source: root, target: root, swap: "innerHTML" })
        .catch(() => notice());
    }, 50);
  } else {
    resolving = false;
    status.textContent = current.interaction?.poll?.state === "failed" && failureNotice ? failureNotice :
      current.interaction ? "Explicit answer required." :
      current.unsupported ? `${current.unsupported} unsupported controls remain visible.` : "Ready";
  }
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
