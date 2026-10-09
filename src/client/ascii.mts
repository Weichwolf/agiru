import type { Control, Page } from "./profile.mjs";
import { ClientError } from "./errors.mjs";

export function quote(value: string): string {
  return JSON.stringify(value).replace(/[\u007f-\u009f\u061c\u200e\u200f\u2028-\u202e\u2066-\u2069]/gu,
    character => `\\u${character.codePointAt(0)!.toString(16).padStart(4, "0")}`);
}

function field(control: Control, command = ""): string {
  const value = control.scalar!;
  const domain = value.domain ? ` domain=${quote(value.domain)} member=${quote(value.member)}` : "";
  const flags = `${value.undefined ? " undefined" : ""}${value.closing ? " closing" : ""}`;
  const display = control.display !== value.value ? ` display=${quote(control.display!)}` : "";
  const choices = control.choices ? ` choices=[${control.choices.map(choice =>
    `{"value":${quote(choice.value)},"member":${quote(choice.member)},"caption":${quote(choice.caption)}}`).join(",")}]` : "";
  return `${quote(control.identity)} ${value.type}=${quote(value.value)}${domain}${flags}${command}${display}${choices}`;
}

export function renderAscii(page: Page, budget = 262144): string {
  const lines = [`page ${page.page} ${quote(page.caption)} handle=${page.handle} rev=${page.revision} view=${page.view}`];
  if (page.interaction) {
    lines.push(`${page.interaction.state} call=${page.interaction.call} command=${page.interaction.originCommand || "(opening)"}`);
    if (page.interaction.poll) lines.push(`receipt=${page.interaction.poll.path} state=${page.interaction.poll.state}`);
    if (page.interaction.state === "modal") lines.push(`modal=${page.interaction.dialog} explicit close required`);
    else if (page.interaction.dialog) lines.push(`dialog=${page.interaction.dialog} default=${page.interaction.defaultChoice} prompt=${quote(page.interaction.prompt!)}`);
  }
  for (const message of page.messages ?? []) lines.push(`message ${message.handle} ${quote(message.text)}`);
  for (const control of page.controls) {
    const key = quote(control.identity);
    if (control.kind === "group") continue;
    if (control.kind === "unsupported") lines.push(`! ${key} unsupported=${quote(control.reason!)}`);
    else if (control.kind === "label") lines.push(`# ${key} ${quote(control.caption)}`);
    else if (control.kind === "action") {
      lines.push(`action ${key} ${quote(control.caption)} ${control.operation!.enabled ? "enabled" : "disabled"} cmd=${control.operation!.command}`);
    } else {
      const command = control.operation ? ` set=${control.operation.command}${control.operation.enabled ? "" : " disabled"}` : " readonly";
      lines.push(field(control, command));
    }
  }
  if (page.rows) {
    lines.push(`rows=${page.rows.length} limit=${page.window!.limit} more=${page.window!.more} direction=${page.window!.direction}`);
    for (const row of page.rows) {
      const values = row.controls.filter(control => control.kind === "field").map(control => field(control));
      lines.push(`row ${row.handle}${row.selected ? " selected" : ""} ${values.join(" ")} select=${row.select.command}`);
      for (const cell of row.controls.filter(control => control.kind === "unsupported")) {
        lines.push(`! row=${row.handle} ${quote(cell.identity)} unsupported=${quote(cell.reason!)}`);
      }
    }
  }
  lines.push(`unsupported=${page.unsupported}`);
  const output = lines.join("\n") + "\n";
  if (!Number.isSafeInteger(budget) || budget < 1 || Buffer.byteLength(output) > budget) {
    throw new ClientError("OutputLimit", "ASCII output budget exceeded; use lossless JSON");
  }
  return output;
}
