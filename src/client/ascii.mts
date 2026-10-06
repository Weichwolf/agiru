import type { Page } from "./profile.mjs";
import { ClientError } from "./errors.mjs";

export function quote(value: string): string {
  return JSON.stringify(value).replace(/[\u007f-\u009f\u061c\u200e\u200f\u2028-\u202e\u2066-\u2069]/gu,
    character => `\\u${character.codePointAt(0)!.toString(16).padStart(4, "0")}`);
}

export function renderAscii(page: Page, budget = 262144): string {
  const lines = [`page ${page.page} ${quote(page.caption)} handle=${page.handle} rev=${page.revision} view=${page.view}`];
  for (const control of page.controls) {
    const key = quote(control.identity);
    if (control.kind === "group") continue;
    if (control.kind === "unsupported") lines.push(`! ${key} unsupported=${quote(control.reason!)}`);
    else if (control.kind === "label") lines.push(`# ${key} ${quote(control.caption)}`);
    else if (control.kind === "action") {
      lines.push(`action ${key} ${quote(control.caption)} ${control.operation!.enabled ? "enabled" : "disabled"} cmd=${control.operation!.command}`);
    } else {
      const value = control.scalar!;
      const detail = value.domain ? ` domain=${quote(value.domain)} member=${quote(value.member)}` : "";
      const flags = `${value.undefined ? " undefined" : ""}${value.closing ? " closing" : ""}`;
      const command = control.operation ? ` set=${control.operation.command}${control.operation.enabled ? "" : " disabled"}` : " readonly";
      const display = control.display !== value.value ? ` display=${quote(control.display!)}` : "";
      lines.push(`${key} ${value.type}=${quote(value.value)}${detail}${flags}${command}${display}`);
    }
  }
  lines.push(`unsupported=${page.unsupported}`);
  const output = lines.join("\n") + "\n";
  if (!Number.isSafeInteger(budget) || budget < 1 || Buffer.byteLength(output) > budget) {
    throw new ClientError("OutputLimit", "ASCII output budget exceeded; use lossless JSON");
  }
  return output;
}
