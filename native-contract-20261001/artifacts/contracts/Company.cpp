#include "platform/Company.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::Company>::kTable.id == ::agiru::TableId{2000000006} && ::agiru::TableTraits<::agiru::platform::Company>::kTable.name == "Company", "native table identity mismatch: Company");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::Company>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 5;
}(), "native field count mismatch: Company");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Company>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Name" || field->caption != "Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Company.Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Company>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Evaluation Company" || field->caption != "Evaluation Company" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Company.Evaluation Company");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Company>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Display Name" || field->caption != "Display Name" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Company.Display Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Company>::kTable, ::agiru::FieldNo{8000});
  if (field == nullptr || field->name != "Id" || field->caption != "Id" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Company.Id");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Company>::kTable, ::agiru::FieldNo{8005});
  if (field == nullptr || field->name != "Business Profile Id" || field->caption != "Business Profile Id" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Company.Business Profile Id");
static_assert(::agiru::TableTraits<::agiru::platform::Company>::kTable.keys.size() == 1, "native key count mismatch: Company");
static_assert(::agiru::TableTraits<::agiru::platform::Company>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::Company>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::Company>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::Company>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1}, "native key declaration mismatch: Company.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::Company>::kTable.dataPerCompany == false, "native company scope mismatch: Company");

