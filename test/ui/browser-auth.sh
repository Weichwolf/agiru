#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-browser-auth.XXXXXX)
controls=(csrf metadata proxy-tls origin ambiguous-cookie duplicate-cookie cookie-flags retained-secrets https-config)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.bin" \) -delete' EXIT
git rev-parse HEAD > "$proof/head"
sha256sum Makefile deploy/dev/agiru.json include/runtime/{BrowserSession,BrowserSessionOptions,PageHostOptions,SecureToken}.h \
  src/rt/BrowserHttp.{h,cpp} src/rt/{BrowserSession,PageCommandHost,PageInteraction,PageModal,NativeService,NativeServiceConfig}.cpp \
  src/rt/{PageInteraction,PageModal}.h src/net/SecureToken.cpp \
  test/gate/{BrowserHttpGate,NativeServiceConfigGate}.cpp test/gate/{OwnedDatabase,PrivateAuthFile}.h \
  test/ui/browser-auth.sh "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
"$B/gate_BrowserHttpGate" > "$proof/current.log" 2>&1
"$B/gate_NativeServiceConfigGate" > "$proof/config.log" 2>&1
cat "$proof/current.log" "$proof/config.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -Isrc/net -Itest/gate \
  "-DAGIRU_TEST_DSN=\"$dsn\"" "-DAGIRU_SOURCE_DIR=\"$PWD\"" \
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 "-L$B" "-Wl,-rpath,$B")
for control in "${controls[@]}"; do
  awk -v control="$control" '
    control == "csrf" && /if \(requireCsrf && !SecureTokenEqual/ {
      sub(/requireCsrf/, "(static_cast<void>(requireCsrf), false)"); changed++
    }
    control == "metadata" && /request.Header\("Sec-Fetch-Site"\) != "same-origin"/ {
      sub(/request.Header\("Sec-Fetch-Site"\) != "same-origin"/, "false"); changed++
    }
    control == "proxy-tls" && /request.Header\("X-Forwarded-Proto"\) != "https"/ {
      sub(/request.Header\("X-Forwarded-Proto"\) != "https"/, "false"); changed++
    }
    control == "origin" && /request.Header\("Origin"\) != options.origin/ {
      changed += gsub(/request.Header\("Origin"\) != options.origin/, "false");
    }
    control == "ambiguous-cookie" && /if \(!request.Header\("Authorization"\).empty\(\)\)/ {
      sub(/if \(!request.Header\("Authorization"\).empty\(\)\)/, "if (false)"); changed++
    }
    control == "duplicate-cookie" && /if \(cookie\) \{ Refuse/ {
      sub(/if \(cookie\)/, "if (false)"); changed++
    }
    control == "cookie-flags" && /constexpr std::string_view kFlags =/ {
      sub(/ HttpOnly;/, ""); changed++
    }
    control == "retained-secrets" && /for \(const std::string_view name :/ {
      sub(/"Origin"/, "\"Origin\", \"Cookie\", \"Authorization\""); changed++
    }
    control == "https-config" && /!options.origin.starts_with\(kHttpsScheme\)/ {
      sub(/!options.origin.starts_with\(kHttpsScheme\)/, "false"); changed++
    }
    { print }
    END { if (changed != (control == "origin" ? 2 : 1)) exit 2 }
  ' src/rt/BrowserHttp.cpp > "$proof/$control.cpp"
  gate_source=BrowserHttpGate
  if [[ "$control" = https-config ]]; then gate_source=NativeServiceConfigGate; fi
  "$CXX" "${flags[@]}" "test/gate/$gate_source.cpp" "$proof/$control.cpp" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.bin" > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  if [[ "$status" != 1 ]]; then
    printf 'browser-auth: %s defect expected gate exit 1, got %s\n' "$control" "$status" >&2
    exit 1
  fi
  case "$control" in
    csrf|metadata|proxy-tls|origin) claim='invalid browser context refuses before protected GET' ;;
    ambiguous-cookie) claim='cookie and bearer ambiguity refuses' ;;
    duplicate-cookie) claim='repeated session cookies refuse' ;;
    cookie-flags) claim='cookie is host-only secure HttpOnly' ;;
    retained-secrets) claim='retained AL work contains no Cookie' ;;
    https-config) claim='cookies cannot be activated on the HTTP' ;;
  esac
  rg -q "$claim" "$proof/$control.log"
done
sha256sum --check --status "$proof/inputs.sha256"
printf 'browser-auth: nine compiled CSRF/metadata/proxy/origin/cookie/retention/configuration defects reject; native protocol gate, not browser or ERP acceptance; %s\n' "$proof"
