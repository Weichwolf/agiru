#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/FeatureKey.h"
#include "runtime/RecordRef.h"
#include "type/FieldClass.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace {

struct FieldSpec {
  int number;
  std::string_view name;
  std::string_view caption;
  agiru::FieldType type;
  int length = 0;
};

constexpr std::array<FieldSpec, 10> kFields{{
    {.number = 1, .name = "ID", .caption = "ID", .type = agiru::FieldType::Text, .length = 50},
    {.number = 2, .name = "Enabled", .caption = "Enabled", .type = agiru::FieldType::Option},
    {.number = 3,
     .name = "Description",
     .caption = "Description",
     .type = agiru::FieldType::Text,
     .length = 2048},
    {.number = 4,
     .name = "Learn More Link",
     .caption = "Learn more",
     .type = agiru::FieldType::Text,
     .length = 2048},
    {.number = 5,
     .name = "Mandatory By",
     .caption = "Approximate mandatory date",
     .type = agiru::FieldType::Text,
     .length = 2048},
    {.number = 6, .name = "Can Try", .caption = "Get started", .type = agiru::FieldType::Boolean},
    {.number = 7, .name = "Is One Way", .caption = "Is One Way", .type = agiru::FieldType::Boolean},
    {.number = 8,
     .name = "Data Update Required",
     .caption = "Data Update Required",
     .type = agiru::FieldType::Boolean},
    {.number = 9,
     .name = "Mandatory By Version",
     .caption = "Approximate mandatory version",
     .type = agiru::FieldType::Text,
     .length = 2048},
    {.number = 10,
     .name = "Description In English",
     .caption = "Description In English",
     .type = agiru::FieldType::Text,
     .length = 2048},
}};

void TheDeclarationMatchesTheSystemSource() {
  const auto &table = agiru::TableTraits<agiru::platform::FeatureKey>::kTable;
  CHECK_TRUE("all ten System fields", table.fields.size() == kFields.size());
  CHECK_TRUE("System table number", table.id.Value() == 2000000211);
  CHECK_TEXT("System table name", table.name, "Feature Key");
  CHECK_TEXT("System table caption", table.caption, "Feature Key");
  CHECK_TRUE("feature keys are tenant-wide", !table.dataPerCompany);
  CHECK_TRUE("one declared primary key", table.keys.size() == 1);
  if (!table.keys.empty()) {
    const auto &key = table.keys.front();
    CHECK_TEXT("declared key name", key.name, "Key1");
    CHECK_TRUE("declared clustered key", key.clustered);
    CHECK_TRUE("ID is the only primary-key member",
               key.fields.size() == 1 && key.fields.front().Value() == 1);
  }
  for (const auto &expected : kFields) {
    const auto *field = agiru::Field(table, agiru::FieldNo{expected.number});
    CHECK_TRUE(expected.name, field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT("declared field name", field->name, expected.name);
    CHECK_TEXT("declared field caption", field->caption, expected.caption);
    CHECK_TRUE("declared type", field->type == expected.type);
    CHECK_TRUE("declared length", field->length == expected.length);
    CHECK_TRUE("no invented FlowField", field->fieldClass == agiru::FieldClass::Normal);
  }
  const auto *enabled = agiru::Field(table, agiru::FieldNo{2});
  CHECK_TRUE("Enabled has both source options", enabled != nullptr && enabled->values.size() == 2);
  if (enabled != nullptr && enabled->values.size() == 2) {
    constexpr std::array<std::string_view, 2> names{"None", "All Users"};
    for (std::size_t i = 0; i < names.size(); ++i) {
      CHECK_TEXT("source option name", enabled->values[i].name, names[i]);
      CHECK_TEXT("source option caption", enabled->values[i].caption, names[i]);
      CHECK_TRUE("source option ordinal", enabled->values[i].ordinal == static_cast<int>(i));
    }
  }
}

void GenericReflectionRetainsTheCompleteTextValues() {
  agiru::platform::FeatureKey record;
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("reflection sees the full source population",
             reflected.FieldCount() == static_cast<int>(kFields.size()));
  for (const auto &expected : kFields) {
    CHECK_TRUE("every declared field is reachable", reflected.FieldExist(expected.number));
    if (expected.type != agiru::FieldType::Text || !reflected.FieldExist(expected.number)) {
      continue;
    }
    const std::string value(static_cast<std::size_t>(expected.length), 'x');
    reflected.Field(expected.number).Value(agiru::Variant(value));
    reflected.SetTable(record);
    agiru::RecordRef read;
    read.GetTable(record);
    CHECK_TEXT("declared text survives typed-record reflection",
               read.Field(expected.number).ToText(),
               value);
  }
}

}

int main() {
  return gate::Run("FeatureKey", [] {
    TheDeclarationMatchesTheSystemSource();
    GenericReflectionRetainsTheCompleteTextValues();
  });
}
