#include "platform/RecordLink.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.id == ::agiru::TableId{2000000068} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.name == "Record Link", "native table identity mismatch: Record Link");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 14;
}(), "native field count mismatch: Record Link");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Link ID" || field->caption != "Link ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Link ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Record ID" || field->caption != "Record ID" || field->type != ::agiru::FieldType::RecordId || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Record ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "URL1" || field->caption != "URL1" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL1");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "URL2" || field->caption != "URL2" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL2");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "URL3" || field->caption != "URL3" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL3");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "URL4" || field->caption != "URL4" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL4");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Type" || field->caption != "Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Link" || field->values[0].caption != "Link") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Note" || field->values[1].caption != "Note") { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Note" || field->caption != "Note" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Note");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "Created" || field->caption != "Created" || field->type != ::agiru::FieldType::DateTime || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Created");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "User ID" || field->caption != "User ID" || field->type != ::agiru::FieldType::Text || field->length != 132) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.User ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "Company" || field->caption != "Company" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Company");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "Notify" || field->caption != "Notify" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Notify");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "To User ID" || field->caption != "To User ID" || field->type != ::agiru::FieldType::Text || field->length != 132) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.To User ID");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() == 3, "native key count mismatch: Record Link");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1}, "native key declaration mismatch: Record Link.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() > 1 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].name == "Key2" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].fields[0] == ::agiru::FieldNo{2}, "native key declaration mismatch: Record Link.Key2");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() > 2 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].name == "Key3" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].fields[0] == ::agiru::FieldNo{12} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].fields[1] == ::agiru::FieldNo{2}, "native key declaration mismatch: Record Link.Key3");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.dataPerCompany == false, "native company scope mismatch: Record Link");

