#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/AllObjWithCaption.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/Integer.h"
#include "platform/User.h"
#include "runtime/RecordRef.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Variant.h"

#include "Check.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace {

struct FieldSpec {
  int number;
  std::string_view name;
  agiru::FieldType type;
  int length = 0;
};

constexpr std::array<FieldSpec, 9> kFields{{
    {.number = 1, .name = "Object Type", .type = agiru::FieldType::Option},
    {.number = 3, .name = "Object ID", .type = agiru::FieldType::Integer},
    {.number = 4, .name = "Object Name", .type = agiru::FieldType::Text, .length = 30},
    {.number = 20, .name = "Object Caption", .type = agiru::FieldType::Text, .length = 249},
    {.number = 30, .name = "Object Subtype", .type = agiru::FieldType::Text, .length = 30},
    {.number = 60, .name = "App Package ID", .type = agiru::FieldType::Guid},
    {.number = 61, .name = "App Runtime Package ID", .type = agiru::FieldType::Guid},
    {.number = 62, .name = "App ID", .type = agiru::FieldType::Guid},
    {.number = 63, .name = "AL Namespace", .type = agiru::FieldType::Text, .length = 500},
}};
constexpr std::array<std::string_view, 23> kObjectTypes{"TableData",
                                                        "Table",
                                                        "",
                                                        "Report",
                                                        "",
                                                        "Codeunit",
                                                        "XMLport",
                                                        "MenuSuite",
                                                        "Page",
                                                        "Query",
                                                        "System",
                                                        "FieldNumber",
                                                        "",
                                                        "",
                                                        "PageExtension",
                                                        "TableExtension",
                                                        "Enum",
                                                        "EnumExtension",
                                                        "Profile",
                                                        "ProfileExtension",
                                                        "PermissionSet",
                                                        "PermissionSetExtension",
                                                        "ReportExtension"};
constexpr std::array<int, 2> kPrimaryKey{1, 3};
constexpr std::array<int, 4> kUndeclaredFields{5, 7, 21, 99};
constexpr int kNativeTableId = 2000000058;
constexpr int kCaptionField = 20;
constexpr int kApplicationField = 62;
constexpr int kNamespaceField = 63;
constexpr std::string_view kPackage = "11111111-1111-1111-1111-111111111111";
constexpr std::string_view kRuntimePackage = "22222222-2222-2222-2222-222222222222";
constexpr std::string_view kApplication = "33333333-3333-3333-3333-333333333333";

