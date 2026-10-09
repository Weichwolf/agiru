#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-page-call-authority.XXXXXX)
cleanup() {
  find "$proof" -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.so' \) -delete
}
trap cleanup EXIT
findmnt -T /tmp > "$proof/mount.txt"
df -h /tmp > "$proof/space.txt"
git rev-parse HEAD > "$proof/head"
sha256sum Makefile src/rt/{CommandAuthority,PageCallAuthority,SessionUser,PageInteraction,PageModal}.{h,cpp} \
  include/runtime/{PageCommandHost,PageHostOptions,Session,SessionCommand,BrowserSession,ClientCredentials}.h \
  include/platform/User.h src/rt/{BrowserSession,ClientCredentials}.cpp src/net/SecureToken.cpp \
  src/rt/{PageCommandHost,Session,SessionCommand,Transaction}.cpp src/rt/SessionState.h \
  test/gate/PageCallAuthorityGate.cpp test/runtime/page-call-authority.sh \
  "$B/gate_PageCallAuthorityGate" "$B/libagiru_"{rt,net,db}.so > "$proof/inputs.sha256"
"$B/gate_PageCallAuthorityGate" > "$proof/current.log" 2>&1
cat "$proof/current.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt \
  -fPIC -shared --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
for control in expiry idle sticky fence wait serialization own-account; do
  source=src/rt/PageCallAuthority.cpp
  case "$control" in
    fence) source=src/rt/CommandAuthority.cpp ;;
    wait) source=src/rt/PageInteraction.cpp ;;
  esac
  awk -v control="$control" '
    /void PageCallAuthority::LockCommit/ { committing=1 }
    control == "expiry" && /c.expires_at>clock_timestamp\(\)/ {
      sub(/c.expires_at>clock_timestamp\(\)/, "c.expires_at IS NOT NULL"); changed++
    }
    control == "idle" && /b.idle_expires_at>clock_timestamp\(\)/ {
      sub(/b.idle_expires_at>clock_timestamp\(\)/, "b.idle_expires_at IS NOT NULL"); changed++
    }
    control == "sticky" && /if \(call_->cancelled \|\| call_->deadline/ {
      sub(/call_->cancelled \|\| /, ""); changed++
    }
    control == "fence" && /state->commandAuthority->LockCommit\(connection\)/ {
      sub(/state->commandAuthority->LockCommit\(connection\)/, "static_cast<void>(connection)"); changed++
    }
    control == "wait" && /CheckCommandAuthority\(\);/ {
      sub(/CheckCommandAuthority\(\)/, "static_cast<void>(0)"); changed++
    }
    control == "serialization" && /FOR SHARE OF c,p/ {
      sub(/ FOR SHARE OF c,p/, ""); sub(/ FOR SHARE OF b,c,p/, ""); changed++
    }
    control == "own-account" && committing && /RequireActiveUser\(authority_, call_->user\)/ {
      sub(/RequireActiveUser\(authority_, call_->user\)/, "RequireActiveUser(connection, call_->user)"); changed++
    }
    { print }
    END { if (changed != (control == "expiry" ? 2 : 1)) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -o "$proof/$control.so" > "$proof/$control.compile.log" 2>&1
  status=0
  LD_PRELOAD="$proof/$control.so" "$B/gate_PageCallAuthorityGate" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]] || { printf 'page-call-authority: %s defect returned %s, expected 1\n' "$control" "$status" >&2; exit 1; }
  case "$control" in
    expiry|idle|fence) claim='expired or revoked active authority refuses explicit Commit' ;;
    sticky) claim='restoring a credential cannot resurrect the cancelled call' ;;
    wait) claim='revocation wakes a suspended AL question' ;;
    serialization) claim='revocation cannot overtake an already authorized committing transaction' ;;
    own-account) claim='own pending account change is not an externally committed revocation' ;;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control.log"
  unlink "$proof/$control.cpp"
  unlink "$proof/$control.so"
done
sha256sum --check --status "$proof/inputs.sha256"
printf 'page-call-authority: seven compiled expiry/idle/sticky/commit/wait/serialization/own-account defects reject; %s\n' "$proof"
