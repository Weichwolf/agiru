#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
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
  agiru::FieldType type;
  int length = 0;
};

// System.app 29.0.55365.0: Virtual Tables/AllObj{,WithCaption}.Table.al.
constexpr int kBareTableId = 2000000038;
constexpr int kCaptionedTableId = 2000000058;
constexpr int kBareNamespaceField = 62;
constexpr int kCaptionedNamespaceField = 63;
constexpr int kApplicationField = 62;

constexpr std::array<FieldSpec, 7> kBareFields{{
    {.number = 1, .name = "Object Type", .type = agiru::FieldType::Option},
    {.number = 3, .name = "Object ID", .type = agiru::FieldType::Integer},
    {.number = 4, .name = "Object Name", .type = agiru::FieldType::Text, .length = 30},
    {.number = 5, .name = "Name", .type = agiru::FieldType::Text, .length = 100},
    {.number = 60, .name = "App Package ID", .type = agiru::FieldType::Guid},
    {.number = 61, .name = "App Runtime Package ID", .type = agiru::FieldType::Guid},
    {.number = 62, .name = "AL Namespace", .type = agiru::FieldType::Text, .length = 500},
}};
constexpr std::array<FieldSpec, 10> kCaptionedFields{{
    {.number = 1, .name = "Object Type", .type = agiru::FieldType::Option},
    {.number = 3, .name = "Object ID", .type = agiru::FieldType::Integer},
    {.number = 4, .name = "Object Name", .type = agiru::FieldType::Text, .length = 30},
    {.number = 5, .name = "Name", .type = agiru::FieldType::Text, .length = 100},
    {.number = 20, .name = "Object Caption", .type = agiru::FieldType::Text, .length = 249},
    {.number = 30, .name = "Object Subtype", .type = agiru::FieldType::Text, .length = 30},
    {.number = 60, .name = "App Package ID", .type = agiru::FieldType::Guid},
    {.number = 61, .name = "App Runtime Package ID", .type = agiru::FieldType::Guid},
    {.number = 62, .name = "App ID", .type = agiru::FieldType::Guid},
    {.number = 63, .name = "AL Namespace", .type = agiru::FieldType::Text, .length = 500},
}};
constexpr std::array<std::string_view, 29> kObjectTypes{"TableData",
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
                                                        "ReportExtension",
                                                        "",
                                                        "",
                                                        "",
                                                        "",
                                                        "",
                                                        "Interface"};
constexpr std::array<int, 2> kPrimaryKey{1, 3};
constexpr std::string_view kPackage = "11111111-1111-1111-1111-111111111111";
constexpr std::string_view kRuntimePackage = "22222222-2222-2222-2222-222222222222";
constexpr std::string_view kApplication = "33333333-3333-3333-3333-333333333333";

template <typename Table, std::size_t Count>
void DeclarationMatchesSource(int id,
                              std::string_view name,
                              const std::array<FieldSpec, Count> &fields) {
  const auto &table = agiru::TableTraits<Table>::kTable;
  CHECK_TRUE("System table ID", table.id.Value() == id);
  CHECK_TEXT("original AL table name", table.name, name);
  CHECK_TRUE("complete source and implicit field population",
             table.fields.size() ==
                 fields.size() + agiru::ImplicitFieldCount(agiru::SystemFieldProfile::Runtime18,
                                                           agiru::TableType::Normal,
                                                           false));
  CHECK_TRUE("shared company scope", !table.dataPerCompany);
  CHECK_TEXT("source inherent permissions", table.inherentPermissions, "rX");
  CHECK_TRUE("only the source key", table.keys.size() == 1);
  const auto *key = table.keys.empty() ? nullptr : &table.keys.front();
  CHECK_TEXT("source primary-key name", key == nullptr ? "<missing>" : key->name, "pk");
  CHECK_TRUE("primary key is clustered by default", key != nullptr && key->clustered);
  CHECK_TRUE("complete key field population",
             key != nullptr && key->fields.size() == kPrimaryKey.size());
  for (std::size_t index = 0; index < kPrimaryKey.size(); ++index) {
    CHECK_TRUE("source key order",
               key != nullptr && index < key->fields.size() &&
                   key->fields[index].Value() == kPrimaryKey[index]);
  }
  for (const auto &expected : fields) {
    const auto *field = agiru::Field(table, agiru::FieldNo{expected.number});
    CHECK_TRUE(expected.name, field != nullptr);
    CHECK_TEXT("source field name", field == nullptr ? "<missing>" : field->name, expected.name);
    CHECK_TEXT(
        "source default caption", field == nullptr ? "<missing>" : field->caption, expected.name);
    CHECK_TRUE("source field type", field != nullptr && field->type == expected.type);
    CHECK_TRUE("source field length", field != nullptr && field->length == expected.length);
    CHECK_TRUE("declared normal field",
               field != nullptr && field->fieldClass == agiru::FieldClass::Normal);
  }
  const auto *type = agiru::Field(table, agiru::FieldNo{1});
  CHECK_TRUE("all source option positions",
             type != nullptr && type->values.size() == kObjectTypes.size());
  for (std::size_t index = 0; index < kObjectTypes.size(); ++index) {
    const auto *value =
        type != nullptr && index < type->values.size() ? &type->values[index] : nullptr;
    CHECK_TRUE("source option ordinal",
               value != nullptr && value->ordinal == static_cast<int>(index));
    CHECK_TEXT("source option names include empty reserved positions",
               value == nullptr ? "<missing>" : value->name,
               kObjectTypes[index]);
    CHECK_TEXT("source option captions include empty reserved positions",
               value == nullptr ? "<missing>" : value->caption,
               kObjectTypes[index]);
  }
}

