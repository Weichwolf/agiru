import { parseFragment, type DefaultTreeAdapterTypes as Tree } from "parse5";
import { ClientError, requireContract as check } from "./errors.mjs";

export const limits = Object.freeze({ bytes: 1048576, nodes: 65536, controls: 4096, depth: 64 });
export type Scalar = Readonly<{
  type: string; value: string; domain: string; member: string; undefined: boolean; closing: boolean;
}>;
export type Operation = Readonly<{
  operation: "set" | "action"; control: string; command: string; enabled: boolean;
}>;
export type Control = Readonly<{
  identity: string; kind: "group" | "field" | "action" | "label" | "unsupported";
  caption: string; depth: number; display?: string; scalar?: Scalar; operation?: Operation; reason?: string;
}>;
export type Page = Readonly<{
  profile: "1" | "2" | "3" | "4"; view: "current-row" | "list" | "interaction"; page: string; handle: string; revision: string;
  caption: string; controls: readonly Control[]; unsupported: number;
  rows?: readonly Row[]; window?: Readonly<{ limit: string; more: boolean; direction: "forward" | "backward" }>;
  messages?: readonly Readonly<{ handle: string; text: string }>[];
  interaction?: Readonly<{ state: "working" | "confirm" | "menu" | "modal"; call: string; originCommand: string;
    dialog?: string; defaultChoice?: string; prompt?: string;
    poll?: Readonly<{ path: string; state: "pending" | "failed" }> }>;
}>;
export type Row = Readonly<{ handle: string; selected: boolean; caption: string;
  controls: readonly Control[]; select: Operation }>;
type Envelope = Readonly<{ path: string; fields: Readonly<Record<string, string>> }>;
type Header = Pick<Page, "handle" | "revision"> & Readonly<{ modalPath?: string }>;
const envelopes = new WeakMap<Page, ReadonlyMap<string, Envelope>>();
const token = /^[A-Za-z0-9_-]{1,128}$/;
const digits = /^[0-9]+$/;
const scalarNames = ["Text", "Code", "Integer", "BigInteger", "Decimal", "Boolean", "Option", "Enum",
  "Date", "Time", "DateTime", "Duration", "Guid", "DateFormula", "RecordId"];

export type Failure = Readonly<{ code: string; command: string; outcome: "refused" | "failed" | "unknown"; message: string }>;

export function parseFailure(html: string): Failure {
  check(new TextEncoder().encode(html).byteLength <= limits.bytes && html.isWellFormed() &&
    !/[\u0000-\u0008\u000B\u000C\u000E-\u001F\u007F]/u.test(html), "Invalid error HTML text");
  const fragment = parseFragment(html, { sourceCodeLocationInfo: true, onParseError: error => {
    if (error.code === "control-character-reference" && html.slice(error.startOffset - 5, error.startOffset) === "&#13;") return;
    if (error.code === "control-character-in-input-stream" && /^[\u0080-\u009f]$/u.test(html[error.startOffset]!)) return;
    if (error.code === "noncharacter-in-input-stream") return;
    throw new ClientError("ProfileRefused", "Malformed error HTML");
  } });
  const roots = children(fragment);
  check(roots.length === 1 && roots[0]!.tagName === "article", "Expected one error article");
  const root = roots[0]!;
  const expected = ["data-agiru-error", "data-code", "data-command", "data-outcome"];
  check(root.attrs.length === expected.length && root.attrs.every(item => expected.includes(item.name)) &&
    attr(root, "data-agiru-error") === "1" && root.sourceCodeLocation?.endTag, "Invalid error envelope");
  const items = children(root);
  check(items.length === 2 && items[0]!.tagName === "h1" && items[1]!.tagName === "p" &&
    items.every(item => item.attrs.length === 0 && item.sourceCodeLocation?.endTag), "Invalid error contents");
  check(text(items[0]!) === "Request failed", "Invalid error heading");
  const code = attr(root, "data-code"), command = attr(root, "data-command"), outcome = attr(root, "data-outcome");
  check(code.length > 0 && code.length <= 128 && (!command || token.test(command)) &&
    ["refused", "failed", "unknown"].includes(outcome) && (outcome !== "failed" || command), "Invalid error identity or outcome");
  return Object.freeze({ code, command, outcome: outcome as Failure["outcome"], message: text(items[1]!) });
}
const responseEffects = ["hx-redirect", "hx-location", "hx-refresh", "hx-trigger", "hx-trigger-after-settle",
  "hx-trigger-after-swap", "hx-retarget", "hx-reswap", "hx-reselect", "hx-push-url", "hx-replace-url"];

