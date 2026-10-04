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
if [ "${AGIRU_CODEPAGE_REFERENCE+x}" = x ]; then
  "$gate" --codepage-reference "$AGIRU_CODEPAGE_REFERENCE" > "$proof/codepage-reference.log" 2>&1
  sha256sum "$AGIRU_CODEPAGE_REFERENCE" > "$proof/codepage-reference.sha256"
fi
if [ "${AGIRU_ASCII_REFERENCE+x}" = x ]; then
  "$gate" --ascii-reference "$AGIRU_ASCII_REFERENCE" > "$proof/ascii-reference.log" 2>&1
  sha256sum "$AGIRU_ASCII_REFERENCE" > "$proof/ascii-reference.sha256"
fi
if [ "${AGIRU_LATIN1_REFERENCE+x}" = x ]; then
  "$gate" --latin1-reference "$AGIRU_LATIN1_REFERENCE" > "$proof/latin1-reference.log" 2>&1
  sha256sum "$AGIRU_LATIN1_REFERENCE" > "$proof/latin1-reference.sha256"
fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/net --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in utf8-passthrough surrogate-validation utf16-tail scalar-range char-units supplementary-fallback latin-alias utf8-preamble unknown-page; do
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
    control == "supplementary-fallback" && /out.append\(2, kReplacement\);/ {
      sub(/append\(2,/, "append(1,"); changed++
    }
    control == "latin-alias" && /return GetEncoding\(Integer\{kLatin1\}\);/ {
      sub(/kLatin1/, "kWindows1252"); changed++
    }
    control == "utf8-preamble" && /if \(page == kUtf8\) \{ return UTF8\(\); \}/ {
      sub(/return UTF8\(\);/, "return Made(kUtf8, false);"); changed++
    }
    control == "unknown-page" && /if \(FindSingleByteCodePage\(page\) != nullptr\)/ {
      sub(/FindSingleByteCodePage\(page\) != nullptr/, "page != kUnset"); changed++
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
for control in windows-decoder windows-best-fit latin-best-fit; do
  awk -v control="$control" '
    /^constexpr std::array<std::uint16_t,/ { decoder = 1 }
    control == "windows-decoder" && decoder && /0x20AC/ {
      sub(/0x20AC/, "0x0080"); changed++
    }
    control == "windows-best-fit" && /\.unit = 0x0301, \.byte = 0xB4/ {
      sub(/\.unit = 0x0301, \.byte = 0xB4/, ".unit = 0x0301, .byte = 0x3F"); changed++
    }
    control == "latin-best-fit" && /\.unit = 0x2018, \.byte = 0x27/ {
      sub(/\.unit = 0x2018, \.byte = 0x27/, ".unit = 0x2018, .byte = 0x3F"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/CodePage.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'encoding: %s escaped the codepage gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sha256sum src/net/Encoding.cpp src/net/CodePage.cpp src/net/CodePage.h \
  include/dotnet/Encoding.h test/gate/EncodingGate.cpp \
  test/gate/Reference.h "$B/libagiru_net.so" "$gate" > "$proof/inputs.sha256"
printf 'encoding: Unicode, codepages and factories pass; twelve compiled controls reject; %s\n' "$proof"
