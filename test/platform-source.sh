#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=${B:-build}
gate="$B/gate_PlatformSourceGate"
if [ ! -x "$gate" ]; then
  printf 'platform-source: missing executable %s; run make\n' "$gate" >&2
  exit 2
fi
if [ -n "${AGIRU_SYSTEM_SYMBOLS+x}" ]; then
  python3 scripts/fetch_symbols.py --verify "$AGIRU_SYSTEM_SYMBOLS"
  exec "$gate" "$AGIRU_SYSTEM_SYMBOLS"
fi
exec "$gate"
