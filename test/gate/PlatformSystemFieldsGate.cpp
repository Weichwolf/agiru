#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"
#include "platform/AllProfile.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/FeatureKey.h"
#include "platform/Field.h"
#include "platform/Integer.h"
#include "platform/ODataEdmType.h"
#include "platform/ObjectOptions.h"
#include "platform/PageMetadata.h"
#include "platform/PageTableField.h"
#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"
#include "platform/RecordLink.h"
#include "platform/TableMetadata.h"
#include "platform/User.h"
#include "platform/UserPersonalization.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/FieldClass.h"
#include "type/Guid.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace {

struct SystemSpec {
  int number;
  std::string_view name;
  agiru::FieldType type;
  std::string_view reflectionName;
};

// devenv-table-system-fields.md: the base SystemId/audit contract, independent of Declare.h.
constexpr std::array<SystemSpec, 5> kSystemFields{{
    {.number = 2000000000,
     .name = "SystemId",
     .type = agiru::FieldType::Guid,
     .reflectionName = "$systemId"},
    {.number = 2000000001,
     .name = "SystemCreatedAt",
     .type = agiru::FieldType::DateTime,
     .reflectionName = "SystemCreatedAt"},
    {.number = 2000000002,
     .name = "SystemCreatedBy",
     .type = agiru::FieldType::Guid,
     .reflectionName = "SystemCreatedBy"},
    {.number = 2000000003,
     .name = "SystemModifiedAt",
     .type = agiru::FieldType::DateTime,
     .reflectionName = "SystemModifiedAt"},
    {.number = 2000000004,
     .name = "SystemModifiedBy",
     .type = agiru::FieldType::Guid,
     .reflectionName = "SystemModifiedBy"},
}};
constexpr auto kCreatedMilliseconds = 123456789;
constexpr auto kModifiedMilliseconds = 234567890;
constexpr std::size_t kFirstAuditLookup = kSystemFields.size() + 1;

template <typename Table> constexpr std::array<int, kSystemFields.size()> Numbers() {
  if constexpr (requires {
                  Table::Field_No::SystemId;
                  Table::Field_No::SystemCreatedAt;
                  Table::Field_No::SystemCreatedBy;
                  Table::Field_No::SystemModifiedAt;
                  Table::Field_No::SystemModifiedBy;
                }) {
    return {Table::Field_No::SystemId.Value(),
            Table::Field_No::SystemCreatedAt.Value(),
            Table::Field_No::SystemCreatedBy.Value(),
            Table::Field_No::SystemModifiedAt.Value(),
            Table::Field_No::SystemModifiedBy.Value()};
  }
  return {};
}

template <typename Table> void CheckSystemFields() {
  const auto &metadata = agiru::TableTraits<Table>::kTable;
  const auto numbers = Numbers<Table>();
  const std::array offsets{offsetof(Table, SystemId),
                           offsetof(Table, SystemCreatedAt),
                           offsetof(Table, SystemCreatedBy),
                           offsetof(Table, SystemModifiedAt),
                           offsetof(Table, SystemModifiedBy)};
  Table row;
  row.SystemId = agiru::Guid("11111111-1111-1111-1111-111111111111");
  row.SystemCreatedBy = agiru::Guid("22222222-2222-2222-2222-222222222222");
  row.SystemModifiedBy = agiru::Guid("33333333-3333-3333-3333-333333333333");
  row.SystemCreatedAt = agiru::DateTime::FromMilliseconds(kCreatedMilliseconds);
  row.SystemModifiedAt = agiru::DateTime::FromMilliseconds(kModifiedMilliseconds);
  const std::array values{row.SystemId.ToText(),
                          row.SystemCreatedAt.ToInvariantString(),
                          row.SystemCreatedBy.ToText(),
                          row.SystemModifiedAt.ToInvariantString(),
                          row.SystemModifiedBy.ToText()};
  agiru::RecordRef reflected;
  reflected.GetTable(row);
  std::size_t declared = 0;
  for (const auto &field : metadata.fields) {
    if (!agiru::IsImplicitSystemField(field.no)) { ++declared; }
  }
  CHECK_TRUE("system fields never inflate indexed AL field count",
             reflected.FieldCount() == static_cast<int>(declared));
  for (std::size_t index = 0; index < kSystemFields.size(); ++index) {
    const auto &expected = kSystemFields[index];
    const std::string identity = std::string(metadata.name) + "." + std::string(expected.name);
    const auto *field = agiru::Field(metadata, agiru::FieldNo{expected.number});
    CHECK_TRUE(identity + " exposes the reserved Field_No", numbers[index] == expected.number);
    CHECK_TRUE(identity + " has immutable metadata", field != nullptr);
    CHECK_TEXT(
        identity + " preserves the AL name", field == nullptr ? "" : field->name, expected.name);
    CHECK_TEXT(
        identity + " preserves the caption", field == nullptr ? "" : field->caption, expected.name);
    CHECK_TRUE(identity + " has the documented type",
               field != nullptr && field->type == expected.type);
    CHECK_TRUE(identity + " has the actual member offset",
               field != nullptr && field->offset == offsets[index]);
    CHECK_TRUE(identity + " is a normal zero-length field",
               field != nullptr && field->length == 0 &&
                   field->fieldClass == agiru::FieldClass::Normal);
    CHECK_TRUE(identity + " is directly reachable", reflected.FieldExist(expected.number));
    CHECK_TEXT(identity + " preserves its original reflection spelling",
               reflected.Field(expected.number).Name(),
               expected.reflectionName);
    CHECK_TEXT(identity + " reflects the typed member",
               field == nullptr ? "<missing>" : reflected.Field(expected.number).ToText(),
               values[index]);
  }
}

