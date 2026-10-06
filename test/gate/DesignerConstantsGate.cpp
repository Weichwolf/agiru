#include "dotnet/DesignerFieldProperty.h"
#include "dotnet/DesignerFieldType.h"
#include "dotnet/Generic.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <concepts>
#include <utility>

namespace {

using namespace agiru;
using dotnet::DesignerFieldProperty;
using dotnet::DesignerFieldType;

constexpr std::array<std::pair<Integer, Integer>, 20> kOriginalConstants{{
    {DesignerFieldProperty::BlankZero(), 46},     {DesignerFieldProperty::Caption(), 4},
    {DesignerFieldProperty::DecimalPlaces(), 57}, {DesignerFieldProperty::Description(), 2},
    {DesignerFieldProperty::Editable(), 68},      {DesignerFieldProperty::Enabled(), 67},
    {DesignerFieldProperty::InitValue(), 63},     {DesignerFieldProperty::MultiLine(), 130},
    {DesignerFieldProperty::OptionString(), 75},  {DesignerFieldProperty::ShowCaption(), 129},
    {DesignerFieldType::BigInteger(), 36096},     {DesignerFieldType::Boolean(), 34048},
    {DesignerFieldType::Code(), 31490},           {DesignerFieldType::Decimal(), 12800},
    {DesignerFieldType::Integer(), 34560},        {DesignerFieldType::Option(), 35584},
    {DesignerFieldType::Text(), 31489},           {DesignerFieldType::Date(), 11776},
    {DesignerFieldType::Time(), 11777},           {DesignerFieldType::DateTime(), 37376},
}};

static_assert(std::same_as<decltype(DesignerFieldProperty::Caption()), Integer>);
static_assert(std::same_as<decltype(DesignerFieldType::Integer()), Integer>);
static_assert(DesignerFieldProperty::Description() == 2);
constexpr Integer kOriginalDecimalType = 12800;
static_assert(DesignerFieldType::Decimal() == kOriginalDecimalType);

void CompleteGetterFamiliesKeepTheirOriginalInt32Values() {
  for (const auto &[actual, expected] : kOriginalConstants) {
    CHECK_TRUE("getter preserves the original constant", actual == expected);
    const Variant boxed{actual};
    CHECK_TRUE("getter returns boxed Int32, not property-name Text", boxed.IsInteger());
  }
}

void DesignerPropertiesAreIntegerDictionaryKeys() {
  dotnet::GenericDictionary2 properties;
  properties = agiru::dotnet::GenericDictionary2::Dictionary();
  properties.Add(DesignerFieldProperty::Caption(), "Caption");
  properties.Add(DesignerFieldProperty::Description(), "Description");
  properties.Add(DesignerFieldProperty::Editable(), "true");
  CHECK_TRUE("all designer properties are present", properties.Count() == 3);
  CHECK_TRUE("Caption is reachable by its Int32 ID", properties.ContainsKey(Integer{4}));
  CHECK_TRUE("Caption is not a stringified key", !properties.ContainsKey("4"));
  CHECK_TRUE("Caption ID returns its own text", properties.Item(Integer{4}) == Variant{"Caption"});
  CHECK_TRUE("Description remains a distinct key",
             properties.Item(Integer{2}) == Variant{"Description"});
}

}

int main() {
  return gate::Run("DesignerConstants", [] {
    CompleteGetterFamiliesKeepTheirOriginalInt32Values();
    DesignerPropertiesAreIntegerDictionaryKeys();
  });
}
