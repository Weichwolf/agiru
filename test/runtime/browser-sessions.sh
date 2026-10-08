#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-browser-sessions.XXXXXX)
controls=(idle deadline-fences source-revocation source-expiry csrf user-lock rotation-revoke rotation-deadline capacity)
cleanup() {
  find "$proof" -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.bin' -o -name '*.so' -o -name '*.o' -o -name 'auth.*' \) -delete
}
trap cleanup EXIT
git rev-parse HEAD > "$proof/head"
sha256sum Makefile include/runtime/{BrowserSession,BrowserSessionOptions,ClientCredentials,SecureToken}.h \
  src/rt/{BrowserSession,ClientCredentials}.cpp src/rt/CredentialFormat.h src/net/SecureToken.cpp \
  test/gate/{BrowserSessionGate.cpp,OwnedDatabase.h,PrivateAuthFile.h} \
  test/runtime/browser-sessions.sh test/runtime/browser-sessions/MacProvider.cpp \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
"$B/gate_BrowserSessionGate" > "$proof/current.log" 2>&1
cat "$proof/current.log"
AGIRU_TRACE_SQL=2 "$B/gate_BrowserSessionGate" --trace-secrets "$proof/auth" > "$proof/trace.log" 2>&1
trace_secrets=("$proof"/auth.*)
[[ ${#trace_secrets[@]} = 5 ]]
rg -q '^sql: .*browser_sessions' "$proof/trace.log"
rg -q '^  -> .* row\(s\):' "$proof/trace.log"
set +e
jq -er '.authorization | if type == "string" and startswith("Bearer ") then ltrimstr("Bearer ") else error("invalid private trace fixture") end' \
  "${trace_secrets[@]}" | rg -F -f - "$proof/trace.log" > /dev/null
trace_statuses=("${PIPESTATUS[@]}")
set -e
if [[ ${trace_statuses[0]} != 0 || ${trace_statuses[1]} -gt 1 ]]; then
  printf 'browser-sessions: SQL secret-scan input or reader failed\n' >&2
  exit 1
fi
if [[ ${trace_statuses[1]} = 0 ]]; then
  printf 'browser-sessions: secret appeared in SQL tracing\n' >&2
  exit 1
fi
for input in "$proof"/auth.*; do unlink "$input"; done
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -Itest/gate \
  "-DAGIRU_TEST_DSN=\"$dsn\"" --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-L$B" "-Wl,-rpath,$B")
for control in "${controls[@]}"; do
  awk -v control="$control" '
    control == "idle" && /b.idle_expires_at>clock_timestamp\(\)/ {
      sub(/b.idle_expires_at>clock_timestamp\(\)/, "b.idle_expires_at IS NOT NULL"); changed++
    }
    control == "deadline-fences" && /b.expires_at>clock_timestamp\(\)/ {
      sub(/b.expires_at>clock_timestamp\(\)/, "b.expires_at IS NOT NULL"); changed++
    }
    control == "deadline-fences" && /b.idle_expires_at>clock_timestamp\(\)/ {
      sub(/b.idle_expires_at>clock_timestamp\(\)/, "b.idle_expires_at IS NOT NULL"); changed++
    }
    control == "source-revocation" && /AND b.idle_expires_at.*c.revoked_at IS NULL/ {
      sub(/c.revoked_at IS NULL/, "true"); changed++
    }
    control == "source-expiry" && /AND c.expires_at>clock_timestamp\(\)/ {
      sub(/c.expires_at>clock_timestamp\(\)/, "c.expires_at IS NOT NULL"); changed++
    }
    control == "csrf" && /AND b.user_security_id=\$2::uuid AND b.csrf_digest=\$3 AND/ {
      sub(/b.csrf_digest=\$3/, "($3::text IS NOT NULL)"); changed++
    }
    control == "user-lock" && /User Security ID.*FOR UPDATE/ {
      sub(/ FOR UPDATE/, ""); changed++
    }
    control == "rotation-revoke" && /WITH old AS.*SET revoked_at=clock_timestamp\(\)/ {
      sub(/revoked_at=clock_timestamp\(\)/, "revoked_at=b.revoked_at"); changed++
    }
    control == "rotation-deadline" && /interval .1 second.\),LEAST\(old.expires_at,old.source_expires_at\)/ {
      sub(/LEAST\(old.expires_at,old.source_expires_at\)/, "old.expires_at+interval \0471 hour\047"); changed++
    }
    control == "capacity" && /population.Value\(0, 0\) != "f"/ {
      sub(/population.Value\(0, 0\) != "f"/, "false"); changed++
    }
    { print }
    END { if (changed != (control == "csrf" || control == "deadline-fences" ? 2 : 1)) exit 2 }
  ' src/rt/BrowserSession.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" test/gate/BrowserSessionGate.cpp "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.bin" > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  if [[ "$status" != 1 ]]; then
    printf 'browser-sessions: %s defect expected gate exit 1, got %s\n' "$control" "$status" >&2
    exit 1
  fi
  case "$control" in
    idle) claim='expired idle session cannot authenticate' ;;
    deadline-fences) claim='absolute expiry refuses both lookup and renewal' ;;
    source-revocation) claim='revoking the source credential invalidates all derived' ;;
    source-expiry) claim='source expired after issuance invalidates a still-live' ;;
    csrf) claim='foreign CSRF cannot renew another browser identity' ;;
    user-lock) claim='cross-connection admission waits on the same authoritative User row' ;;
    rotation-revoke) claim='rotating transaction sees its exact replacement and denies its old token' ;;
    rotation-deadline) claim='rotation preserves the original absolute deadline' ;;
    capacity) claim='live same-user browser admission obeys the trusted capacity' ;;
  esac
  if ! rg -q "$claim" "$proof/$control.log"; then
    printf 'browser-sessions: %s defect missed its named refusal\n' "$control" >&2
    exit 1
  fi
  unlink "$proof/$control.cpp"
  unlink "$proof/$control.bin"
done
provider_flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -c)
"$CXX" "${provider_flags[@]}" test/runtime/browser-sessions/MacProvider.cpp \
  -o "$proof/mac-provider.o" > "$proof/provider.compile.log" 2>&1
"$CXX" -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -shared \
  "$proof/mac-provider.o" -o "$proof/mac-provider.so" > "$proof/provider.link.log" 2>&1
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/browser-sessions/MacProvider.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${provider_flags[@]}" test/runtime/browser-sessions/MacProvider.cpp \
  -o "$proof/mac-provider.o" > "$B/fixture-commands/browser-session-provider.json"
LD_PRELOAD="$proof/mac-provider.so" "$B/gate_BrowserSessionGate" --mac-provider > "$proof/provider.log" 2>&1
cat "$proof/provider.log"
sha256sum --check --status "$proof/inputs.sha256"
printf 'browser-sessions: nine compiled expiry/source/CSRF/admission/rotation defects reject; MAC provider failure refuses; SQL tracing is secret-free; storage gate, not HTTPS/browser acceptance; %s\n' "$proof"
