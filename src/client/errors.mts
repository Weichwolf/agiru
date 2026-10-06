export class ClientError extends Error {
  constructor(readonly code: string, message: string, readonly command?: string) {
    super(message);
  }
}

export function requireContract(condition: unknown, message: string): asserts condition {
  if (!condition) throw new ClientError("ProfileRefused", message);
}

export function errorResult(error: unknown): Record<string, unknown> {
  if (error instanceof ClientError) {
    return { error: error.code, message: error.message, ...(error.command ? { command: error.command } : {}) };
  }
  return { error: "ClientFailure", message: "Client operation failed" };
}
