#include "dotnet/Refused.h"
#include "dotnet/Type.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"

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

}

int main() {
  return gate::Run("Refused", [] {
    RefusedChainsNameTheMember();
    ImmutableDescriptorsStillRefuseByName();
  });
}
