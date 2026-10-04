#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-base64-core.XXXXXX)
gate="$B/gate_Base64Gate"
"$gate" > "$proof/current.log" 2>&1
if [ "${AGIRU_BASE64_REFERENCE+x}" = x ]; then
  "$gate" "$AGIRU_BASE64_REFERENCE" > "$proof/reference.log" 2>&1
  sha256sum "$AGIRU_BASE64_REFERENCE" > "$proof/reference.sha256"
fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in line-width padding whitespace validation raw-write; do
  awk -v control="$control" '
    control == "line-width" && /^constexpr std::size_t kLineWidth = 76;/ {
      sub(/76/, "64"); changed++
    }
    control == "padding" && /const std::size_t count = group\[2\] == kPadding/ {
      $0 = "    const std::size_t count = kInputGroup;"; changed++
    }
    control == "whitespace" && /if \(value == kWhitespace\)/ {
      sub(/value == kWhitespace/, "value == kWhitespace || byte == 11 || byte == 12"); changed++
    }
    control == "validation" && /^  Decode\(text, nullptr\);$/ {
      $0 = ""; changed++
    }
    control == "raw-write" && /stream_->WriteBytes\(/ {
      sub(/stream_->WriteBytes\(/, "stream_->Write("); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/Base64.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'base64: %s escaped the byte codec gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sha256sum src/net/Base64.cpp include/type/Base64.h src/net/Stream.cpp \
  include/type/Stream.h test/gate/Base64Gate.cpp "$B/libagiru_net.so" "$gate" \
  > "$proof/inputs.sha256"
printf 'base64: raw codec and stream output pass; five compiled controls reject; %s\n' "$proof"
