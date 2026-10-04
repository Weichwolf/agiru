#include "runtime/ErrorValue.h"
#include "type/Base64.h"
#include "type/Blob.h"
#include "type/Stream.h"

#include "Check.h"
#include "Reference.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <utility>

namespace {

constexpr std::size_t kLineBytes = 57;
constexpr std::size_t kLineColumns = 76;
constexpr std::size_t kLargeInput = 16385;
constexpr std::size_t kByteValues = 256;

std::string_view Bytes(const agiru::Blob &blob) {
  if (!blob.HasValue()) { return {}; }
  return {reinterpret_cast<const char *>(blob.Bytes().data()), blob.Length()};
}

void KnownValues() {
  constexpr std::array<std::pair<std::string_view, std::string_view>, 9> cases{{
      {"", ""},
      {"f", "Zg=="},
      {"fo", "Zm8="},
      {"foo", "Zm9v"},
      {"foob", "Zm9vYg=="},
      {"fooba", "Zm9vYmE="},
      {"foobar", "Zm9vYmFy"},
      {std::string_view("\0\xff\x80", 3), "AP+A"},
      {std::string_view("\xc6\0\xd8\0\xc5\0\xe6\0\xf8\0\xe5\0", 12), "xgDYAMUA5gD4AOUA"},
  }};
  for (const auto &[bytes, text] : cases) {
    CHECK_TEXT("known raw bytes encode exactly", agiru::EncodeBase64(bytes), text);
    CHECK_TEXT("known encoded bytes decode exactly", agiru::DecodeBase64(text), bytes);
    agiru::Blob encoded;
    auto encodeOut = encoded.CreateOutStream();
    agiru::EncodeBase64(bytes, encodeOut);
    CHECK_TEXT("raw encoded stream matches text, without terminator", Bytes(encoded), text);
    agiru::Blob decoded;
    auto decodeOut = decoded.CreateOutStream();
    agiru::DecodeBase64(text, decodeOut);
    CHECK_TEXT("raw decoded stream preserves binary bytes", Bytes(decoded), bytes);
  }
}

void LineBreaks() {
  const std::string line(kLineBytes, 'a');
  const std::string plain = agiru::EncodeBase64(line);
  CHECK_TRUE("57 bytes produce exactly 76 columns", plain.size() == kLineColumns);
  CHECK_TEXT("a full final line has no trailing CRLF", agiru::EncodeBase64(line, true), plain);
  CHECK_TEXT("the next group begins after CRLF",
             agiru::EncodeBase64(line + "a", true),
             plain + "\r\nYQ==");
  CHECK_TEXT("two full lines have only one separating CRLF",
             agiru::EncodeBase64(line + line, true),
             plain + "\r\n" + plain);
  CHECK_TEXT("a padded group is formatted as one whole group",
             agiru::EncodeBase64(line + "ab", true),
             plain + "\r\nYWI=");
}

void DecoderRules() {
  CHECK_TEXT(
      "only ignored whitespace can represent empty bytes", agiru::DecodeBase64(" \r\n\t"), "");
  CHECK_TEXT("whitespace may separate padding", agiru::DecodeBase64("T Q=\n=\r\t"), "M");
  CHECK_TEXT(
      "unused bits in double padding match CLR acceptance", agiru::DecodeBase64("TR=="), "M");
  CHECK_TEXT(
      "unused bits in single padding match CLR acceptance", agiru::DecodeBase64("TWF="), "Ma");
  constexpr std::array<std::string_view, 16> invalid{{
      "TQ",
      "TQ=",
      "T===",
      "TQ===",
      "=TQ=",
      "TQ==TQ==",
      "TQ==A",
      "TQ==\v",
      "TQ==\f",
      "TQ==\xc2\xa0",
      std::string_view("TQ==\0", 5),
      "!!!!",
      "TQ=_",
      "TQ-=",
      "AAA",
      "AAAA=",
  }};
  for (const auto text : invalid) {
    bool refused = false;
    try {
      static_cast<void>(agiru::DecodeBase64(text));
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("invalid Base64 refuses instead of returning partial bytes", refused);
  }
  agiru::Blob blob;
  auto out = blob.CreateOutStream();
  out.WriteBytes("prefix");
  bool refused = false;
  try {
    agiru::DecodeBase64(std::string(kLargeInput, 'A') + "!", out);
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("invalid input is rejected before block writes", refused);
  CHECK_TEXT("a late decoder error leaves the existing output unchanged", Bytes(blob), "prefix");
}

void BoundedOutputAndAliasing() {
  std::string binary;
  for (std::size_t index = 0; index < kLargeInput; ++index) {
    binary.push_back(static_cast<char>(index % kByteValues));
  }
  for (const bool lines : {false, true}) {
    const std::string text = agiru::EncodeBase64(binary, lines);
    agiru::Blob blob;
    auto out = blob.CreateOutStream();
    out.WriteBytes("prefix");
    CHECK_TRUE("independent byte storage is not borrowed", !out.Borrows(binary));
    agiru::EncodeBase64(binary, out, lines);
    CHECK_TEXT("block encoding retains prefix and line state", Bytes(blob), "prefix" + text);
    agiru::Blob decoded;
    auto decodeOut = decoded.CreateOutStream();
    decodeOut.WriteBytes("prefix");
    agiru::DecodeBase64(text, decodeOut);
    CHECK_TEXT("block decoding retains prefix and binary bytes", Bytes(decoded), "prefix" + binary);
    CHECK_TEXT("all byte values round trip through text output", agiru::DecodeBase64(text), binary);
    agiru::Blob encodeAlias;
    auto aliasOut = encodeAlias.CreateOutStream();
    aliasOut.WriteBytes(binary);
    CHECK_TRUE("self input is recognized before its first output block",
               aliasOut.Borrows(Bytes(encodeAlias)));
    agiru::EncodeBase64(Bytes(encodeAlias), aliasOut, lines);
    CHECK_TEXT("encoding aliased storage preserves the entire original input",
               Bytes(encodeAlias),
               binary + text);
    agiru::Blob decodeAlias;
    auto aliasDecodeOut = decodeAlias.CreateOutStream();
    aliasDecodeOut.WriteBytes(text);
    agiru::DecodeBase64(Bytes(decodeAlias), aliasDecodeOut);
    CHECK_TEXT("decoding aliased storage preserves the entire original input",
               Bytes(decodeAlias),
               text + binary);
  }
  agiru::OutStream unbound;
  CHECK_TRUE("an empty view borrows nothing", !unbound.Borrows(""));
  bool refused = false;
  try {
    agiru::EncodeBase64("a", unbound);
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("nonempty output requires a bound destination", refused);
}

void ReferenceRow(const std::string &line) {
  const auto fields = gate::ReferenceFields<4>(line);
  const std::string input = gate::Unhex(fields[1]);
  agiru::Blob blob;
  auto out = blob.CreateOutStream();
  if (fields[0] == "E" && (fields[2] == "0" || fields[2] == "1")) {
    const bool lines = fields[2] == "1";
    const std::string expected = gate::Unhex(fields[3]);
    CHECK_TEXT("original native byte encoder matches string output",
               agiru::EncodeBase64(input, lines),
               expected);
    agiru::EncodeBase64(input, out, lines);
    CHECK_TEXT("original native byte encoder matches raw stream output", Bytes(blob), expected);
    return;
  }
  if (fields[0] != "D" || fields[2] != "0") {
    throw agiru::Error("unknown Base64 reference operation");
  }
  std::string decoded;
  bool refused = false;
  try {
    decoded = agiru::DecodeBase64(input);
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("CLR decoder and string output agree on invalid input",
             refused == (fields[3] == "FORMAT"));
  refused = false;
  try {
    agiru::DecodeBase64(input, out);
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("CLR decoder and stream output agree on invalid input",
             refused == (fields[3] == "FORMAT"));
  if (fields[3] == "FORMAT") {
    CHECK_TRUE("invalid reference input writes no bytes", !blob.HasValue());
    return;
  }
  const std::string expected = gate::Unhex(fields[3]);
  CHECK_TEXT("CLR decoder matches binary string output", decoded, expected);
  CHECK_TEXT("CLR decoder matches raw stream output", Bytes(blob), expected);
}

void Reference(const char *path) {
  std::ifstream fixture(path);
  if (!fixture) { throw agiru::Error("cannot open Base64 reference fixture"); }
  std::string line;
  std::size_t rows = 0;
  while (std::getline(fixture, line)) {
    ReferenceRow(line);
    ++rows;
  }
  if (!fixture.eof()) { throw agiru::Error("cannot read Base64 reference fixture"); }
  CHECK_TRUE("the external reference population is nonempty", rows != 0);
  std::println("Base64 reference: {} row(s)", rows);
}

}

int main(int argc, char **argv) {
  return gate::Run("Base64", [&] {
    KnownValues();
    LineBreaks();
    DecoderRules();
    BoundedOutputAndAliasing();
    if (argc == 2) { Reference(argv[1]); }
    if (argc > 2) { throw agiru::Error("expected at most one Base64 reference fixture"); }
  });
}
