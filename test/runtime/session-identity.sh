#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-session-identity.XXXXXX)
controls=(state guid name)
command_controls=(rollback commit epoch cursor-owner)
credential_controls=(credential-expiry credential-revocation credential-owner)
provider_controls=(random-fallback digest-fallback)
value_controls=(area-thread random-thread zero-seed negative-bound clock-seed wide-bound)
cleanup() {
  for control in "${controls[@]}" "${command_controls[@]}" "${credential_controls[@]}" "${provider_controls[@]}" "${value_controls[@]}"; do
    for extension in cpp bin; do
      input="$proof/$control.$extension"
      if [[ -f "$input" ]]; then unlink "$input"; fi
    done
  done
  if [[ -f "$proof/provider-failure.so" ]]; then unlink "$proof/provider-failure.so"; fi
  if [[ -f "$proof/provider-failure.o" ]]; then unlink "$proof/provider-failure.o"; fi
  find "$proof" -maxdepth 1 -type f \( -name '*.so' -o -name 'clock.o' \) -delete
}
trap cleanup EXIT
git rev-parse HEAD > "$proof/head"
sha256sum src/rt/Session.cpp include/runtime/Session.h include/platform/User.h \
  src/rt/SessionCommand.cpp include/runtime/SessionCommand.h src/rt/Cursor.cpp src/rt/Cursor.h \
  src/rt/Transaction.cpp include/runtime/Transaction.h src/db/Connection.cpp include/runtime/Database.h \
  include/runtime/SingleInstance.h src/rt/SingleInstance.cpp src/rt/SessionState.h \
  test/gate/SessionCommandGate.cpp \
  test/gate/SessionIdentityGate.cpp test/gate/OwnedDatabase.h test/gate/PrivateAuthFile.h test/runtime/session-identity.sh \
  test/gate/AccountFixturePermissions.h include/runtime/TablePermissions.h src/rt/TablePermissions.cpp \
  include/runtime/{SecureToken,ClientCredentials}.h src/net/SecureToken.cpp src/rt/ClientCredentials.cpp \
  test/gate/ClientCredentialsGate.cpp test/runtime/client-credentials/ProviderFailure.cpp \
  include/BuiltinsWritten.h src/rt/written/BuiltinsWritten.cpp src/rt/SessionRandom.{h,cpp} \
  src/rt/UiHost.cpp test/gate/{SessionValues,GenReceiver}Gate.cpp test/runtime/session-values/Clock.cpp \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
"$B/gate_SessionIdentityGate" > "$proof/current.log" 2>&1
"$B/gate_SessionCommandGate" > "$proof/commands.log" 2>&1
"$B/gate_ClientCredentialsGate" > "$proof/credentials.log" 2>&1
"$B/gate_SessionValuesGate" > "$proof/values.log" 2>&1
"$B/gate_GenReceiverGate" > "$proof/values-generator.log" 2>&1
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
for control in "${credential_controls[@]}"; do
  awk -v control="$control" '
    control == "credential-expiry" && /AND expires_at > clock_timestamp\(\)/ {
      sub(/AND expires_at > clock_timestamp\(\)/, ""); changed++
    }
    control == "credential-revocation" && /WHERE digest = \$1 AND revoked_at IS NULL AND/ {
      sub(/AND revoked_at IS NULL /, ""); changed++
    }
    control == "credential-owner" && /SELECT user_security_id::text FROM agiru_client.credentials/ {
      sub(/user_security_id::text/, "\04700000000-0000-0000-0000-000000000001\047::text"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/ClientCredentials.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" test/gate/ClientCredentialsGate.cpp "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -pthread -o "$proof/$control.bin" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    credential-expiry) rg -q 'expiry is evaluated against PostgreSQL time' "$proof/$control.log" ;;
    credential-revocation) rg -q 'revocation survives a different connection' "$proof/$control.log" ;;
    credential-owner) rg -q 'independent credentials retain exact system GUID ownership' "$proof/$control.log" ;;
  esac
done
provider_flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -c)
"$CXX" "${provider_flags[@]}" test/runtime/client-credentials/ProviderFailure.cpp \
  -o "$proof/provider-failure.o" > "$proof/provider-failure.compile.log" 2>&1
"$CXX" -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -shared \
  "$proof/provider-failure.o" -o "$proof/provider-failure.so" \
  > "$proof/provider-failure.link.log" 2>&1
for provider in random digest; do
  LD_PRELOAD="$proof/provider-failure.so" "$B/gate_ClientCredentialsGate" \
    --provider-failure "$provider" > "$proof/provider-$provider.log" 2>&1