void DeclarationMatchesSystemSource() {
  const auto &table = agiru::TableTraits<agiru::platform::AllObjWithCaption>::kTable;
  CHECK_TRUE("all nine source fields remain visible", table.fields.size() == kFields.size());
  CHECK_TRUE("the native ID is not an application table ID", table.id.Value() == kNativeTableId);
  CHECK_TEXT("the original AL name", table.name, "AllObjWithCaption");
  CHECK_TRUE("tenant-wide metadata", !table.dataPerCompany);
  CHECK_TEXT("the original inherent permissions", table.inherentPermissions, "rX");
  CHECK_TRUE("only the source primary key", table.keys.size() == 1);
  if (!table.keys.empty()) {
    CHECK_TEXT("the original primary-key name", table.keys.front().name, "pk");
    CHECK_TRUE("the primary key is clustered by default", table.keys.front().clustered);
    CHECK_TRUE("both primary-key fields", table.keys.front().fields.size() == kPrimaryKey.size());
    const auto count = std::min(table.keys.front().fields.size(), kPrimaryKey.size());
    for (std::size_t i = 0; i < count; ++i) {
      CHECK_TRUE("source key order", table.keys.front().fields[i].Value() == kPrimaryKey[i]);
    }
  }
  for (const auto &expected : kFields) {
    const auto *field = agiru::Field(table, agiru::FieldNo{expected.number});
    CHECK_TRUE(expected.name, field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT("source field name", field->name, expected.name);
    CHECK_TEXT("default caption is the original name", field->caption, expected.name);
    CHECK_TRUE("source field type", field->type == expected.type);
    CHECK_TRUE("source field length", field->length == expected.length);
    CHECK_TRUE("metadata fields are not invented FlowFields",
               field->fieldClass == agiru::FieldClass::Normal);
  }
  for (const int number : kUndeclaredFields) {
    CHECK_TRUE("invented field numbers are absent",
               agiru::Field(table, agiru::FieldNo{number}) == nullptr);
  }
  const auto *type = agiru::Field(table, agiru::FieldNo{1});
  CHECK_TRUE("every source option position is retained",
             type != nullptr && type->values.size() == kObjectTypes.size());
  if (type == nullptr) { return; }
  for (std::size_t i = 0; i < type->values.size() && i < kObjectTypes.size(); ++i) {
    CHECK_TRUE("source option ordinal", type->values[i].ordinal == static_cast<int>(i));
    CHECK_TEXT("source option name, including holes", type->values[i].name, kObjectTypes[i]);
    CHECK_TEXT("source option caption, including holes", type->values[i].caption, kObjectTypes[i]);
  }
}

void ReflectionUsesDeclaredNumbersAndTypes() {
  agiru::platform::AllObjWithCaption record;
  record.ObjectCaption = "Original object caption";
  record.ObjectSubtype = "Original subtype";
  record.AppPackageID = agiru::Guid(kPackage);
  record.AppRuntimePackageID = agiru::Guid(kRuntimePackage);
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("reflection retains the complete declaration",
             reflected.FieldCount() == static_cast<int>(kFields.size()));
  for (const auto &field : kFields) {
    CHECK_TRUE("each declared field has a typed reflection slot",
               reflected.FieldExist(field.number));
    if (!reflected.FieldExist(field.number)) { continue; }
    if (field.type == agiru::FieldType::Text) {
      const std::string value(static_cast<std::size_t>(field.length), 'x');
      reflected.Field(field.number).Value(agiru::Variant(value));
      CHECK_TEXT("full declared text roundtrips", reflected.Field(field.number).ToText(), value);
    }
  }
  if (reflected.FieldExist(kApplicationField)) {
    reflected.Field(kApplicationField).Value(agiru::Variant(agiru::Guid(kApplication)));
  }
  reflected.SetTable(record);
  CHECK_TEXT("App ID does not alias the package identity",
             record.AppID.ToText(),
             "{" + std::string(kApplication) + "}");
  CHECK_TEXT("package identity survives other metadata writes",
             record.AppPackageID.ToText(),
             "{" + std::string(kPackage) + "}");
  CHECK_TEXT("runtime package is independent",
             record.AppRuntimePackageID.ToText(),
             "{" + std::string(kRuntimePackage) + "}");
  record.ObjectCaption = "Original object caption";
  record.ALNamespace = "Original.AL.Namespace";
  CHECK_TEXT("the same primitive as a page control reads the caption",
             record.FieldFormat(agiru::FieldNo{kCaptionField}),
             "Original object caption");
  CHECK_TEXT("the same primitive reads the original namespace",
             record.FieldFormat(agiru::FieldNo{kNamespaceField}),
             "Original.AL.Namespace");
}

void NativeScopeMatchesSystemSource() {
  CHECK_TRUE("Company is shared across companies",
             !agiru::TableTraits<agiru::platform::Company>::kTable.dataPerCompany);
  CHECK_TRUE("Date periods are shared across companies",
             !agiru::TableTraits<agiru::platform::Date>::kTable.dataPerCompany);
  CHECK_TRUE("Integer rows are shared across companies",
             !agiru::TableTraits<agiru::platform::Integer>::kTable.dataPerCompany);
  CHECK_TRUE("User identities are shared across companies",
             !agiru::TableTraits<agiru::platform::User>::kTable.dataPerCompany);
  CHECK_TEXT("Integer retains the source key spelling",
             agiru::TableTraits<agiru::platform::Integer>::kTable.keys.front().name,
             "pk");
}

void AuthenticationEmailIsText() {
  const auto &table = agiru::TableTraits<agiru::platform::User>::kTable;
  const auto *field = agiru::Field(table, agiru::platform::User::Field_No::AuthenticationEmail);
  CHECK_TRUE("Authentication Email remains declared", field != nullptr);
  if (field == nullptr) { return; }
  CHECK_TRUE("Authentication Email has the source Text type",
             field->type == agiru::FieldType::Text);
  CHECK_TRUE("Authentication Email retains all 250 characters", field->length == 250);
  agiru::platform::User user;
  constexpr std::string_view value = " Mixed.Case@Example.com ";
  user.AuthenticationEmail = value;
  CHECK_TEXT("direct assignment preserves Text case and spaces", user.AuthenticationEmail, value);
  CHECK_TEXT("the page-read primitive does not normalize authentication text",
             user.FieldFormat(agiru::platform::User::Field_No::AuthenticationEmail),
             value);
  agiru::RecordRef reflected;
  reflected.GetTable(user);
  constexpr std::string_view changed = " Other.Mixed@Example.com ";
  reflected.Field(agiru::platform::User::Field_No::AuthenticationEmail.Value())
      .Value(agiru::Variant(changed));
  reflected.SetTable(user);
  CHECK_TEXT("reflection preserves Text case and spaces", user.AuthenticationEmail, changed);
  const std::string full(250, 'x');
  user.AuthenticationEmail = full;
  CHECK_TEXT("the full declared text roundtrips", user.AuthenticationEmail, full);
}

}

int main() {
  return gate::Run("NativeObject", [] {
    DeclarationMatchesSystemSource();
    ReflectionUsesDeclaredNumbersAndTypes();
    NativeScopeMatchesSystemSource();
    AuthenticationEmailIsText();
  });
}
