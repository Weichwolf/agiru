#include "type/Stream.h"

#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

namespace {

constexpr std::string_view kLineBreak = "\r\n";

}

void OutStream::RefuseTyped() {
  throw Error("a typed Write puts the platform's own binary layout into the stream, and this "
              "runtime does not have it. Only the text forms are here");
}

void InStream::RefuseTyped() {
  throw Error("a typed Read expects the platform's own binary layout in the stream, and this "
              "runtime does not have it. Only the text forms are here");
}

Integer OutStream::WriteText(std::string_view text) {
  std::vector<std::uint8_t> bytes = blob_->Bytes();
  for (const char c : text) { bytes.push_back(static_cast<std::uint8_t>(c)); }
  blob_->Set(std::move(bytes));
  return static_cast<Integer>(text.size());
}

Integer OutStream::WriteText() {
  return WriteText(kLineBreak);
}

Boolean InStream::EOS() const {
  return position_ >= blob_->Length();
}

Integer InStream::Length() const {
  return static_cast<Integer>(blob_->Length());
}

Integer InStream::ReadText(std::string &text, Integer length) {
  const std::size_t left =
      blob_->Length() - (position_ < blob_->Length() ? position_ : blob_->Length());
  const std::size_t want = length < 0 ? 0 : static_cast<std::size_t>(length);
  const std::size_t take = want < left ? want : left;
  text.assign(reinterpret_cast<const char *>(blob_->Bytes().data()) + position_, take);
  position_ += take;
  return static_cast<Integer>(take);
}

Integer InStream::ReadText(std::string &text) {
  return ReadText(text, static_cast<Integer>(blob_->Length() - position_));
}

}

namespace agiru {

OutStream Blob::CreateOutStream() {
  return OutStream{*this};
}

InStream Blob::CreateInStream() const {
  return InStream{*this};
}

}