export function checkResponseProfile(contentType: string, hasHeader: (name: string) => boolean): void {
  if (!/^text\/html(?:\s*;\s*charset=utf-8)?\s*$/i.test(contentType)) {
    throw new ClientError("ResponseRefused", "Expected UTF-8 semantic HTML");
  }
  if (responseEffects.some(hasHeader)) {
    throw new ClientError("ResponseRefused", "Undeclared htmx response effects are not supported by this profile");
  }
}
const tags = new Set(["article", "h1", "h2", "h3", "section", "form", "input", "button", "output", "aside", "p"]);
const scalarAttributes = ["data-type", "data-value", "data-domain", "data-member", "data-undefined", "data-closing"];
const attributes: Readonly<Record<string, readonly string[]>> = {
  article: ["data-agiru-profile", "data-view", "data-page", "data-handle", "data-revision", "data-limit", "data-more", "data-direction",
    "data-state", "data-call", "data-origin-command", "data-dialog", "data-default", "data-poll", "data-poll-state"],
  h1: [], h2: [], h3: [], section: ["data-control", "data-kind", "data-row", "data-selected"],
  form: ["method", "action", "hx-post", "hx-target", "hx-swap"],
  input: ["type", "name", "value", "aria-label", ...scalarAttributes],
  button: ["type", "disabled"], output: ["data-unsupported-count", ...scalarAttributes],
  aside: ["role", "data-control", "data-unsupported"], p: ["data-control", "data-message", "data-prompt"],
};

function element(node: Tree.ChildNode): Tree.Element {
  check("tagName" in node, "Expected a profile element");
  return node;
}

function attr(node: Tree.Element, name: string): string {
  const value = node.attrs.find(attribute => attribute.name === name);
  check(value, `Missing ${name}`);
  return value.value;
}

function has(node: Tree.Element, name: string): boolean {
  return node.attrs.some(attribute => attribute.name === name);
}

function children(node: Tree.Element | Tree.DocumentFragment): Tree.Element[] {
  return node.childNodes.filter(child => !(child.nodeName === "#text" && "value" in child && /^\s*$/.test(child.value)))
    .map(element);
}

function text(node: Tree.Element): string {
  return node.childNodes.map(child => {
    check(child.nodeName === "#text" && "value" in child, "Business text cannot contain markup");
    return child.value;
  }).join("");
}

function one(node: Tree.Element, tag: string): Tree.Element {
  const selected = children(node).filter(child => child.tagName === tag);
  check(selected.length === 1, `Expected one ${tag}`);
  return selected[0]!;
}

function exactFlag(node: Tree.Element, name: string): boolean {
  const value = attr(node, name);
  check(value === "true" || value === "false", `Invalid ${name}`);
  return value === "true";
}

function scalar(node: Tree.Element): Scalar {
  const result = Object.freeze({ type: attr(node, "data-type"), value: attr(node, "data-value"),
    domain: attr(node, "data-domain"), member: attr(node, "data-member"),
    undefined: exactFlag(node, "data-undefined"), closing: exactFlag(node, "data-closing") });
  check(scalarNames.includes(result.type), "Unknown scalar type");
  if (["Integer", "BigInteger", "Option", "Enum", "Duration"].includes(result.type)) {
    check(/^-?[0-9]{1,20}$/.test(result.value), "Invalid integer scalar");
  }
  if (result.type === "Decimal") check(/^-?[0-9]{1,29}(?:\.[0-9]{1,28})?$/.test(result.value), "Invalid Decimal scalar");
  if (result.type === "Boolean") check(result.value === "true" || result.value === "false", "Invalid Boolean scalar");
  if (result.type === "Enum" || result.type === "Option") check(result.domain.length > 0, "Missing enum domain");
  return result;
}

