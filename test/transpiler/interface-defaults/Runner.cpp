#include "type/Integer.h"

#include "Check.h"
#include "core/codeunit/DefaultConsumer.h"
#include "core/codeunit/InterfaceCaller.h"
#include "core/codeunit/OverrideConsumer.h"
#include "core/interface/DefaultContract.h"
#include "core/interface/ExtendedContract.h"

#include <type_traits>

namespace {

constexpr agiru::Integer kDefaultFee = 13;
constexpr agiru::Integer kOverriddenFee = 21;
constexpr agiru::Integer kScale = 3;
constexpr agiru::Integer kDelta = 4;

static_assert(!std::is_abstract_v<agiru::DefaultConsumer_Codeunit>);
static_assert(!std::is_abstract_v<agiru::OverrideConsumer_Codeunit>);

void DefaultsDispatchThroughTheInterface() {
  agiru::DefaultConsumer_Codeunit unit;
  agiru::DefaultContract_Interface &face = unit;
  agiru::Integer value = 0;
  face.Empty(value);
  CHECK_TRUE("the explicitly empty hook preserves its argument", (value) == (0));
  face.Add(value, kDelta);
  CHECK_TRUE("a default delegates virtually and preserves var", (value) == (1 + kDelta));
  CHECK_TRUE("the source default returns its value", (face.DefaultFee()) == (kDefaultFee));
  CHECK_TRUE("the overload retains its own body",
             (face.DefaultFee(kScale)) == (kDefaultFee * kScale));
  CHECK_TRUE("named returns and local variables execute",
             (face.Named(kDelta)) == (kDefaultFee + kDelta));
  const auto label = face.LabelValue();
  CHECK_TEXT("local labels return owned text", label, "default value");
  CHECK_TRUE("an empty named return is initialized", (face.Zero()) == (0));
  CHECK_TRUE("local option vocabularies are emitted", face.Options());
  unit.Add(value);
  CHECK_TRUE("the declared codeunit API remains public", (value) == (2 + kDelta));
}

void OverridesAndInheritedDefaultsRemainVirtual() {
  agiru::OverrideConsumer_Codeunit unit;
  agiru::ExtendedContract_Interface &child = unit;
  agiru::DefaultContract_Interface &base = unit;
  agiru::Integer value = 0;
  base.Add(value, kDelta);
  CHECK_TRUE("a different implementor controls the same default delegation",
             (value) == (2 + kDelta));
  CHECK_TRUE("an explicit override replaces the default", (base.DefaultFee()) == (kOverriddenFee));
  CHECK_TRUE("an inherited default calls the override",
             (base.DefaultFee(kScale)) == (kOverriddenFee * kScale));
  CHECK_TRUE("extension defaults call inherited overloads",
             (child.ExtendedFee(kScale)) == (kOverriddenFee * kScale + 1));
  CHECK_TRUE("the overriding codeunit keeps its declared API",
             (unit.DefaultFee()) == (kOverriddenFee));
}

}

int main() {
  return gate::Run("Generated Interface Defaults", [] {
    DefaultsDispatchThroughTheInterface();
    OverridesAndInheritedDefaultsRemainVirtual();
    agiru::InterfaceCaller_Codeunit caller;
    CHECK_TRUE("AL interface variables execute the same dispatch", caller.Check());
  });
}
