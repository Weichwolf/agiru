import { z } from "zod";
import { AgentClient, type Result } from "./http.mjs";
import { renderAscii } from "./ascii.mjs";
import { ClientError } from "./errors.mjs";

const path = z.string().min(1).max(8192);
const token = z.string().regex(/^[A-Za-z0-9_-]{1,128}$/);
export const readSchema = z.object({ path }).strict();
export const executeSchema = z.object({ path, page: token, revision: z.string().regex(/^[0-9]{1,128}$/),
  command: token, control: z.string().min(1).max(4096), operation: z.enum(["set", "action"]),
  text: z.string().max(1048576).optional() }).strict().superRefine((value, context) => {
    if ((value.operation === "set") !== (value.text !== undefined)) {
      context.addIssue({ code: "custom", message: "Set requires text; action must not supply text" });
    }
  });

export async function operate(client: AgentClient, name: string, argumentsValue: unknown): Promise<Result> {
  if (name === "read") {
    const parsed = readSchema.safeParse(argumentsValue);
    if (!parsed.success) throw new ClientError("ArgumentsRefused", "Read requires only a bounded path");
    return client.read(parsed.data.path);
  }
  if (name === "execute") {
    const parsed = executeSchema.safeParse(argumentsValue);
    if (!parsed.success) throw new ClientError("ArgumentsRefused", "Execute requires explicit page/revision/command/control/operation and set text");
    const { path: target, text, ...request } = parsed.data;
    return client.execute(target, { ...request, ...(text !== undefined ? { text } : {}) });
  }
  throw new ClientError("OperationRefused", "Unknown client operation");
}

export function present(result: Result): { text: string; structured: Result & { presentation?: { ascii: "refused"; code: "OutputLimit" } } } {
  try { return { text: renderAscii(result.page), structured: result }; }
  catch (error) {
    if (!(error instanceof ClientError) || error.code !== "OutputLimit") throw error;
    return { text: "! ASCII output budget exceeded; exact values remain in --json / MCP structuredContent\n",
      structured: { ...result, presentation: { ascii: "refused", code: "OutputLimit" } } };
  }
}