function inspectTree(fragment: Tree.DocumentFragment): void {
  const pending: Array<{ node: Tree.ChildNode; depth: number }> = fragment.childNodes.map(node => ({ node, depth: 0 }));
  let count = 0;
  while (pending.length) {
    const { node, depth } = pending.pop()!;
    check(++count <= limits.nodes && depth <= limits.depth + 8, "HTML tree budget exceeded");
    if (node.nodeName === "#text" && "value" in node) continue;
    const current = element(node);
    check(tags.has(current.tagName) && current.namespaceURI === "http://www.w3.org/1999/xhtml", "Unknown profile element");
    check(current.sourceCodeLocation && (current.tagName === "input" || current.sourceCodeLocation.endTag), "Missing explicit closing tag");
    check(current.attrs.every(attribute => attributes[current.tagName]!.includes(attribute.name)), "Unknown or misplaced profile attribute");
    for (const child of current.childNodes) pending.push({ node: child, depth: depth + 1 });
  }
}

function form(node: Tree.Element, page: Header, identity: string,
              operation: "set" | "action", commands: Map<string, Envelope>): Operation {
  check(attr(node, "method") === "post", "Commands must use POST");
  const path = attr(node, "action");
  check(/^\/(?!\/)[A-Za-z0-9_/-]*$/.test(path) && !path.split("/").some(part => part === "." || part === ".."), "Unsafe command path");
  check(page.modalPath ? path === page.modalPath : !path.startsWith("/modal-commands/"), "Modal command identity mismatch");
  check(attr(node, "hx-post") === path && attr(node, "hx-target") === "closest article" &&
    attr(node, "hx-swap") === "outerHTML", "Web and agent command contracts differ");
  const fields: Record<string, string> = Object.create(null) as Record<string, string>;
  const items = children(node);
  for (const input of items.filter(child => child.tagName === "input" && attr(child, "type") === "hidden")) {
    check(children(input).length === 0, "Input cannot have children");
    const name = attr(input, "name");
    check(["page", "revision", "command", "csrf", "operation", "control"].includes(name) && !(name in fields), "Unknown or duplicate command field");
    fields[name] = attr(input, "value");
  }
  check(Object.keys(fields).length === 6 && fields.page === page.handle && fields.revision === page.revision &&
    fields.control === identity && fields.operation === operation && fields.csrf!.length > 0 &&
    token.test(fields.command!), "Command envelope does not match the page");
  check(!commands.has(fields.command!), "Duplicate command identity");
  const button = one(node, "button");
  check(attr(button, "type") === "submit", "Invalid command button");
  text(button);
  check(items.length === (operation === "set" ? 8 : 7), "Unknown form content");
  if (operation === "set") {
    const inputs = items.filter(child => child.tagName === "input" && attr(child, "type") === "text");
    check(inputs.length === 1 && attr(inputs[0]!, "name") === "text", "Missing field text input");
  }
  commands.set(fields.command!, Object.freeze({ path, fields: Object.freeze(fields) }));
  return Object.freeze({ operation, control: identity, command: fields.command!, enabled: !has(button, "disabled") });
}

