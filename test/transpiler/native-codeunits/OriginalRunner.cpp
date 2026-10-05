#include "runtime/ErrorValue.h"
#include "runtime/NativeBase64.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/TextEncoding.h"

#include "Check.h"
#include "system/runtime/codeunit/Base64Convert.h"

#include <array>
#include <bit>
#include <cstdint>
#include <string>
#include <string_view>

namespace {

constexpr agiru::Integer kCodepage = 1252;
constexpr std::uint8_t kBinaryByte = 255;

template <typename Call> void Refuses(Call call, std::string_view signature) {
  std::string message;
  try {
    static_cast<void>(call());
  } catch (const agiru::Error &error) { message = error.what(); }
  CHECK_TRUE("original native overload refuses rather than returning empty success",
             message.contains("codeunit 2000000024 System.Runtime.Base64Convert.") &&
                 message.contains(signature) && message.contains("has no native implementation"));
}

void OriginalOverloadsRetainTheirContracts() {
  agiru::System::Runtime::Base64Convert_Codeunit unit;
  agiru::Blob input;
  input.Set({'A', 0, kBinaryByte});
  auto stream = input.CreateInStream();
  agiru::Blob destination;
  destination.Set({kBinaryByte});
  auto output = destination.CreateOutStream();
  const auto position = stream.Position();
  Refuses([&] { return unit.ToBase64(stream, false); },
          "ToBase64(InStream: InStream; InsertLineBreaks: Boolean): Text");
  Refuses([&] { unit.ToBase64(stream, false, output); },
          "ToBase64(InStream: InStream; InsertLineBreaks: Boolean; OutStream: OutStream)");
  Refuses([&] { return unit.FromBase64(stream); }, "FromBase64(InStream: InStream): Text");
  Refuses([&] { unit.FromBase64(stream, output); },
          "FromBase64(InStream: InStream; OutStream: OutStream)");
  CHECK_TRUE("all unbound original overloads leave the output untouched",
             destination.Length() == 1 && destination.Bytes().front() == kBinaryByte);
  CHECK_TRUE("unbound original overloads do not move input", stream.Position() == position);
}

std::string Bytes(const agiru::Blob &blob) {
  const auto &bytes = blob.Bytes();
  if (bytes.empty()) { return {}; }
  return {reinterpret_cast<const char *>(bytes.data()), bytes.size()};
}

void TextBindings() {
  using Encoding = agiru::TextEncoding;
  agiru::System::Runtime::Base64Convert_Codeunit unit;

  struct Sample {
    Encoding encoding;
    agiru::Integer page;
    std::string_view text;
    std::string_view base64;
  };

  constexpr std::array samples{
      Sample{.encoding = Encoding::UTF8, .page = 0, .text = "ÆØÅæøå", .base64 = "w4bDmMOFw6bDuMOl"},
      Sample{.encoding = Encoding::UTF16,
             .page = kCodepage,
             .text = "ÆØÅæøå",
             .base64 = "xgDYAMUA5gD4AOUA"},
      Sample{
          .encoding = Encoding::Windows, .page = kCodepage, .text = "ÆØÅæøå", .base64 = "xtjF5vjl"},
      Sample{
          .encoding = Encoding::MSDos, .page = kCodepage, .text = "ÆØÅæøå", .base64 = "xtjF5vjl"},
      Sample{.encoding = Encoding::UTF8, .page = -1, .text = "input", .base64 = "aW5wdXQ="},
      Sample{
          .encoding = Encoding::UTF16, .page = -1, .text = "input", .base64 = "aQBuAHAAdQB0AA=="},
      Sample{.encoding = Encoding::UTF8, .page = 0, .text = {}, .base64 = {}},
      Sample{.encoding = Encoding::UTF16, .page = 0, .text = {}, .base64 = {}}};
  for (const auto &sample : samples) {
    const agiru::Text<0> text{sample.text};
    const agiru::Text<0> base64{sample.base64};
    CHECK_TEXT("original native text encoder preserves the selected encoding",
               unit.ToBase64(text, false, sample.encoding, sample.page),
               sample.base64);
    CHECK_TEXT("original native text decoder preserves the selected encoding",
               unit.FromBase64(base64, sample.encoding, sample.page),
               sample.text);
    agiru::Blob blob;
    agiru::OutStream output;
    blob.CreateOutStream(output, Encoding::UTF16);
    output.WriteBytes("prefix:");
    unit.ToBase64(text, false, sample.encoding, sample.page, output);
    CHECK_TEXT("native encoder appends raw ASCII despite output encoding",
               Bytes(blob),
               "prefix:" + std::string(sample.base64));
    blob.Set({});
    unit.FromBase64(base64, sample.encoding, sample.page, output);
    const std::string decoded = Bytes(blob);
    blob.Set({});
    unit.FromBase64(base64, output);
    CHECK_TEXT("both native text-output decoders use the same raw bytes", Bytes(blob), decoded);
  }
}

template <typename Call> void Fails(Call call, std::string_view diagnostic) {
  std::string error;
  try {
    static_cast<void>(call());
  } catch (const agiru::Error &failure) { error = failure.what(); }
  CHECK_TRUE("native binding retains explicit invalid/unsupported diagnostics",
             error.contains(diagnostic));
}

void OutputAndErrors() {
  using Encoding = agiru::TextEncoding;
  agiru::System::Runtime::Base64Convert_Codeunit unit;
  const agiru::Text<0> binaryBase64{"QQD/"};
  const std::string binary("A\0\xFF", 3);
  agiru::Blob blob;
  agiru::OutStream output;
  blob.CreateOutStream(output, Encoding::UTF16);
  unit.FromBase64(binaryBase64, output);
  CHECK_TEXT("original native raw decode preserves binary zeros/high bytes", Bytes(blob), binary);
  blob.Set({});
  constexpr auto invalidEncoding = std::bit_cast<Encoding>(std::int32_t{-1});
  unit.FromBase64(binaryBase64, invalidEncoding, -1, output);
  CHECK_TEXT("native output decoder ignores even invalid encoding/page", Bytes(blob), binary);
  const auto before = Bytes(blob);
  Fails([&] { unit.FromBase64(agiru::Text<0>{"QQ==!"}, output); }, "invalid Base64");
  CHECK_TEXT("malformed native decode leaves output unchanged", Bytes(blob), before);
  Fails([&] { return unit.ToBase64(agiru::Text<0>{"x"}, false, invalidEncoding, 0); },
        "invalid native Base64 text encoding");
  CHECK_TEXT("unknown decode option uses the native UTF-8 fallback",
             unit.FromBase64(agiru::Text<0>{"w4Y="}, invalidEncoding, -1),
             "Æ");
  for (const auto encoding : {Encoding::Windows, Encoding::MSDos}) {
    Fails([&] { return unit.ToBase64(agiru::Text<0>{"x"}, false, encoding, 0); },
          "locale codepage");
    Fails([&] { return unit.FromBase64(agiru::Text<0>{"eA=="}, encoding, 0); }, "locale codepage");
  }
  Fails([&] { return unit.ToBase64(agiru::Text<0>{"x"}, false, Encoding::Windows, -1); },
        "Encoding.GetEncoding");
  CHECK_TEXT("empty encode bypasses invalid option/page validation",
             unit.ToBase64(agiru::Text<0>{}, false, invalidEncoding, -1),
             "");
  CHECK_TEXT("empty decode bypasses invalid page validation",
             unit.FromBase64(agiru::Text<0>{}, Encoding::Windows, -1),
             "");
  CHECK_TEXT("native decoded BOM remains an AL character",
             unit.FromBase64(agiru::Text<0>{"77u/eA=="}, Encoding::UTF8, 0),
             "\xEF\xBB\xBFx");
  blob.Set({});
  const agiru::Text<0> longText{std::string(58, 'A')};
  const std::string line =
      "QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFB";
  CHECK_TEXT("native text encoding inserts the original 76-column CRLF",
             unit.ToBase64(longText, true, Encoding::UTF8, 0),
             line + "\r\nQQ==");
  unit.ToBase64(longText, true, Encoding::UTF8, 0, output);
  CHECK_TEXT(
      "native output text encoding uses the same line breaks", Bytes(blob), line + "\r\nQQ==");
}

void OutputLimits() {
  using Encoding = agiru::TextEncoding;
  agiru::System::Runtime::Base64Convert_Codeunit unit;
  agiru::OutStream unbound;
  Fails([&] { unit.ToBase64(agiru::Text<0>{}, false, Encoding::UTF8, 0, unbound); }, "OutStream");
  Fails([&] { unit.FromBase64(agiru::Text<0>{}, unbound); }, "OutStream");
  Fails([&] { unit.FromBase64(agiru::Text<0>{" "}, unbound); }, "OutStream");
  const agiru::Text<0> boundary{std::string(agiru::kNativeBase64BufferCharacters, 'A')};
  agiru::Blob blob;
  auto output = blob.CreateOutStream();
  unit.ToBase64(boundary, false, Encoding::UTF8, 0, output);
  CHECK_TRUE("original native memory boundary is included, not truncated",
             blob.Length() == ((agiru::kNativeBase64BufferCharacters + 2) / 3) * 4);
  const auto before = blob.Length();
  const agiru::Text<0> above{std::string(agiru::kNativeBase64BufferCharacters + 1, 'A')};
  Fails([&] { unit.ToBase64(above, false, Encoding::UTF8, 0, output); }, "transform-block output");
  Fails([&] { unit.FromBase64(above, output); }, "transform-block output");
  CHECK_TRUE("unqualified native transform branches refuse before output changes",
             blob.Length() == before);
}

}

int main() {
  return gate::Run("Original Native Base64 Bindings", [] {
    OriginalOverloadsRetainTheirContracts();
    TextBindings();
    OutputAndErrors();
    OutputLimits();
  });
}
