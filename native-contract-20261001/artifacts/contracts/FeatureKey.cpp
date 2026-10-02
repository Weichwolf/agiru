#include "platform/FeatureKey.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.id == ::agiru::TableId{2000000211} && ::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.name == "Feature Key", "native table identity mismatch: Feature Key");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 10;
}(), "native field count mismatch: Feature Key");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "ID" || field->caption != "ID" || field->type != ::agiru::FieldType::Text || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Enabled" || field->caption != "Enabled" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "None" || field->values[0].caption != "None") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "All Users" || field->values[1].caption != "All Users") { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Enabled");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Learn More Link" || field->caption != "Learn more" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Learn More Link");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Mandatory By" || field->caption != "Approximate mandatory date" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Mandatory By");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Can Try" || field->caption != "Get started" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Can Try");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Is One Way" || field->caption != "Is One Way" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Is One Way");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Data Update Required" || field->caption != "Data Update Required" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Data Update Required");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Mandatory By Version" || field->caption != "Approximate mandatory version" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Mandatory By Version");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "Description In English" || field->caption != "Description In English" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Feature Key.Description In English");
static_assert(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.keys.size() == 1, "native key count mismatch: Feature Key");
static_assert(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1}, "native key declaration mismatch: Feature Key.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::FeatureKey>::kTable.dataPerCompany == false, "native company scope mismatch: Feature Key");