function controls(nodes: readonly Tree.Element[], page: Header, commands: Map<string, Envelope>): Control[] {
  const result: Control[] = [];
  const identities = new Set<string>();
  function visit(node: Tree.Element, depth: number): void {
    check(!has(node, "data-row") && !has(node, "data-selected"), "Misplaced row metadata");
    check(result.length < limits.controls && depth <= limits.depth, "Control budget exceeded");
    const identity = attr(node, "data-control");
    check(identity.length > 0 && !identities.has(identity), "Empty or duplicate control identity");
    identities.add(identity);
    if (node.tagName === "aside") {
      check(attr(node, "role") === "alert", "Unsupported controls must be alerts");
      result.push(Object.freeze({ identity, depth, kind: "unsupported", caption: text(node), reason: attr(node, "data-unsupported") }));
      return;
    }
    if (node.tagName === "p") {
      result.push(Object.freeze({ identity, depth, kind: "label", caption: text(node) }));
      return;
    }
    check(node.tagName === "section", "Unknown control structure");
    const kind = attr(node, "data-kind");
    if (kind === "group") {
      const items = children(node);
      check(items[0]?.tagName === "h2", "Missing group heading");
      result.push(Object.freeze({ identity, depth, kind, caption: text(items[0]!) }));
      for (const child of items.slice(1)) visit(child, depth + 1);
    } else if (kind === "field") {
      check(children(node).length === 2, "Invalid field structure");
      const caption = text(one(node, "h3"));
      const contents = children(node).filter(child => child.tagName !== "h3")[0]!;
      let value: Tree.Element;
      let operation: Operation | undefined;
      if (contents.tagName === "form") {
        operation = form(contents, page, identity, "set", commands);
        value = children(contents).find(child => child.tagName === "input" && attr(child, "type") === "text")!;
        check(attr(value, "aria-label") === caption, "Field labels disagree");
      } else {
        check(contents.tagName === "output", "Missing field value");
        value = contents;
      }
      const display = value.tagName === "input" ? attr(value, "value") : text(value);
      result.push(Object.freeze({ identity, depth, kind, caption, display, scalar: scalar(value), ...(operation ? { operation } : {}) }));
    } else if (kind === "action") {
      check(children(node).length === 1, "Invalid action structure");
      const contents = one(node, "form");
      const operation = form(contents, page, identity, "action", commands);
      result.push(Object.freeze({ identity, depth, kind, caption: text(one(contents, "button")), operation }));
    } else check(false, "Unknown control kind");
  }
  for (const child of nodes) visit(child, 0);
  return result;
}

function listRows(node: Tree.Element, page: Header,
                  commands: Map<string, Envelope>, limit: string): Row[] {
  check(node.tagName === "section" && attr(node, "data-kind") === "rows" &&
    attr(node, "data-control") === "$agiru.rows" && node.attrs.length === 2, "Invalid list container");
  const seen = new Set<string>();
  const result = children(node).map(row => {
    check(row.tagName === "section" && attr(row, "data-kind") === "row" && row.attrs.length === 3, "Invalid list row");
    const handle = attr(row, "data-row");
    check(token.test(handle) && !seen.has(handle), "Invalid or repeated row handle");
    seen.add(handle);
    const items = children(row);
    check(items[0]?.tagName === "h2", "Missing row heading");
    const entries = controls(items.slice(1), page, commands);
    const action = entries.at(-1);
    check(action?.kind === "action" && action.identity === `$agiru.row_${handle}` &&
      action.operation?.operation === "action", "Missing row selection command");
    const cells = entries.slice(0, -1);
    check(cells.every(cell => !cell.operation && cell.kind !== "action"), "Rows must be read-only projections");
    return Object.freeze({ handle, selected: exactFlag(row, "data-selected"), caption: text(items[0]!),
      controls: Object.freeze(cells), select: action.operation });
  });
  check(BigInt(result.length) <= BigInt(limit) && result.length <= limits.controls, "List row bound exceeded");
  check(result.filter(row => row.selected).length === (result.length ? 1 : 0), "Invalid selected row census");
  return result;
}

function messages(nodes: readonly Tree.Element[]): NonNullable<Page["messages"]> {
  const seen = new Set<string>();
  const entries = nodes.filter(node => has(node, "data-message")).map(node => {
    const handle = attr(node, "data-message");
    check(node.tagName === "p" && node.attrs.length === 1 && token.test(handle) && !seen.has(handle), "Invalid message");
    seen.add(handle);
    return Object.freeze({ handle, text: text(node) });
  });
  check(entries.length <= limits.controls, "Message budget exceeded");
  return Object.freeze(entries);
}

function poll(root: Tree.Element): NonNullable<Page["interaction"]>["poll"] {
  if (!has(root, "data-poll") && !has(root, "data-poll-state")) return;
  const path = attr(root, "data-poll"), state = attr(root, "data-poll-state");
  check(/^\/modal-commands\/[A-Za-z0-9_-]{1,128}\/[A-Za-z0-9_-]{1,128}$/.test(path) &&
    (state === "pending" || state === "failed"), "Invalid modal receipt identity");
  return Object.freeze({ path, state });
}

