#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/TextEncoding.h"

#include "Check.h"
#include "system/runtime/codeunit/Base64Convert.h"

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
  agiru::Text<0> text{"input"};
  agiru::Blob input;
  input.Set({'A', 0, kBinaryByte});
  auto stream = input.CreateInStream();
  agiru::Blob destination;
  destination.Set({kBinaryByte});
  auto output = destination.CreateOutStream();
  const auto position = stream.Position();
  Refuses([&] { return unit.ToBase64(text, false, agiru::TextEncoding::UTF8, kCodepage); },
          "ToBase64(String: Text; InsertLineBreaks: Boolean; TextEncoding: TextEncoding; Codepage: "
          "Integer): Text");
  Refuses([&] { unit.ToBase64(text, false, agiru::TextEncoding::UTF8, kCodepage, output); },
          "Codepage: Integer; OutStream: OutStream)");
  Refuses([&] { return unit.ToBase64(stream, false); },
          "ToBase64(InStream: InStream; InsertLineBreaks: Boolean): Text");
  Refuses([&] { unit.ToBase64(stream, false, output); },
          "ToBase64(InStream: InStream; InsertLineBreaks: Boolean; OutStream: OutStream)");
  Refuses([&] { return unit.FromBase64(text, agiru::TextEncoding::UTF8, kCodepage); },
          "FromBase64(Base64String: Text; TextEncoding: TextEncoding; CodePage: Integer): Text");
  Refuses([&] { unit.FromBase64(text, agiru::TextEncoding::UTF8, kCodepage, output); },
          "CodePage: Integer; OutStream: OutStream)");
  Refuses([&] { unit.FromBase64(text, output); },
          "FromBase64(Base64String: Text; OutStream: OutStream)");
  Refuses([&] { return unit.FromBase64(stream); }, "FromBase64(InStream: InStream): Text");
  Refuses([&] { unit.FromBase64(stream, output); },
          "FromBase64(InStream: InStream; OutStream: OutStream)");
  CHECK_TRUE("all unbound original overloads leave the output untouched",
             destination.Length() == 1 && destination.Bytes().front() == kBinaryByte);
  CHECK_TRUE("unbound original overloads do not move input", stream.Position() == position);
}

}

int main() {
  return gate::Run("Original Native Base64 Declarations", OriginalOverloadsRetainTheirContracts);
}
