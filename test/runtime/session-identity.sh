#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-session-identity.XXXXXX)
controls=(state guid name)
command_controls=(rollback commit epoch cursor-owner)
cleanup() {
  for control in "${controls[@]}" "${command_controls[@]}"; do
    for extension in cpp bin; do
      input="$proof/$control.$extension"
      if [[ -f "$input" ]]; then unlink "$input"; fi
    done
  done
}
trap cleanup EXIT
git rev-parse HEAD > "$proof/head"
sha256sum src/rt/Session.cpp include/runtime/Session.h include/platform/User.h \
  src/rt/SessionCommand.cpp include/runtime/SessionCommand.h src/rt/Cursor.cpp src/rt/Cursor.h \
  src/rt/Transaction.cpp include/runtime/Transaction.h src/db/Connection.cpp include/runtime/Database.h \
  include/runtime/SingleInstance.h src/rt/SingleInstance.cpp src/rt/SessionState.h \
  test/gate/SessionCommandGate.cpp \
  test/gate/SessionIdentityGate.cpp test/gate/OwnedDatabase.h test/runtime/session-identity.sh \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
"$B/gate_SessionIdentityGate" > "$proof/current.log" 2>&1
"$B/gate_SessionCommandGate" > "$proof/commands.log" 2>&1
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
for control in "${command_controls[@]}"; do
  case "$control" in
    rollback|commit) source=src/rt/SessionCommand.cpp ;;
    epoch) source=src/rt/Transaction.cpp ;;
    cursor-owner) source=src/rt/Cursor.cpp ;;
  esac
  awk -v control="$control" '
    control == "rollback" && /connection_->Run\("ROLLBACK"\)/ {
      sub(/connection_->Run\("ROLLBACK"\)/, "connection_->Run(\"COMMIT\")"); changed++
    }
    control == "commit" && /session_->boundaries_\.Commit\(\*connection_\);/ {
      sub(/session_->boundaries_\.Commit\(\*connection_\);/, "static_cast<void>(session_->boundaries_);"); changed++
    }
    /void Boundaries::ClearCommand\(\)/ { clearing=1 }
    clearing && control == "epoch" && /\+\+cursorEpoch_;/ { changed++; next }
    clearing && /^}/ { clearing=0 }
    control == "cursor-owner" && /Session::Current\(\) != session_/ {
      sub(/\&Session::Current\(\) != session_/, "(static_cast<void>(session_), false)"); changed++
    }
    control == "cursor-owner" && /Session::Current\(\) == session_/ {
      sub(/\&Session::Current\(\) == session_/, "(static_cast<void>(session_), true)"); changed++
    }
    { print }
    END { if (changed != (control == "cursor-owner" ? 2 : 1)) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" test/gate/SessionCommandGate.cpp "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -pthread -o "$proof/$control.bin" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    rollback) rg -q 'unfinished commands roll back their writes' "$proof/$control.log" ;;
    commit) rg -q 'Keep detaches immediately and returns an idle connection' "$proof/$control.log" ;;
    epoch) rg -q 'host rollback invalidates a cursor even when session and connection are reused' "$proof/$control.log" ;;
    cursor-owner) rg -q 'same connection and epoch do not grant another user.s cursor ownership' "$proof/$control.log" ;;
  esac
done
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
cat "$proof/current.log"
cat "$proof/commands.log"
printf 'session-identity: state/GUID/name and rollback/commit/epoch/cursor-owner defects reject; receipts %s\n' "$proof"