function interaction(root: Tree.Element, header: Pick<Page, "handle" | "revision">, pageId: string,
                     items: readonly Tree.Element[], census: string): Page {
  const call = attr(root, "data-call"), originCommand = attr(root, "data-origin-command");
  const state = attr(root, "data-state");
  const receipt = poll(root);
  const extra = receipt ? 2 : 0;
  check(token.test(call) && (!originCommand || token.test(originCommand)) &&
    items[0]!.attrs.length === 0 && items.at(-1)!.attrs.length === 1 && census === "0", "Invalid interaction identity");
  if (state === "working") {
    check(root.attrs.length === 8 + extra && items.length === 2 && text(items[0]!) === "Working", "Invalid working interaction");
    return Object.freeze({ profile: "3", view: "interaction", page: pageId, ...header, caption: "Working",
      controls: Object.freeze([]), unsupported: 0, interaction: Object.freeze({ state, call, originCommand,
        ...(receipt ? { poll: receipt } : {}) }) });
  }
  check((state === "confirm" || state === "menu") && root.attrs.length === 10 + extra &&
    text(items[0]!) === "Question", "Invalid question interaction");
  const dialog = attr(root, "data-dialog"), defaultChoice = attr(root, "data-default");
  check(token.test(dialog) && /^(?:0|[1-9][0-9]{0,2})$/.test(defaultChoice), "Invalid question identity or default");
  const body = items.slice(1, -1);
  const prompts = body.filter(node => has(node, "data-prompt"));
  check(prompts.length === 1 && prompts[0]!.tagName === "p" && prompts[0]!.attrs.length === 1 &&
    attr(prompts[0]!, "data-prompt") === "true", "Invalid question prompt");
  const commands = new Map<string, Envelope>();
  const entries = controls(body.filter(node => !has(node, "data-message") && !prompts.includes(node)), header, commands);
  check(entries.length > 0 && entries.length <= 128 && BigInt(defaultChoice) < BigInt(entries.length), "Invalid question choices");
  for (const [index, entry] of entries.entries()) {
    const command = `${dialog}_${index}`;
    check(entry.kind === "action" && entry.identity === `$agiru.answer_${command}` &&
      entry.operation?.command === command && entry.operation.enabled && commands.get(command)?.path === "/answers",
      "Question answer contract mismatch");
  }
  if (state === "confirm") check(entries.length === 2 && entries[0]!.caption === "No" && entries[1]!.caption === "Yes", "Invalid confirm choices");
  else check(entries.length >= 2 && entries[0]!.caption === "Cancel", "Invalid menu cancellation");
  const result: Page = Object.freeze({ profile: "3", view: "interaction", page: pageId, ...header, caption: "Question",
    controls: Object.freeze(entries), unsupported: 0, messages: messages(body),
    interaction: Object.freeze({ state, call, originCommand, dialog, defaultChoice, prompt: text(prompts[0]!),
      ...(receipt ? { poll: receipt } : {}) }) });
  envelopes.set(result, commands);
  return result;
}

