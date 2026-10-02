// Generated from Option.Page.al. Do not edit.

#include "NativeOption.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "platform/AllObjWithCaption.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Query.h"
#include "runtime/Report.h"
#include "runtime/Table.h"
#include "type/Action.h"
#include "type/Enum.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/ObjectType.h"
#include "type/Option.h"

#include "platform/AllObjWithCaption.h"

#include "platform/AllObjWithCaption.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.id == ::agiru::TableId{2000000058} && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.name == "AllObjWithCaption", "native table identity mismatch: AllObjWithCaption");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 9;
}(), "native field count mismatch: AllObjWithCaption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Object Type" || field->caption != "Object Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 23) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "TableData" || field->values[0].caption != "TableData") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Table" || field->values[1].caption != "Table") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "" || field->values[2].caption != "") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Report" || field->values[3].caption != "Report") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "" || field->values[4].caption != "") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "Codeunit" || field->values[5].caption != "Codeunit") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "XMLport" || field->values[6].caption != "XMLport") { return false; }
  if (field->values[7].ordinal != 7 || field->values[7].name != "MenuSuite" || field->values[7].caption != "MenuSuite") { return false; }
  if (field->values[8].ordinal != 8 || field->values[8].name != "Page" || field->values[8].caption != "Page") { return false; }
  if (field->values[9].ordinal != 9 || field->values[9].name != "Query" || field->values[9].caption != "Query") { return false; }
  if (field->values[10].ordinal != 10 || field->values[10].name != "System" || field->values[10].caption != "System") { return false; }
  if (field->values[11].ordinal != 11 || field->values[11].name != "FieldNumber" || field->values[11].caption != "FieldNumber") { return false; }
  if (field->values[12].ordinal != 12 || field->values[12].name != "" || field->values[12].caption != "") { return false; }
  if (field->values[13].ordinal != 13 || field->values[13].name != "" || field->values[13].caption != "") { return false; }
  if (field->values[14].ordinal != 14 || field->values[14].name != "PageExtension" || field->values[14].caption != "PageExtension") { return false; }
  if (field->values[15].ordinal != 15 || field->values[15].name != "TableExtension" || field->values[15].caption != "TableExtension") { return false; }
  if (field->values[16].ordinal != 16 || field->values[16].name != "Enum" || field->values[16].caption != "Enum") { return false; }
  if (field->values[17].ordinal != 17 || field->values[17].name != "EnumExtension" || field->values[17].caption != "EnumExtension") { return false; }
  if (field->values[18].ordinal != 18 || field->values[18].name != "Profile" || field->values[18].caption != "Profile") { return false; }
  if (field->values[19].ordinal != 19 || field->values[19].name != "ProfileExtension" || field->values[19].caption != "ProfileExtension") { return false; }
  if (field->values[20].ordinal != 20 || field->values[20].name != "PermissionSet" || field->values[20].caption != "PermissionSet") { return false; }
  if (field->values[21].ordinal != 21 || field->values[21].name != "PermissionSetExtension" || field->values[21].caption != "PermissionSetExtension") { return false; }
  if (field->values[22].ordinal != 22 || field->values[22].name != "ReportExtension" || field->values[22].caption != "ReportExtension") { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.Object Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Object ID" || field->caption != "Object ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.Object ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Object Name" || field->caption != "Object Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.Object Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{20});
  if (field == nullptr || field->name != "Object Caption" || field->caption != "Object Caption" || field->type != ::agiru::FieldType::Text || field->length != 249) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.Object Caption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{30});
  if (field == nullptr || field->name != "Object Subtype" || field->caption != "Object Subtype" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.Object Subtype");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{60});
  if (field == nullptr || field->name != "App Package ID" || field->caption != "App Package ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.App Package ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{61});
  if (field == nullptr || field->name != "App Runtime Package ID" || field->caption != "App Runtime Package ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.App Runtime Package ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{62});
  if (field == nullptr || field->name != "App ID" || field->caption != "App ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.App ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable, ::agiru::FieldNo{63});
  if (field == nullptr || field->name != "AL Namespace" || field->caption != "AL Namespace" || field->type != ::agiru::FieldType::Text || field->length != 500) { return false; }
  return true;
}(), "native field declaration mismatch: AllObjWithCaption.AL Namespace");
static_assert(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys.size() == 1, "native key count mismatch: AllObjWithCaption");
static_assert(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].name == "pk" && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].fields[1] == ::agiru::FieldNo{3} && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: AllObjWithCaption.pk");
static_assert(::agiru::TableTraits<::agiru::platform::AllObjWithCaption>::kTable.dataPerCompany == false, "native company scope mismatch: AllObjWithCaption");

namespace agiru::Fixture {

constexpr std::array<ControlDef, 1> kNativeOption_C1{{
    ControlDef{.kind = ControlKind::Field, .name = "ObjectType", .source = "Rec.Object Type", .field = ::agiru::FieldNo{1}, .applicationArea = "All"},
}};

constexpr std::array<ControlDef, 1> kNativeOptionLayout{{
    ControlDef{.kind = ControlKind::Area, .area = AreaKind::Content, .children = kNativeOption_C1},
}};

constexpr std::array<ControlDef, 1> kNativeOption_C3{{
    ControlDef{.kind = ControlKind::Action, .name = "Select", .applicationArea = "All"},
}};

constexpr std::array<ControlDef, 1> kNativeOptionActions{{
    ControlDef{.kind = ControlKind::Area, .area = AreaKind::Processing, .children = kNativeOption_C3},
}};

constexpr PageDef kNativeOptionPage{
    .id = NativeOption_Page::kId,
    .name = NativeOption_Page::kName,
    .caption = NativeOption_Page::kName,
    .type = PageType::List,
    .source = ::agiru::TableId{2000000058},
    .layout = kNativeOptionLayout,
    .actions = kNativeOptionActions,
};

static_assert(Depth(kNativeOptionLayout) == 2,
              "a page's layout is a TREE and the generator keeps it -- a flattened one is one level deep (board:0553)");
static_assert(Depth(kNativeOptionActions) == 2,
              "a page's layout is a TREE and the generator keeps it -- a flattened one is one level deep (board:0553)");

} // namespace agiru::Fixture
namespace agiru::Fixture {

namespace {
namespace NativeOption_unit {
const RegisterPage<NativeOption_Page> kInPageCatalogue;
} // namespace NativeOption_unit
} // namespace

} // namespace agiru::Fixture
