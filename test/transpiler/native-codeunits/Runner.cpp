#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"

#include "Check.h"
#include "system/fixture/codeunit/NativeFixture.h"

#include <array>
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

void UnavailableRecordFieldsCompileAndRefuse() {
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  constexpr std::array<std::string_view, 10> kMethods{"ModifyAll",
                                                      "LoadFields",
                                                      "GetRangeMin",
                                                      "GetRangeMax",
                                                      "GetFilter",
                                                      "GetAscending",
                                                      "CopyFilter",
                                                      "FieldActive",
                                                      "Relation",
                                                      "AreFieldsLoaded"};
  for (agiru::Integer operation = 0; operation < static_cast<agiru::Integer>(kMethods.size());
       ++operation) {
    agiru::Integer counter = kValue;
    std::string message;
    try {
      unit.UnavailableFields(operation, counter);
    } catch (const agiru::Error &error) { message = error.what(); }
    CHECK_TRUE("implicit unavailable fields compile but their operation refuses explicitly",
               message.contains("UnavailableRow." + std::string(kMethods[operation])));
    CHECK_TRUE("unavailable operations never reach subsequent AL effects", counter == 41);
  }
}

void UnavailableIndexedFieldsCompileAndRefuse() {
  constexpr std::array<std::string_view, 5> kMembers{
      "ArrayOnly", "NestedOnly", "MatrixOnly", "BracketOnly", "Validate"};
  constexpr agiru::Integer kIndexWithSideEffect = 3;
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  for (agiru::Integer operation = 0; operation < static_cast<agiru::Integer>(kMembers.size());
       ++operation) {
    agiru::Integer counter = kValue;
    std::string message;
    try {
      unit.UnavailableIndexedFields(operation, counter);
    } catch (const agiru::Error &error) { message = error.what(); }
    CHECK_TRUE("indexed unavailable members compile and refuse with their own identity",
               message.contains("IndexedRow." + std::string(kMembers[operation])));
    CHECK_TRUE("the index executes once and refusal precedes subsequent AL effects",
               counter == (operation == kIndexWithSideEffect ? 42 : 41));
  }
}

void UnavailableFieldOperationsCompileAndRefuse() {
  constexpr agiru::Integer kOperationCount = 6;
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  for (agiru::Integer operation = 0; operation < kOperationCount; ++operation) {
    agiru::Integer counter = kValue;
    std::string message;
    try {
      unit.UnavailableFieldOperations(operation, counter);
    } catch (const agiru::Error &error) { message = error.what(); }
    CHECK_TRUE("compound assignments and both Clear forms retain the missing field identity",
               message.contains("ArithmeticRow.Amount"));
    CHECK_TRUE("unavailable field operations refuse before later AL effects", counter == 41);
  }
}

void UnavailableNamedPagesCompileAndRefuse() {
  constexpr agiru::Integer kOperationCount = 6;
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  for (agiru::Integer operation = 0; operation < kOperationCount; ++operation) {
    agiru::Integer counter = kValue;
    std::string message;
    try {
      unit.UnavailablePage(operation, counter);
    } catch (const agiru::Error &error) { message = error.what(); }
    CHECK_TRUE("unselected Page Run and RunModal refuse with their original AL identity",
               message ==
                   "Page::Unselected Page names an object this run does not carry (board:0034)");
    CHECK_TRUE("unselected named pages never execute subsequent AL effects", counter == 41);
  }
}

void FieldNumbersRetainTheirIntegerContract() {
  constexpr agiru::Integer kOperationCount = 4;
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  for (agiru::Integer operation = 0; operation < kOperationCount; ++operation) {
    agiru::Integer counter = kValue;
    std::string message;
    try {
      unit.UnavailableFieldNumber(operation, counter);
    } catch (const agiru::Error &error) { message = error.what(); }
    CHECK_TRUE("unavailable FieldNo remains Integer-shaped with its original refusal identity",
               message.contains("NumberRow.FieldNo"));
    CHECK_TRUE("FieldNo refusal precedes overload invocation and subsequent AL effects",
               counter == 41);
  }
  CHECK_TRUE("selected Record FieldNo reaches the Integer overload",
             unit.AvailableFieldNumber() == 1);
  CHECK_TEXT("a Codeunit method named FieldNo retains its declared Text result",
             unit.OrdinaryFieldNumberName(),
             "own-field-number");
}

void UnavailablePartsRefuseWithoutDefaultAnswers() {
  constexpr agiru::Integer kOperationCount = 5;
  constexpr agiru::Integer kArgumentOperation = 3;
  agiru::System::Fixture::NativeFixture_Codeunit unit;
  for (agiru::Integer operation = 0; operation < kOperationCount; ++operation) {
    agiru::Integer counter = kValue;
    std::string message;
    try {
      unit.UnavailablePart(operation, counter);
    } catch (const agiru::Error &error) { message = error.what(); }
    CHECK_TRUE("an unavailable TestPage part refuses with its original control path",
               message.contains("Host.MissingPart.") && message.contains("board:0034"));
    CHECK_TRUE("part arguments execute once but later AL effects never execute",
               counter == (operation >= kArgumentOperation ? 42 : 41));
  }
  CHECK_TRUE("an untaken unavailable part branch does not refuse a working path",
             unit.UnavailablePartInUntakenBranch() == kValue);
}

}

int main() {
  return gate::Run("Generated Native Codeunit Refusals", [] {
    NativeMethodsRefuseBeforeEffects();
    UnavailableRecordFieldsCompileAndRefuse();
    UnavailableIndexedFieldsCompileAndRefuse();
    UnavailableFieldOperationsCompileAndRefuse();
    UnavailableNamedPagesCompileAndRefuse();
    FieldNumbersRetainTheirIntegerContract();
    UnavailablePartsRefuseWithoutDefaultAnswers();
  });
}
