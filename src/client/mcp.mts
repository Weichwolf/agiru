import { McpServer } from "@modelcontextprotocol/sdk/server/mcp.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import { Transform } from "node:stream";
import { configuredClient } from "./http.mjs";
import { executeSchema, operate, present, readSchema } from "./command.mjs";
import { errorResult } from "./errors.mjs";

async function main(): Promise<void> {
  const client = await configuredClient();
  const server = new McpServer({ name: "agiru-agent", version: "0.1.0" }, { maxToolInputElements: 64 });
  for (const [name, inputSchema] of [["read", readSchema], ["execute", executeSchema]] as const) {
    server.registerTool(`agiru_${name}`, {
      description: name === "read" ? "Read the shared semantic HTML page; discover exact values and explicit commands" :
        "Submit one advertised command with explicit page handle, revision and command ID; never retries uncertain writes",
      inputSchema,
      annotations: { readOnlyHint: name === "read", destructiveHint: name === "execute", idempotentHint: name === "read" },
    }, async (argumentsValue: unknown) => {
      try {
        const value = present(await operate(client, name, argumentsValue));
        return { content: [{ type: "text" as const, text: value.text }], structuredContent: { ...value.structured } };
      } catch (error) {
        const value = errorResult(error);
        return { isError: true, content: [{ type: "text" as const, text: JSON.stringify(value) }], structuredContent: value };
      }
    });
  }
  const decoder = new TextDecoder("utf-8", { fatal: true });
  const input = new Transform({
    transform(chunk: Buffer, _encoding, callback) {
      try { decoder.decode(chunk, { stream: true }); callback(null, chunk); }
      catch { callback(new Error("Invalid MCP UTF-8")); }
    },
    flush(callback) {
      try { decoder.decode(); callback(); }
      catch { callback(new Error("Incomplete MCP UTF-8")); }
    },
  });
  const transport = new StdioServerTransport(input, process.stdout, { maxBufferSize: 1048576 });
  let stopped = false;
  const refuse = () => {
    if (stopped) return;
    stopped = true;
    process.stderr.write("MCP input refused\n");
    process.stdin.unpipe(input);
    process.stdin.pause();
    input.destroy();
    process.exitCode = 2;
    void server.close();
  };
  input.on("error", refuse);
  transport.onerror = refuse;
  await server.connect(transport);
  process.stdin.pipe(input);
}

main().catch(error => { process.stderr.write(JSON.stringify(errorResult(error)) + "\n"); process.exitCode = 2; });
