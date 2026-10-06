#include "dotnet/StreamReader.h"

#include "dotnet/Encoding.h"
#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::int32_t kReadBlock = 65536;
constexpr std::int32_t kEnd = -1;

std::string Whole(InStream &stream) {
  std::string out;
  while (!stream.EOS()) {
    const std::string block = stream.ReadBytes(kReadBlock);
    if (block.empty()) { break; }
    out += block;
  }
  return out;
}

std::string WithoutMark(std::string text, const Encoding &encoding) {
  if (encoding.CodePage() == Encoding::kUtf16 && text.starts_with("\xFF\xFE")) { text.erase(0, 2); }
  if (encoding.CodePage() != Encoding::kUtf16 && text.starts_with("\xEF\xBB\xBF")) {
    text.erase(0, 3);
  }
  return text;
}

}

StreamReader StreamReader::Binder::operator()(InStream &stream, Boolean detect) const {
  return (*this)(stream, Encoding::Made(Encoding::kUtf8, static_cast<bool>(detect)));
}

StreamReader
StreamReader::Binder::operator()(InStream &stream, const Encoding &encoding, Boolean detect) const {
  static_cast<void>(detect);
  return (*this)(stream, encoding);
}

StreamReader StreamReader::Binder::operator()(std::string_view path) const {
  std::ifstream file{std::string(path), std::ios::binary};
  if (!file) { throw Error("StreamReader(" + std::string(path) + "): the file cannot be opened"); }
  const std::string bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  const Encoding encoding = Encoding::Made(Encoding::kUtf8, false);
  DecodedStreamReader out;
  out.text_ = encoding.Decode(WithoutMark(bytes, encoding));
  out.at_ = 0;
  out.encoding_ = encoding;
  return out;
}

StreamReader StreamReader::Binder::operator()(InStream &stream) const {
  return (*this)(stream, Encoding::Made(Encoding::kUtf8, false));
}

StreamReader StreamReader::Binder::operator()(InStream &stream, const Encoding &encoding) const {
  DecodedStreamReader out;
  out.text_ = encoding.Decode(WithoutMark(Whole(stream), encoding));
  out.at_ = 0;
  out.encoding_ = encoding;
  return out;
}

::agiru::Text<0> StreamReader::ReadLine() {
  if (at_ >= text_.size()) { return {}; }
  const std::size_t end = text_.find('\n', at_);
  std::string line = text_.substr(at_, end == std::string::npos ? std::string::npos : end - at_);
  if (!line.empty() && line.back() == '\r') { line.pop_back(); }
  at_ = end == std::string::npos ? text_.size() : end + 1;
  return ::agiru::Text<0>{line};
}

::agiru::Text<0> StreamReader::ReadToEnd() {
  const std::string rest = at_ >= text_.size() ? std::string{} : text_.substr(at_);
  at_ = text_.size();
  return ::agiru::Text<0>{rest};
}

Integer StreamReader::Read() {
  if (at_ >= text_.size()) { return kEnd; }
  return static_cast<std::int32_t>(static_cast<unsigned char>(text_[at_++]));
}

Integer StreamReader::Peek() const {
  if (at_ >= text_.size()) { return kEnd; }
  return static_cast<std::int32_t>(static_cast<unsigned char>(text_[at_]));
}

void StreamReader::Close() {
  text_.clear();
  at_ = 0;
}

}