template <typename Table, std::size_t Count>
void ReflectionPreservesText(const std::array<FieldSpec, Count> &fields) {
  Table record;
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("reflection retains source field population",
             reflected.FieldCount() == static_cast<int>(Count));
  for (const auto &field : fields) {
    const bool exists = reflected.FieldExist(field.number);
    CHECK_TRUE("reflection retains original field numbers", exists);
    if (field.type != agiru::FieldType::Text) { continue; }
    const std::string full(static_cast<std::size_t>(field.length), 'x');
    if (exists &&
        agiru::Field(agiru::TableTraits<Table>::kTable, agiru::FieldNo{field.number})->type ==
            agiru::FieldType::Text) {
      reflected.Field(field.number).Value(agiru::Variant(full));
    }
    CHECK_TEXT("full declared text roundtrips",
               exists ? reflected.Field(field.number).ToText() : "<missing>",
               full);
  }
}

template <typename Table> void IdentitiesRemainIndependent(int namespaceField, bool captioned) {
  Table record;
  record.ObjectName = "Acc. Schedule Name";
  record.AppPackageID = agiru::Guid(kPackage);
  record.AppRuntimePackageID = agiru::Guid(kRuntimePackage);
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  const std::string full(500, 'x');
  if (reflected.FieldExist(namespaceField)) {
    reflected.Field(namespaceField).Value(agiru::Variant(full));
  }
  if (captioned && reflected.FieldExist(kApplicationField)) {
    reflected.Field(kApplicationField).Value(agiru::Variant(agiru::Guid(kApplication)));
  }
  reflected.SetTable(record);
  CHECK_TEXT(
      "AL name is not replaced by the longer caption", record.ObjectName, "Acc. Schedule Name");
  CHECK_TEXT("namespace retains original 500-character length",
             reflected.FieldExist(namespaceField)
                 ? record.FieldFormat(agiru::FieldNo{namespaceField})
                 : "<missing>",
             full);
  CHECK_TEXT("package identity is independent",
             record.AppPackageID.ToText(),
             "{" + std::string(kPackage) + "}");
  CHECK_TEXT("runtime package identity is independent",
             record.AppRuntimePackageID.ToText(),
             "{" + std::string(kRuntimePackage) + "}");
  if (captioned) {
    CHECK_TEXT("application identity is not either package identity",
               reflected.FieldExist(kApplicationField)
                   ? record.FieldFormat(agiru::FieldNo{kApplicationField})
                   : "<missing>",
               "{" + std::string(kApplication) + "}");
  }
}

void UndeclaredFieldsRemainAbsent() {
  const auto &bare = agiru::TableTraits<agiru::platform::AllObj>::kTable;
  const auto &captioned = agiru::TableTraits<agiru::platform::AllObjWithCaption>::kTable;
  for (const int number : {7, 20, 21, 63}) {
    CHECK_TRUE("caption-only and invented AllObj fields stay absent",
               agiru::Field(bare, agiru::FieldNo{number}) == nullptr);
  }
  for (const int number : {7, 21, 99}) {
    CHECK_TRUE("invented AllObjWithCaption fields stay absent",
               agiru::Field(captioned, agiru::FieldNo{number}) == nullptr);
  }
  agiru::platform::AllObjWithCaption record;
  record.ObjectName = "Acc. Schedule Name";
  record.ObjectCaption = "Financial Report Row Definition";
  CHECK_TEXT("caption does not replace the AL name", record.ObjectName, "Acc. Schedule Name");
  CHECK_TEXT("the source caption number reads text, not the old package GUID",
             record.FieldFormat(agiru::FieldNo{20}),
             "Financial Report Row Definition");
}

}

int main() {
  return gate::Run("ObjectCatalogue", [] {
    DeclarationMatchesSource<agiru::platform::AllObj>(kBareTableId, "AllObj", kBareFields);
    DeclarationMatchesSource<agiru::platform::AllObjWithCaption>(
        kCaptionedTableId, "AllObjWithCaption", kCaptionedFields);
    ReflectionPreservesText<agiru::platform::AllObj>(kBareFields);
    ReflectionPreservesText<agiru::platform::AllObjWithCaption>(kCaptionedFields);
    IdentitiesRemainIndependent<agiru::platform::AllObj>(kBareNamespaceField, false);
    IdentitiesRemainIndependent<agiru::platform::AllObjWithCaption>(kCaptionedNamespaceField, true);
    UndeclaredFieldsRemainAbsent();
  });
}
