import { configuredClient } from "./http.mjs";
import { operate, present } from "./command.mjs";
import { ClientError, errorResult } from "./errors.mjs";

async function main(): Promise<void> {
  const args = process.argv.slice(2);
  const json = args[0] === "--json";
  if (json) args.shift();
  if (args.length === 1 && args[0] === "--help") {
    process.stdout.write("agiru-agent [--json] read <root-relative-path>\n" +
      "agiru-agent [--json] execute <JSON-command>\n" +
      "JSON-command: path,page,revision,command,control,operation; text required only for set\n" +
      "AGIRU_ORIGIN defaults to http://127.0.0.1:8080; AGIRU_AUTH_FILE names private headers JSON\n");
    return;
  }
  if (args.length !== 2) throw new ClientError("ArgumentsRefused", "Use --help for the noninteractive command contract");
  const [name, value] = args as [string, string];
  if (Buffer.byteLength(value) > 1048576) throw new ClientError("ArgumentsRefused", "Command arguments exceed the byte budget");
  let input: unknown;
  try { input = name === "read" ? { path: value } : JSON.parse(value); }
  catch { throw new ClientError("ArgumentsRefused", "Execute needs one valid JSON object"); }
  const result = present(await operate(await configuredClient(), name, input));
  process.stdout.write(json ? JSON.stringify(result.structured) + "\n" : result.text);
}

main().catch(error => {
  process.stderr.write(JSON.stringify(errorResult(error)) + "\n");
  process.exitCode = error instanceof ClientError && error.code === "WriteUncertain" ? 3 : 2;
});
