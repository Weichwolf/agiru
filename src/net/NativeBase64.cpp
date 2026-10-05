#include "runtime/NativeBase64.h"

#include "dotnet/Encoding.h"
#include "runtime/ErrorValue.h"
#include "type/Base64.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/TextEncoding.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace agiru {
namespace {

dotnet::Encoding NativeEncoding(TextEncoding encoding, std::int32_t codepage, bool decoding) {
  switch (encoding) {
    case TextEncoding::UTF8: return dotnet::Encoding::UTF8();
    case TextEncoding::UTF16: return dotnet::Encoding::Unicode();
    case TextEncoding::MSDos:
    case TextEncoding::Windows:
      if (codepage == 0) { throw Error("native Base64 locale codepage is unavailable"); }
      return dotnet::Encoding::GetEncoding(codepage);
  }
  if (decoding) { return dotnet::Encoding::UTF8(); }
  throw Error("invalid native Base64 text encoding");
}

}

std::string
NativeToBase64(std::string_view text, bool lines, TextEncoding encoding, std::int32_t codepage) {
  if (text.empty()) { return {}; }
  return EncodeBase64(NativeEncoding(encoding, codepage, false).Encode(text), lines);
}

void NativeToBase64(std::string_view text,
                    bool lines,
                    TextEncoding encoding,
                    std::int32_t codepage,
                    OutStream &output) {
  if (text.empty()) {
    output.WriteBytes({});
    return;
  }
  if (detail::Utf16Length(text) > kNativeBase64BufferCharacters) {
    throw Error("native Base64 transform-block output is unavailable");
  }
  EncodeBase64(NativeEncoding(encoding, codepage, false).Encode(text), output, lines);
}

std::string NativeFromBase64(std::string_view text, TextEncoding encoding, std::int32_t codepage) {
  if (text.empty()) { return {}; }
  const std::string bytes = DecodeBase64(text);
  return NativeEncoding(encoding, codepage, true).Decode(bytes);
}

void NativeFromBase64(std::string_view text, OutStream &output) {
  if (detail::Utf16Length(text) > kNativeBase64BufferCharacters) {
    throw Error("native Base64 transform-block output is unavailable");
  }
  DecodeBase64(text, output);
  output.WriteBytes({});
}

}
