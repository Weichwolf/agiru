#include "platform/Integer.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::Integer>::kTable.id == ::agiru::TableId{2000000026} && ::agiru::TableTraits<::agiru::platform::Integer>::kTable.name == "Integer", "native table identity mismatch: Integer");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::Integer>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 1;
}(), "native field count mismatch: Integer");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Integer>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Number" || field->caption != "Number" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Integer.Number");
static_assert(::agiru::TableTraits<::agiru::platform::Integer>::kTable.keys.size() == 1, "native key count mismatch: Integer");
static_assert(::agiru::TableTraits<::agiru::platform::Integer>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::Integer>::kTable.keys[0].name == "pk" && ::agiru::TableTraits<::agiru::platform::Integer>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::Integer>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1}, "native key declaration mismatch: Integer.pk");
static_assert(::agiru::TableTraits<::agiru::platform::Integer>::kTable.dataPerCompany == false, "native company scope mismatch: Integer");