export function parsePage(html: string): Page {
  check(new TextEncoder().encode(html).byteLength <= limits.bytes, "HTML byte budget exceeded");
  check(!/[\u0000-\u0008\u000B\u000C\u000E-\u001F\u007F]/u.test(html) && html.isWellFormed(), "Invalid HTML text");
  const fragment = parseFragment(html, { sourceCodeLocationInfo: true, onParseError: error => {
    if (error.code === "control-character-reference" && html.slice(error.startOffset - 5, error.startOffset) === "&#13;") return;
    if (error.code === "control-character-in-input-stream" && /^[\u0080-\u009f]$/u.test(html[error.startOffset]!)) return;
    if (error.code === "noncharacter-in-input-stream") return;
    throw new ClientError("ProfileRefused", "Malformed HTML");
  } });
  inspectTree(fragment);
  const roots = children(fragment);
  check(roots.length === 1 && roots[0]!.tagName === "article", "Expected one profile article");
  const root = roots[0]!;
  const profile = attr(root, "data-agiru-profile");
  const view = attr(root, "data-view");
  check((profile === "1" && view === "current-row") || (profile === "2" && view === "list") ||
    (profile === "3" && view === "interaction") ||
    (profile === "4" && (view === "current-row" || view === "list")), "Unsupported HTML profile");
  let modal: Page["interaction"];
  if (profile === "4") {
    const receipt = poll(root);
    const call = attr(root, "data-call"), originCommand = attr(root, "data-origin-command"), dialog = attr(root, "data-dialog");
    check(attr(root, "data-state") === "modal" && token.test(call) && (!originCommand || token.test(originCommand)) &&
      token.test(dialog) && !has(root, "data-default") && root.attrs.length === (view === "list" ? 12 : 9) + (receipt ? 2 : 0), "Invalid modal identity");
    modal = Object.freeze({ state: "modal", call, originCommand, dialog, ...(receipt ? { poll: receipt } : {}) });
  }
  const header: Header = { handle: attr(root, "data-handle"), revision: attr(root, "data-revision"),
    ...(modal ? { modalPath: `/modal-commands/${modal.dialog}` } : {}) };
  const pageId = attr(root, "data-page");
  check(token.test(header.handle) && digits.test(header.revision) && digits.test(pageId), "Invalid page identity");
  const items = children(root);
  check(items[0]?.tagName === "h1" && items.at(-1)?.tagName === "output", "Missing page heading or census");
  const census = attr(items.at(-1)!, "data-unsupported-count");
  check(digits.test(census) && census.length <= 4 && text(items.at(-1)!) === "", "Invalid unsupported census");
  if (profile === "3") {
    return interaction(root, header, pageId, items, census);
  }
  if (!modal) check(!["data-state", "data-call", "data-origin-command", "data-dialog", "data-default", "data-poll", "data-poll-state"].some(name => has(root, name)), "Misplaced interaction metadata");
  const commands = new Map<string, Envelope>();
  const body = items.slice(1, -1);
  const lists = body.filter(node => node.tagName === "section" && attr(node, "data-kind") === "rows");
  check(lists.length === (view === "list" ? 1 : 0), "List profile structure mismatch");
  const queued = messages(body);
  const entries = controls(body.filter(node => !lists.includes(node) && !has(node, "data-message")), header, commands);
  let rows: readonly Row[] | undefined;
  let window: Page["window"];
  if (view === "list") {
    const limit = attr(root, "data-limit");
    const direction = attr(root, "data-direction");
    check(/^[1-9][0-9]{0,18}$/.test(limit) && BigInt(limit) < 9223372036854775807n, "Invalid trusted row bound");
    check(direction === "forward" || direction === "backward", "Invalid window direction");
    window = Object.freeze({ limit, more: exactFlag(root, "data-more"), direction });
    rows = Object.freeze(listRows(lists[0]!, header, commands, limit));
    for (const row of rows) entries.push(Object.freeze({ identity: row.select.control, kind: "action",
      depth: 0, caption: "Select row", operation: row.select }));
  } else check(!["data-limit", "data-more", "data-direction"].some(name => has(root, name)), "Misplaced list metadata");
  const cells = [...entries, ...(rows?.flatMap(row => row.controls) ?? [])];
  check(cells.length <= limits.controls && new Set(entries.map(entry => entry.identity)).size === entries.length,
    "Combined control budget or identity violated");
  const unsupported = cells.filter(control => control.kind === "unsupported").length;
  check(BigInt(census) === BigInt(unsupported), "Unsupported controls disappeared from the census");
  const result: Page = Object.freeze({ profile: profile as "1" | "2" | "4", view: view as "current-row" | "list", page: pageId,
    handle: header.handle, revision: header.revision,
    caption: text(items[0]!), controls: Object.freeze(entries), unsupported, ...(queued.length ? { messages: queued } : {}),
    ...(modal ? { interaction: modal } : {}),
    ...(rows && window ? { rows, window } : {}) });
  envelopes.set(result, commands);
  return result;
}

export function commandEnvelope(page: Page, requested: Operation, textValue?: string): Envelope {
  const operation = page.controls.find(control => control.identity === requested.control)?.operation;
  check(operation && operation.operation === requested.operation && operation.command === requested.command,
    "Command is not advertised by this page");
  if (!operation.enabled) throw new ClientError("CommandDisabled", "The advertised command is disabled");
  check((operation.operation === "set") === (textValue !== undefined), "Set needs explicit text; action cannot accept text");
  const envelope = envelopes.get(page)?.get(operation.command);
  check(envelope, "Missing private command envelope");
  return Object.freeze({ path: envelope.path, fields: Object.freeze({ ...envelope.fields,
    ...(textValue !== undefined ? { text: textValue } : {}) }) });
}
