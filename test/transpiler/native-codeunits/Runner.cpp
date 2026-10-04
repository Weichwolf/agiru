#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"

#include "Check.h"
#include "system/fixture/codeunit/NativeFixture.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace {

constexpr agiru::Integer kValue = 7;
constexpr std::uint8_t kBinaryByte = 255;

template <typename Call> std::string Refuses(Call call, std::string_view signature) {
  std::string message;
  try {
    static_cast<void>(call());
  } catch (const agiru::Error &error) { message = error.what(); }
  CHECK_TRUE("unbound Native refuses with its original typed signature",
             message.contains(signature));
  CHECK_TRUE("native refusal is not reported as a missing .NET capability",
             message.contains("codeunit 50311 System.Fixture.NativeFixture.") &&
                 message.contains("has no native implementation (board:0034)"));
  return message;
}

void NativeMethodsRefuseBeforeEffects() {
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  Refuses([&unit] { unit.Empty(); }, "Empty()");
  const auto integerError =
      Refuses([&unit] { return unit.Read(kValue); }, "Read(Value: Integer): Integer");
  const auto textError =
      Refuses([&unit] { return unit.Read(agiru::Text<0>{"value"}); }, "Read(Value: Text): Text");
  CHECK_TRUE("overloaded native refusals retain distinct source signatures",
             integerError != textError);
  Refuses([&unit] { return unit.Named(); }, "Named() Result: Text");
  agiru::Integer value = kValue;
  agiru::Blob blob;
  blob.Set({0, kBinaryByte});
  auto output = blob.CreateOutStream();
  Refuses([&] { unit.Write(value, output); }, "Write(var Value: Integer; Output: OutStream)");
  CHECK_TRUE("unbound Native does not change var arguments", value == kValue);
  CHECK_TRUE("unbound Native does not write into borrowed streams",
             blob.Length() == 2 && blob.Bytes().front() == 0 && blob.Bytes().back() == kBinaryByte);
  Refuses([&] { unit.Publish(value); }, "Publish(var Value: Integer)");
  CHECK_TRUE("native publishers do not masquerade as successful events", value == kValue);
  Refuses([&unit] { unit.CallHidden(); }, "Hidden()");
  unit.OrdinaryEmpty();
  CHECK_TRUE("ordinary AL code remains executable", unit.Ordinary() == kValue);
}

}

int main() {
  return gate::Run("Generated Native Codeunit Refusals", NativeMethodsRefuseBeforeEffects);
}
