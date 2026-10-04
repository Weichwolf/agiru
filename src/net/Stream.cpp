#include "type/Stream.h"

#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/TextEncoding.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

constexpr std::string_view kLineBreak = "\r\n";

}

Blob &OutStream::Bound() const {
  if (blob_ == nullptr) {
    throw Error("this OutStream was never given a source: AL declares the variable and "
                "`CreateOutStream` binds it");
  }
  return *blob_;
}

const Blob &InStream::Bound() const {
  if (blob_ == nullptr) {
    throw Error("this InStream was never given a source: AL declares the variable and "
                "`CreateInStream` binds it");
  }
  return *blob_;
}

void OutStream::RefuseTyped() {
  throw Error("a typed Write puts the platform's own binary layout into the stream, and this "
              "runtime does not have it. Only the text forms are here");
}

void InStream::RefuseTyped() {
  throw Error("a typed Read expects the platform's own binary layout in the stream, and this "
              "runtime does not have it. Only the text forms are here");
}

Integer OutStream::Append(std::string_view text, bool terminated) {
  auto &bytes = Bound().bytes_;
  const std::size_t maximum = bytes.max_size();
  const std::size_t terminator = terminated ? 1 : 0;
  if (terminator > maximum - bytes.size() || text.size() > maximum - bytes.size() - terminator) {
    throw Error("OutStream.Write exceeds byte storage capacity");
  }
  std::string owned;
  if (!text.empty() && !bytes.empty()) {
    const auto *first = reinterpret_cast<const char *>(bytes.data());
    const std::less<> before;
    if (!before(text.data(), first) && before(text.data(), first + bytes.size())) {
      owned = text;
      text = owned;
    }
  }
  const std::size_t oldSize = bytes.size();
  const std::size_t required = oldSize + text.size() + terminator;
  if (required > bytes.capacity()) {
    const std::size_t doubled = bytes.capacity() > maximum / 2 ? maximum : bytes.capacity() * 2;
    bytes.reserve(required > doubled ? required : doubled);
  }
  bytes.resize(required);
  if (!text.empty()) { std::memcpy(bytes.data() + oldSize, text.data(), text.size()); }
  if (terminated) { bytes.back() = 0; }
  return static_cast<Integer>(text.size() + terminator);
}

Integer OutStream::WriteTerminated(std::string_view text, Integer length) {
  const std::size_t want = length < 0 ? text.size() : static_cast<std::size_t>(length);
  return Append(text.substr(0, want < text.size() ? want : text.size()), true);
}

Integer OutStream::WriteText(std::string_view text) {
  return Append(text, false);
}

Integer OutStream::WriteBytes(std::string_view bytes) {
  return Append(bytes, false);
}

std::string InStream::ReadBytes(Integer count) {
  const std::vector<std::uint8_t> &bytes = Bound().Bytes();
  const std::size_t want = count < 0 ? 0 : static_cast<std::size_t>(count);
  const std::size_t end = position_ + want < bytes.size() ? position_ + want : bytes.size();
  std::string out;
  for (std::size_t at = position_; at < end; ++at) { out.push_back(static_cast<char>(bytes[at])); }
  position_ = end;
  return out;
}

Integer OutStream::WriteText() {
  return WriteText(kLineBreak);
}

Boolean InStream::EOS() const {
  return position_ >= Bound().Length();
}

Integer InStream::Length() const {
  return static_cast<Integer>(Bound().Length());
}

Integer InStream::ReadTerminated(std::string &into, Integer length) {
  const std::vector<std::uint8_t> &bytes = Bound().Bytes();
  const std::size_t end = bytes.size();
  const std::size_t want = length < 0 ? end : static_cast<std::size_t>(length);
  std::size_t at = position_;
  std::size_t taken = 0;
  while (at < end && taken < want && bytes[at] != 0) {
    into.push_back(static_cast<char>(bytes[at]));
    ++at;
    ++taken;
  }
  if (at < end && bytes[at] == 0) {
    ++at;
    ++taken;
  }
  position_ = at;
  return static_cast<Integer>(taken);
}

Integer InStream::ReadText(::agiru::Text<0> &text, Integer length) {
  const std::size_t left =
      Bound().Length() - (position_ < Bound().Length() ? position_ : Bound().Length());
  const std::size_t want = length < 0 ? 0 : static_cast<std::size_t>(length);
  const std::size_t take = want < left ? want : left;
  text = std::string_view(reinterpret_cast<const char *>(Bound().Bytes().data()) + position_, take);
  position_ += take;
  return static_cast<Integer>(take);
}

Integer InStream::ReadText(::agiru::Text<0> &text) {
  return ReadText(text, static_cast<Integer>(Bound().Length() - position_));
}

}

namespace agiru {

OutStream Blob::CreateOutStream() {
  return OutStream{*this};
}

void Blob::CreateOutStream(OutStream &into, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  into = OutStream(*this);
}

void Blob::CreateInStream(InStream &from, const TextEncoding &Encoding) const {
  static_cast<void>(Encoding);
  from = InStream(*this);
}

InStream Blob::CreateInStream() const {
  return InStream{*this};
}

std::string Blob::Export(std::string_view Name) {
  throw Error("Blob.Export(" + std::string(Name) + ") needs a client (board:0030)");
}

std::string Blob::Import(std::string_view Name) {
  throw Error("Blob.Import(" + std::string(Name) + ") needs a client (board:0030)");
}

}