done
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" \
  --arg file "$PWD/test/runtime/client-credentials/ProviderFailure.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${provider_flags[@]}" test/runtime/client-credentials/ProviderFailure.cpp \
  -o "$proof/provider-failure.o" > "$B/fixture-commands/client-credential-provider.json"
for control in "${provider_controls[@]}"; do
  awk -v control="$control" '
    control == "random-fallback" && /if \(RAND_priv_bytes/ {
      sub(/!= 1/, "== 1"); changed++
    }
    control == "digest-fallback" && /if \(status != 1 \|\| length != digest.size\(\)\)/ {
      sub(/status != 1 \|\| length != digest.size\(\)/, "status == 1"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/SecureToken.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" test/gate/ClientCredentialsGate.cpp "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -lcrypto -pthread -o "$proof/$control.bin" \
    > "$proof/$control.compile.log" 2>&1
  provider=${control%-fallback}
  status=0
  LD_PRELOAD="$proof/provider-failure.so" "$proof/$control.bin" --provider-failure "$provider" \
    > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  rg -q 'provider failure explicitly refuses without fallback' "$proof/$control.log"
done
clock_flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -Iinclude -c)
"$CXX" "${clock_flags[@]}" test/runtime/session-values/Clock.cpp -o "$proof/clock.o" \
  > "$proof/clock.compile.log" 2>&1
"$CXX" -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -shared \
  "$proof/clock.o" -o "$proof/clock.so" > "$proof/clock.link.log" 2>&1
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/session-values/Clock.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${clock_flags[@]}" test/runtime/session-values/Clock.cpp -o "$proof/clock.o" \
  > "$B/fixture-commands/session-values-clock.json"
LD_PRELOAD="$proof/clock.so" "$B/gate_SessionValuesGate" --clock-fixture > "$proof/clock.log" 2>&1
for control in "${value_controls[@]}"; do
  source=src/rt/written/BuiltinsWritten.cpp
  claim='nested randomization cannot change the outer sequence'
  case "$control" in
    area-thread) claim='nested application-area changes do not overwrite the outer session';;
    zero-seed) claim='explicit seed zero is not silently replaced with one';;
    negative-bound) claim='negative and positive bounds preserve exactly the same draw';;
    clock-seed) claim='parameterless Randomize uses the current clock seed';;
    wide-bound)
      source=src/rt/SessionRandom.cpp
      claim='Random accepts representable inclusive positive or negative bounds'
      ;;
  esac
  awk -v control="$control" '
    control == "area-thread" && /return detail::SessionState::Current\(\).applicationArea;/ {
      print "  static thread_local std::string area; return area;"; changed++; next
    }
    control == "random-thread" && /auto &sequence = detail::SessionState::Current\(\).random;/ {
      print "  static thread_local std::unique_ptr<detail::SessionRandom> sequence;"; changed++; next
    }
    control == "zero-seed" && /Sequence\(\).Reseed\(Seed\);/ {
      sub(/Reseed\(Seed\)/, "Reseed(Seed != 0 ? Seed : kDefaultSeed)"); changed++
    }
    control == "negative-bound" && /return Sequence\(\).Bounded\(largest\);/ {
      sub(/Bounded\(largest\)/, "Bounded(MaxNumber < 0 ? 1 : largest)"); changed++
    }
    control == "clock-seed" && /Randomize\(CurrentTime\(\).AsMilliseconds\(\)\);/ {
      sub(/CurrentTime\(\).AsMilliseconds\(\)/, "kDefaultSeed"); changed++
    }
    control == "wide-bound" && /static_cast<double>\(maximum\)/ {
      sub(/static_cast<double>\(maximum\)/, "static_cast<double>(maximum + 1)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  LD_PRELOAD="$proof/clock.so:$proof/$control.so" "$B/gate_SessionValuesGate" --clock-fixture \
    > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  rg -q "FAIL .*${claim}" "$proof/$control.log"
done
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
cat "$proof/current.log"
cat "$proof/commands.log"
cat "$proof/credentials.log"
cat "$proof/values.log"
cat "$proof/values-generator.log"
cat "$proof/clock.log"
printf 'session-values: six compiled ownership/seed/bound/clock defects reject; indirect-minimum Random bound remains unqualified\n'
printf 'session-identity: twelve identity/command/credential/provider defects reject; secure random/digest provider failures refuse; receipts %s\n' "$proof"
