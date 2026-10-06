#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-session-identity.XXXXXX)
controls=(state guid name)
cleanup() {
  for control in "${controls[@]}"; do
    for extension in cpp bin; do
      input="$proof/$control.$extension"
      if [[ -f "$input" ]]; then unlink "$input"; fi
    done
  done
}
trap cleanup EXIT
git rev-parse HEAD > "$proof/head"
sha256sum src/rt/Session.cpp include/runtime/Session.h include/platform/User.h \
  test/gate/SessionIdentityGate.cpp test/gate/OwnedDatabase.h test/runtime/session-identity.sh \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
"$B/gate_SessionIdentityGate" > "$proof/current.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt \
  -Itest/gate "-DAGIRU_TEST_DSN=\"$dsn\"" --rtlib=compiler-rt --unwindlib=libunwind \
  -fuse-ld=lld-19 "-L$B" "-Wl,-rpath,$B")
for control in "${controls[@]}"; do
  awk -v control="$control" '
    control == "state" && /user.State != platform::UserState::Enabled/ {
      sub(/user.State != platform::UserState::Enabled/, "false"); changed++
    }
    control == "guid" && /userSecurityId_ = user.UserSecurityID;/ {
      sub(/userSecurityId_ = user.UserSecurityID;/, "userSecurityId_ = Guid{};"); changed++
    }
    control == "name" && /userId_ = user.UserName.Value\(\);/ {
      sub(/userId_ = user.UserName.Value\(\);/, "userId_ = \"SYSTEM\";"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/Session.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" test/gate/SessionIdentityGate.cpp "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -pthread -o "$proof/$control.bin" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    state) rg -q 'invalid or inactive identity refuses session construction' "$proof/$control.log" ;;
    guid) rg -q 'SQL creator is the original session.s typed security ID' "$proof/$control.log" ;;
    name) rg -q 'UserId comes from the Code-typed database field' "$proof/$control.log" ;;
  esac
done
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
cat "$proof/current.log"
printf 'session-identity: state, GUID and name defects reject; receipts %s\n' "$proof"
