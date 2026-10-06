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
  profile: "1"; view: "current-row"; page: string; handle: string; revision: string;
  caption: string; controls: readonly Control[]; unsupported: number;
}>;
type Envelope = Readonly<{ path: string; fields: Readonly<Record<string, string>> }>;
const envelopes = new WeakMap<Page, ReadonlyMap<string, Envelope>>();
const token = /^[A-Za-z0-9_-]{1,128}$/;
const digits = /^[0-9]+$/;
const scalarNames = ["Text", "Code", "Integer", "BigInteger", "Decimal", "Boolean", "Option", "Enum",
  "Date", "Time", "DateTime", "Duration", "Guid", "DateFormula", "RecordId"];
const tags = new Set(["article", "h1", "h2", "h3", "section", "form", "input", "button", "output", "aside", "p"]);
const scalarAttributes = ["data-type", "data-value", "data-domain", "data-member", "data-undefined", "data-closing"];
const attributes: Readonly<Record<string, readonly string[]>> = {
  article: ["data-agiru-profile", "data-view", "data-page", "data-handle", "data-revision"],
  h1: [], h2: [], h3: [], section: ["data-control", "data-kind"],
  form: ["method", "action", "hx-post", "hx-target", "hx-swap"],
  input: ["type", "name", "value", "aria-label", ...scalarAttributes],
  button: ["type", "disabled"], output: ["data-unsupported-count", ...scalarAttributes],
  aside: ["role", "data-control", "data-unsupported"], p: ["data-control"],
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

function form(node: Tree.Element, page: Pick<Page, "handle" | "revision">, identity: string,
              operation: "set" | "action", commands: Map<string, Envelope>): Operation {
  check(attr(node, "method") === "post", "Commands must use POST");
  const path = attr(node, "action");
  check(/^\/(?!\/)[A-Za-z0-9_/-]*$/.test(path) && !path.split("/").some(part => part === "." || part === ".."), "Unsafe command path");
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

function controls(root: Tree.Element, page: Pick<Page, "handle" | "revision">, commands: Map<string, Envelope>): Control[] {
  const result: Control[] = [];
  const identities = new Set<string>();
  function visit(node: Tree.Element, depth: number): void {
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
  for (const child of children(root).slice(1, -1)) visit(child, 0);
  return result;
}

export function parsePage(html: string): Page {
  check(Buffer.byteLength(html, "utf8") <= limits.bytes, "HTML byte budget exceeded");
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
  check(attr(root, "data-agiru-profile") === "1" && attr(root, "data-view") === "current-row", "Unsupported HTML profile");
  const header = { handle: attr(root, "data-handle"), revision: attr(root, "data-revision") };
  const pageId = attr(root, "data-page");
  check(token.test(header.handle) && digits.test(header.revision) && digits.test(pageId), "Invalid page identity");
  const items = children(root);
  check(items[0]?.tagName === "h1" && items.at(-1)?.tagName === "output", "Missing page heading or census");
  const census = attr(items.at(-1)!, "data-unsupported-count");
  check(digits.test(census) && census.length <= 4 && text(items.at(-1)!) === "", "Invalid unsupported census");
  const commands = new Map<string, Envelope>();
  const entries = controls(root, header, commands);
  const unsupported = entries.filter(control => control.kind === "unsupported").length;
  check(BigInt(census) === BigInt(unsupported), "Unsupported controls disappeared from the census");
  const result: Page = Object.freeze({ profile: "1", view: "current-row", page: pageId, ...header,
    caption: text(items[0]!), controls: Object.freeze(entries), unsupported });
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
