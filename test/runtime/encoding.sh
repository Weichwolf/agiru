#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-encoding.XXXXXX)
gate="$B/gate_EncodingGate"
"$gate" > "$proof/current.log" 2>&1
if [ "${AGIRU_ENCODING_REFERENCE+x}" = x ]; then
  "$gate" "$AGIRU_ENCODING_REFERENCE" > "$proof/reference.log" 2>&1
  sha256sum "$AGIRU_ENCODING_REFERENCE" > "$proof/reference.sha256"
fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in utf8-passthrough surrogate-validation utf16-tail scalar-range char-units; do
  awk -v control="$control" '
    /^Array Encoding::GetChars\(const Array &bytes, Integer / { chars = 1 }
    /^Array Encoding::GetBytes/ { chars = 0 }
    control == "utf8-passthrough" && /AppendUtf8\(out, NextUtf8\(bytes, position, false\)\);/ {
      $0 = "      out += bytes[position++];"; changed++
    }
    control == "surrogate-validation" && /IsLowSurrogate\(Utf16Unit\(bytes, position\)\)\)/ {
      sub(/IsLowSurrogate\(Utf16Unit\(bytes, position\)\)/, "true"); changed++
    }
    control == "utf16-tail" && /if \(position < bytes.size\(\)\) \{ AppendUtf8\(out, kUnicodeReplacement\); \}/ && changed == 0 {
      $0 = ""; changed++
    }
    control == "scalar-range" && /if \(code > kLastScalar/ {
      sub(/code > kLastScalar/, "code > kLastScalar + 1U"); changed++
    }
    control == "char-units" && chars && /if \(code >= kSurrogateBase\)/ {
      sub(/code >= kSurrogateBase/, "code > static_cast<std::int32_t>(kLastScalar)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/Encoding.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'encoding: %s escaped the Unicode gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sha256sum src/net/Encoding.cpp include/dotnet/Encoding.h test/gate/EncodingGate.cpp \
  test/gate/Reference.h "$B/libagiru_net.so" "$gate" > "$proof/inputs.sha256"
printf 'encoding: Unicode and UTF-16 char arrays pass; five compiled controls reject; %s\n' "$proof"
