#include "dotnet/Refused.h"
#include "dotnet/Type.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Option.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <string_view>
#include <type_traits>

namespace {

static_assert(std::is_const_v<decltype(agiru::dotnet::Type::GetField)>,
              "refusal descriptors are immutable");
static_assert(std::is_const_v<decltype(agiru::dotnet::Type::MakeGenericType)>,
              "refusal descriptors are immutable");
static_assert(
    !std::is_assignable_v<decltype((agiru::dotnet::Type::GetField)), agiru::dotnet::Refused>,
    "refusal descriptors cannot be reassigned");

void RefusedChainsNameTheMember() {
  bool getRefused = false;
  try {
    static_cast<void>(agiru::dotnet::Refused::Get(1));
  } catch (const agiru::Error &error) {
    getRefused = std::string_view(error.what()).find("<result>.Get") != std::string_view::npos;
  }
  CHECK_TRUE("a chained absent Get refuses at execution", getRefused);

  bool errorRefused = false;
  try {
    agiru::RaiseOrCollect(agiru::dotnet::Refused::Message);
  } catch (const agiru::Error &error) {
    errorRefused =
        std::string_view(error.what()).find("<result>.Message") != std::string_view::npos;
  }
  CHECK_TRUE("an absent error value preserves its refusal", errorRefused);
}

void ImmutableDescriptorsStillRefuseByName() {
  bool fieldRefused = false;
  try {
    static_cast<void>(agiru::dotnet::Type::GetField("Name"));
  } catch (const agiru::Error &error) {
    fieldRefused = std::string_view(error.what()).find("Type.GetField") != std::string_view::npos;
  }
  CHECK_TRUE("the immutable reflection descriptor remains an explicit refusal", fieldRefused);

  bool genericRefused = false;
  try {
    static_cast<void>(agiru::dotnet::Type::MakeGenericType());
  } catch (const agiru::Error &error) {
    genericRefused =
        std::string_view(error.what()).find("Type.MakeGenericType") != std::string_view::npos;
  }
  CHECK_TRUE("the immutable generic descriptor preserves its refusal identity", genericRefused);
}

template <typename Call> void MemberOperationRefuses(Call call) {
  bool refused = false;
  try {
    call();
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).contains("Fixture.Amount");
  }
  CHECK_TRUE("unavailable scalar operations retain the original member identity", refused);
}

void CompoundAssignmentsAndClearRefuseByName() {
  agiru::dotnet::Refused amount{{.type = "Fixture", .member = "Amount"}};
  const auto value = agiru::Decimal::FromInvariantString("1.23");
  MemberOperationRefuses([&] { amount += value; });
  MemberOperationRefuses([&] { amount -= value; });
  MemberOperationRefuses([&] { amount *= value; });
  MemberOperationRefuses([&] { amount /= value; });
  MemberOperationRefuses([&] { agiru::Clear(amount); });

  auto option = agiru::RefusedOption("Fixture.Status::Absent");
  bool optionRefused = false;
  try {
    agiru::Clear(option);
  } catch (const agiru::Error &error) {
    optionRefused = std::string_view(error.what()).contains("Fixture.Status::Absent");
  }
  CHECK_TRUE("Clear cannot replace an unknown option with a successful default", optionRefused);
  agiru::Integer scalar = 1;
  agiru::Clear(scalar);
  CHECK_TRUE("ordinary scalar Clear remains implemented", scalar == 0);
}

}

int main() {
  return gate::Run("Refused", [] {
    RefusedChainsNameTheMember();
    ImmutableDescriptorsStillRefuseByName();
    CompoundAssignmentsAndClearRefuseByName();
  });
}