void CheckSelectedProfile(agiru::SystemFieldProfile profile,
                          agiru::TableType type,
                          bool linked,
                          bool audit) {
  constexpr std::array<int, 10> numbers{0,
                                        2000000000,
                                        2000000001,
                                        2000000002,
                                        2000000003,
                                        2000000004,
                                        2000000005,
                                        2000000006,
                                        2000000007,
                                        2000000008};
  std::size_t present = 0;
  for (std::size_t i = 0; i < numbers.size(); ++i) {
    const auto &field = agiru::kImplicitSystemFields[i];
    CHECK_TRUE("canonical implicit identities remain in original number order",
               field.no.Value() == numbers[i]);
    const bool expected =
        i < 2 ||
        (audit && (i < kFirstAuditLookup || profile == agiru::SystemFieldProfile::Runtime18));
    const bool selected = agiru::IncludesSystemField(field, profile, type, linked);
    CHECK_TRUE("the complete selected identity population agrees", selected == expected);
    present += selected ? 1 : 0;
  }
  const std::size_t expected = !audit                                            ? 2
                               : profile == agiru::SystemFieldProfile::Runtime18 ? 10
                                                                                 : 6;
  CHECK_TRUE("the complete denominator includes timestamp and identity", present == expected);
}

void CompleteProfilesRespectVersionKindAndLinkedObject() {
  using agiru::SystemFieldProfile;
  using agiru::TableType;

  struct KindCase {
    TableType type;
    bool audit;
  };

  constexpr std::array kinds{KindCase{.type = TableType::Normal, .audit = true},
                             KindCase{.type = TableType::CRM, .audit = false},
                             KindCase{.type = TableType::CDS, .audit = false},
                             KindCase{.type = TableType::ExternalSQL, .audit = false},
                             KindCase{.type = TableType::Exchange, .audit = false},
                             KindCase{.type = TableType::MicrosoftGraph, .audit = false},
                             KindCase{.type = TableType::Temporary, .audit = true}};
  CHECK_TRUE("the canonical profile contains every original implicit identity",
             agiru::kImplicitSystemFields.size() == 10);
  for (const auto kind : kinds) {
    for (const bool linked : {false, true}) {
      const bool audit = kind.audit && !linked;
      CHECK_TRUE("audit applicability follows kind and LinkedObject, not numeric ordinals",
                 agiru::CarriesAuditFields(kind.type, linked) == audit);
      for (const auto profile : {SystemFieldProfile::Runtime17, SystemFieldProfile::Runtime18}) {
        CheckSelectedProfile(profile, kind.type, linked, audit);
      }
    }
  }
  for (std::size_t i = kFirstAuditLookup; i < agiru::kImplicitSystemFields.size(); ++i) {
    const auto &field = agiru::kImplicitSystemFields[i];
    CHECK_TRUE("user-name/full-name capacities match the original profile",
               field.length == (i % 2 == 0 ? 50 : 80));
    CHECK_TRUE("lookup owners distinguish created and modified audit GUIDs",
               field.auditOwner.Value() == (i < 8 ? 2000000002 : 2000000004));
    CHECK_TRUE("lookups distinguish User name and full-name source fields",
               field.userField.Value() == (i % 2 == 0 ? 2 : 3));
  }
}
}

int main() {
  return gate::Run("PlatformSystemFields", [] {
    CompleteProfilesRespectVersionKindAndLinkedObject();
    CheckSystemFields<agiru::platform::AllObj>();
    CheckSystemFields<agiru::platform::AllObjWithCaption>();
    CheckSystemFields<agiru::platform::AllProfile>();
    CheckSystemFields<agiru::platform::Company>();
    CheckSystemFields<agiru::platform::Date>();
    CheckSystemFields<agiru::platform::FeatureKey>();
    CheckSystemFields<agiru::platform::Field>();
    CheckSystemFields<agiru::platform::Integer>();
    CheckSystemFields<agiru::platform::ODataEdmType>();
    CheckSystemFields<agiru::platform::ObjectOptions>();
    CheckSystemFields<agiru::platform::PageMetadata>();
    CheckSystemFields<agiru::platform::PageTableField>();
    CheckSystemFields<agiru::platform::PrivacyNotice>();
    CheckSystemFields<agiru::platform::PrivacyNoticeApproval>();
    CheckSystemFields<agiru::platform::RecordLink>();
    CheckSystemFields<agiru::platform::TableMetadata>();
    CheckSystemFields<agiru::platform::User>();
    CheckSystemFields<agiru::platform::UserPersonalization>();
  });
}
