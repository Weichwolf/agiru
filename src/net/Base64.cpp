#include "type/Base64.h"

#include "runtime/ErrorValue.h"
#include "type/Stream.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace agiru {
namespace {

constexpr std::string_view kAlphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr std::size_t kLineWidth = 76;
constexpr std::size_t kScratchSize = 4096;
constexpr std::size_t kInputGroup = 3;
constexpr std::size_t kOutputGroup = 4;
constexpr int kPadding = 64;
constexpr int kWhitespace = -2;
constexpr int kInvalid = -1;
constexpr unsigned kSextetBits = 6;
constexpr unsigned kByteBits = 8;
constexpr unsigned kSextetMask = 63;
static_assert(kScratchSize >= kOutputGroup);

constexpr auto kValues = [] {
  std::array<int, 256> result{};
  result.fill(kInvalid);
  for (std::size_t at = 0; at < kAlphabet.size(); ++at) {
    result[static_cast<unsigned char>(kAlphabet[at])] = static_cast<int>(at);
  }
  result['='] = kPadding;
  for (const char byte : std::string_view(" \t\r\n")) {
    result[static_cast<unsigned char>(byte)] = kWhitespace;
  }
  return result;
}();

class Output {
public:
  explicit Output(std::string &text) : text_(&text) {}

  explicit Output(OutStream &stream) : stream_(&stream) {}

  void Put(std::string_view bytes) {
    if (text_ != nullptr) {
      text_->append(bytes);
      return;
    }
    if (bytes.size() > buffer_.size() - used_) { Flush(); }
    std::memcpy(buffer_.data() + used_, bytes.data(), bytes.size());
    used_ += bytes.size();
  }

  void Flush() {
    if (used_ == 0) { return; }
    stream_->WriteBytes(std::string_view(buffer_.data(), used_));
    used_ = 0;
  }

private:
  std::string *text_ = nullptr;
  OutStream *stream_ = nullptr;
  std::array<char, kScratchSize> buffer_{};
  std::size_t used_ = 0;
};

std::size_t EncodedSize(std::size_t bytes, bool lines, std::size_t maximum) {
  const std::size_t groups = bytes / kInputGroup + (bytes % kInputGroup != 0 ? 1 : 0);
  if (groups > maximum / kOutputGroup) { throw Error("Base64 output exceeds text capacity"); }
  const std::size_t characters = groups * kOutputGroup;
  const std::size_t breaks = lines && characters != 0 ? (characters - 1) / kLineWidth : 0;
  if (breaks > (maximum - characters) / 2) { throw Error("Base64 output exceeds text capacity"); }
  return characters + breaks * 2;
}

void Encode(Output &output, std::string_view bytes, bool lines) {
  std::size_t column = 0;
  for (std::size_t at = 0; at < bytes.size();) {
    if (lines && column == kLineWidth) {
      output.Put("\r\n");
      column = 0;
    }
    const std::size_t left = bytes.size() - at;
    const std::size_t take = left < kInputGroup ? left : kInputGroup;
    std::uint32_t value = 0;
    for (std::size_t byte = 0; byte < kInputGroup; ++byte) {
      value <<= kByteBits;
      if (byte < take) { value |= static_cast<unsigned char>(bytes[at + byte]); }
    }
    std::array<char, kOutputGroup> group{};
    for (std::size_t index = 0; index < group.size(); ++index) {
      const auto shift = static_cast<unsigned>((group.size() - index - 1) * kSextetBits);
      group[index] = index <= take ? kAlphabet[(value >> shift) & kSextetMask] : '=';
    }
    output.Put(std::string_view(group.data(), group.size()));
    column += kOutputGroup;
    at += take;
  }
  output.Flush();
}

[[noreturn]] void Invalid() {
  throw Error("invalid Base64 input: alphabet, padding or incomplete group");
}

std::size_t Decode(std::string_view text, Output *output) {
  std::array<int, kOutputGroup> group{};
  std::size_t used = 0;
  std::size_t size = 0;
  bool ended = false;
  for (const unsigned char byte : text) {
    const int value = kValues[byte];
    if (value == kWhitespace) { continue; }
    if (value == kInvalid || ended) { Invalid(); }
    group[used++] = value;
    if (used != group.size()) { continue; }
    if (group[0] == kPadding || group[1] == kPadding ||
        (group[2] == kPadding && group[3] != kPadding)) {
      Invalid();
    }
    const std::size_t count = group[2] == kPadding ? 1 : (group[3] == kPadding ? 2 : 3);
    std::uint32_t packed = 0;
    for (const int sextet : group) {
      packed = (packed << kSextetBits) | static_cast<unsigned>(sextet & kSextetMask);
    }
    std::array<char, kInputGroup> bytes{};
    for (std::size_t index = 0; index < count; ++index) {
      const auto shift = static_cast<unsigned>((bytes.size() - index - 1) * kByteBits);
      bytes[index] = static_cast<char>(packed >> shift);
    }
    if (output != nullptr) { output->Put(std::string_view(bytes.data(), count)); }
    size += count;
    ended = count < kInputGroup;
    used = 0;
  }
  if (used != 0) { Invalid(); }
  if (output != nullptr) { output->Flush(); }
  return size;
}

}

std::string EncodeBase64(std::string_view bytes, bool lineBreaks) {
  std::string result;
  result.reserve(EncodedSize(bytes.size(), lineBreaks, result.max_size()));
  Output output(result);
  Encode(output, bytes, lineBreaks);
  return result;
}

std::string DecodeBase64(std::string_view text) {
  const std::size_t size = Decode(text, nullptr);
  std::string result;
  result.reserve(size);
  Output output(result);
  Decode(text, &output);
  return result;
}

void EncodeBase64(std::string_view bytes, OutStream &output, bool lineBreaks) {
  std::string owned;
  if (output.Borrows(bytes)) {
    owned = bytes;
    bytes = owned;
  }
  Output sink(output);
  Encode(sink, bytes, lineBreaks);
}

void DecodeBase64(std::string_view text, OutStream &output) {
  Decode(text, nullptr);
  std::string owned;
  if (output.Borrows(text)) {
    owned = text;
    text = owned;
  }
  Output sink(output);
  Decode(text, &sink);
}

}
