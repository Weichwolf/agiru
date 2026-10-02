#include "platform/PageTableField.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.id == ::agiru::TableId{2000000171} && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.name == "Page Table Field", "native table identity mismatch: Page Table Field");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 15;
}(), "native field count mismatch: Page Table Field");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Page ID" || field->caption != "Page ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Page ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Index" || field->caption != "Index" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Index");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Type" || field->caption != "Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 21) { return false; }
  if (field->values[0].ordinal != 4912 || field->values[0].name != "TableFilter" || field->values[0].caption != "TableFilter") { return false; }
  if (field->values[1].ordinal != 4988 || field->values[1].name != "RecordID" || field->values[1].caption != "RecordID") { return false; }
  if (field->values[2].ordinal != 11519 || field->values[2].name != "OemText" || field->values[2].caption != "OemText") { return false; }
  if (field->values[3].ordinal != 11775 || field->values[3].name != "Date" || field->values[3].caption != "Date") { return false; }
  if (field->values[4].ordinal != 11776 || field->values[4].name != "Time" || field->values[4].caption != "Time") { return false; }
  if (field->values[5].ordinal != 11797 || field->values[5].name != "DateFormula" || field->values[5].caption != "DateFormula") { return false; }
  if (field->values[6].ordinal != 12799 || field->values[6].name != "Decimal" || field->values[6].caption != "Decimal") { return false; }
  if (field->values[7].ordinal != 26207 || field->values[7].name != "Media" || field->values[7].caption != "Media") { return false; }
  if (field->values[8].ordinal != 26208 || field->values[8].name != "MediaSet" || field->values[8].caption != "MediaSet") { return false; }
  if (field->values[9].ordinal != 31488 || field->values[9].name != "Text" || field->values[9].caption != "Text") { return false; }
  if (field->values[10].ordinal != 31489 || field->values[10].name != "Code" || field->values[10].caption != "Code") { return false; }
  if (field->values[11].ordinal != 33791 || field->values[11].name != "NotSupported_Binary" || field->values[11].caption != "NotSupported_Binary") { return false; }
  if (field->values[12].ordinal != 33793 || field->values[12].name != "BLOB" || field->values[12].caption != "BLOB") { return false; }
  if (field->values[13].ordinal != 34047 || field->values[13].name != "Boolean" || field->values[13].caption != "Boolean") { return false; }
  if (field->values[14].ordinal != 34559 || field->values[14].name != "Integer" || field->values[14].caption != "Integer") { return false; }
  if (field->values[15].ordinal != 35071 || field->values[15].name != "OemCode" || field->values[15].caption != "OemCode") { return false; }
  if (field->values[16].ordinal != 35583 || field->values[16].name != "Option" || field->values[16].caption != "Option") { return false; }
  if (field->values[17].ordinal != 36095 || field->values[17].name != "BigInteger" || field->values[17].caption != "BigInteger") { return false; }
  if (field->values[18].ordinal != 36863 || field->values[18].name != "Duration" || field->values[18].caption != "Duration") { return false; }
  if (field->values[19].ordinal != 37119 || field->values[19].name != "GUID" || field->values[19].caption != "GUID") { return false; }
  if (field->values[20].ordinal != 37375 || field->values[20].name != "DateTime" || field->values[20].caption != "DateTime") { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Length" || field->caption != "Length" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Length");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Caption" || field->caption != "Caption" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Caption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Status" || field->caption != "Status" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 3) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "New" || field->values[0].caption != "New") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Ready" || field->values[1].caption != "Ready") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Placed" || field->values[2].caption != "Placed") { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Status");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "IsTableField" || field->caption != "IsTableField" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.IsTableField");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Scope" || field->caption != "Scope" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 10) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "TableFieldVisibleOnPage" || field->values[0].caption != "TableFieldVisibleOnPage") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "TableFieldHiddenOnPage" || field->values[1].caption != "TableFieldHiddenOnPage") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "TableFieldNotOnPage" || field->values[2].caption != "TableFieldNotOnPage") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "TableExtensionFieldVisibleOnPage" || field->values[3].caption != "TableExtensionFieldVisibleOnPage") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "TableExtensionFieldHiddenOnPage" || field->values[4].caption != "TableExtensionFieldHiddenOnPage") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "TableExtensionFieldNotOnPage" || field->values[5].caption != "TableExtensionFieldNotOnPage") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "PageFieldVisible" || field->values[6].caption != "PageFieldVisible") { return false; }
  if (field->values[7].ordinal != 7 || field->values[7].name != "PageFieldHidden" || field->values[7].caption != "PageFieldHidden") { return false; }
  if (field->values[8].ordinal != 8 || field->values[8].name != "PageExtensionFieldVisible" || field->values[8].caption != "PageExtensionFieldVisible") { return false; }
  if (field->values[9].ordinal != 9 || field->values[9].name != "PageExtensionFieldHidden" || field->values[9].caption != "PageExtensionFieldHidden") { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Scope");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Tooltip" || field->caption != "Tooltip" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Tooltip");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "FieldKind" || field->caption != "FieldKind" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 3) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "TableField" || field->values[0].caption != "TableField") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "PageFieldBoundToTable" || field->values[1].caption != "PageFieldBoundToTable") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "PageFieldBoundToExpression" || field->values[2].caption != "PageFieldBoundToExpression") { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.FieldKind");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "Name" || field->caption != "Name" || field->type != ::agiru::FieldType::Text || field->length != 256) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "Field ID" || field->caption != "Field ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Field ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "Table No" || field->caption != "Table No" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Table No");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable, ::agiru::FieldNo{15});
  if (field == nullptr || field->name != "Table Field Id" || field->caption != "Table Field Id" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Table Field.Table Field Id");
static_assert(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys.size() == 1, "native key count mismatch: Page Table Field");
static_assert(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].name == "pk" && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Page Table Field.pk");
static_assert(::agiru::TableTraits<::agiru::platform::PageTableField>::kTable.dataPerCompany == false, "native company scope mismatch: Page Table Field");

