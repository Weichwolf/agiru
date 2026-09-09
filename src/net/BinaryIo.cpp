#include "dotnet/BinaryReader.h"
#include "dotnet/BinaryWriter.h"

#include "runtime/Error.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr unsigned kSevenBits = 0x7FU;
constexpr unsigned kCarryBit = 0x80U;
constexpr unsigned kBitsPerByte = 7U;
constexpr unsigned kLongestPrefix = 5U;

std::string LengthPrefix(std::size_t length) {
  std::string out;
  std::size_t left = length;
  do {
    const unsigned low = static_cast<unsigned>(left & kSevenBits);
    left >>= kBitsPerByte;
    out.push_back(static_cast<char>(left != 0 ? low | kCarryBit : low));
  } while (left != 0);
  return out;
}

}

void BinaryWriter::Write(std::string_view text) {
  if (output_ == nullptr) {
    throw Error("BinaryWriter.Write: the writer was never bound to an OutStream");
  }
  output_->WriteBytes(LengthPrefix(text.size()));
  output_->WriteBytes(text);
}

BinaryReader::Stream BinaryReader::BaseStream() const {
  if (input_ == nullptr) {
    throw Error("BinaryReader.BaseStream: the reader was never bound to an InStream");
  }
  return Stream(*input_);
}

::agiru::Text<0> BinaryReader::ReadString() {
  if (input_ == nullptr) {
    throw Error("BinaryReader.ReadString: the reader was never bound to an InStream");
  }
  std::size_t length = 0;
  unsigned shift = 0;
  for (unsigned taken = 0; taken < kLongestPrefix; ++taken) {
    const std::string byte = input_->ReadBytes(1);
    if (byte.empty()) { throw Error("BinaryReader.ReadString: the stream ends inside the length"); }
    const auto value = static_cast<unsigned>(static_cast<std::uint8_t>(byte.front()));
    length |= static_cast<std::size_t>(value & kSevenBits) << shift;
    if ((value & kCarryBit) == 0) { break; }
    shift += kBitsPerByte;
  }
  const std::string text = input_->ReadBytes(static_cast<Integer>(length));
  if (text.size() != length) {
    throw Error("BinaryReader.ReadString: the stream ends inside the string");
  }
  return ::agiru::Text<0>{std::string_view(text)};
}

}
