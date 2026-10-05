#include "dotnet/Convert.h"

#include "dotnet/Base64FormattingOptions.h"
#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Base64.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "ByteArray.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::size_t kBytesPerLine = 57;
constexpr std::size_t kLinesPerBlock = 64;
constexpr std::size_t kInputBlockBytes = kBytesPerLine * kLinesPerBlock;
static_assert(kInputBlockBytes % 3 == 0);

bool LineBreaks(Base64FormattingOptions options) {
  if (options.Ordinal() == Base64FormattingOptions::None().Ordinal()) { return false; }
  if (options.Ordinal() == Base64FormattingOptions::InsertLineBreaks().Ordinal()) { return true; }
  throw Error("Convert.ToBase64String: invalid formatting option");
}

}

std::string Convert::ToBase64String(const Array &bytes, Base64FormattingOptions options) {
  return ToBase64String(bytes, 0, bytes.Length(), options);
}

std::string Convert::ToBase64String(const Array &bytes,
                                    Integer offset,
                                    Integer length,
                                    Base64FormattingOptions options) {
  detail::ValidateByteRegion(bytes, offset, length);
  const bool lines = LineBreaks(options);
  std::array<unsigned char, kInputBlockBytes> block{};
  std::string result;
  Integer consumed = 0;
  while (consumed < length) {
    const auto count = std::min(static_cast<std::size_t>(length - consumed), block.size());
    detail::ReadByteBlock(bytes, offset + consumed, std::span(block).first(count));
    if (lines && consumed != 0) { result += "\r\n"; }
    result +=
        EncodeBase64(std::string_view(reinterpret_cast<const char *>(block.data()), count), lines);
    consumed += static_cast<Integer>(count);
  }
  return result;
}

Array Convert::FromBase64String(std::string_view text) {
  const std::string bytes = DecodeBase64(text);
  Array result;
  for (const char byte : bytes) { result.Add(Variant{Integer{static_cast<unsigned char>(byte)}}); }
  return result;
}

}
