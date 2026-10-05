#include "meta/Declare.h"
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
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
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
constexpr auto kSampleRowVersion = 123;
constexpr std::size_t kFirstAuditLookup = kSystemFields.size() + 1;

// Independent source field counts from the original System 29.0.55365.0 declarations.
struct OriginalSourceFields {
  static constexpr int AllObj = 7;
  static constexpr int AllObjWithCaption = 10;
  static constexpr int AllProfile = 17;
  static constexpr int Company = 5;
  static constexpr int Date = 6;
  static constexpr int FeatureKey = 10;
  static constexpr int Field = 24;
  static constexpr int Integer = 1;
  static constexpr int ODataEdmType = 3;
  static constexpr int ObjectOptions = 9;
  static constexpr int PageMetadata = 32;
  static constexpr int PageTableField = 15;
  static constexpr int PrivacyNotice = 6;
  static constexpr int PrivacyNoticeApproval = 4;
  static constexpr int RecordLink = 14;
  static constexpr int TableMetadata = 23;
  static constexpr int User = 12;
  static constexpr int UserPersonalization = 19;
};

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

template <agiru::SystemFieldProfile Profile, agiru::TableType Type, bool Linked>
void MaterializedProfile(bool audit) {
  using Row = agiru::platform::TableMetadata;
  constexpr auto fields =
      agiru::WithImplicitFields<Row, Profile, Type, Linked>(std::array<agiru::FieldDef, 0>{});
  const std::size_t expected = !audit                                            ? 2
                               : Profile == agiru::SystemFieldProfile::Runtime18 ? 10
                                                                                 : 6;
  CHECK_TRUE("materialization retains the complete selected denominator",
             fields.size() == expected);
  constexpr std::array offsets{offsetof(Row, SystemRowVersion),
                               offsetof(Row, SystemId),
                               offsetof(Row, SystemCreatedAt),
                               offsetof(Row, SystemCreatedBy),
                               offsetof(Row, SystemModifiedAt),
                               offsetof(Row, SystemModifiedBy),
                               offsetof(Row, SystemCreatedByUserName),
                               offsetof(Row, SystemCreatedByFullName),
                               offsetof(Row, SystemModifiedByUserName),
                               offsetof(Row, SystemModifiedByFullName)};
  constexpr std::array types{agiru::FieldType::BigInteger,
                             agiru::FieldType::Guid,
                             agiru::FieldType::DateTime,
                             agiru::FieldType::Guid,
                             agiru::FieldType::DateTime,
                             agiru::FieldType::Guid};
  constexpr std::array calculations{
      R"(lookup(User."User Name" where("User Security ID" = field(SystemCreatedBy))))",
      R"(lookup(User."Full Name" where("User Security ID" = field(SystemCreatedBy))))",
      R"(lookup(User."User Name" where("User Security ID" = field(SystemModifiedBy))))",
      R"(lookup(User."Full Name" where("User Security ID" = field(SystemModifiedBy))))"};
  for (std::size_t i = 0; i < fields.size(); ++i) {
    const auto &field = fields[i];
    CHECK_TRUE("materialized offsets reach the actual selected members",
               field.offset == offsets[i]);
    CHECK_TRUE("timestamp precedes sorted reserved identities",
               i == 0 ? field.no.Value() == 0
                      : field.no.Value() == 2000000000 + static_cast<int>(i - 1));
    CHECK_TRUE("only timestamp aliases SQL rowversion", field.sqlTimestamp == (i == 0));
    CHECK_TRUE("materialized timestamp, identity and audit types match the original profile",
               i >= types.size() || field.type == types[i]);
    CHECK_TRUE("lookups are nonstored FlowFields",
               field.fieldClass == (i >= kFirstAuditLookup ? agiru::FieldClass::FlowField
                                                           : agiru::FieldClass::Normal));
    CHECK_TRUE("only timestamp and lookup buffers are declared read-only",
               field.editable == (i != 0 && i < kFirstAuditLookup));
    if (i >= kFirstAuditLookup) {
      CHECK_TRUE("lookup text has its independently specified capacity",
                 field.type == agiru::FieldType::Text && field.length == (i % 2 == 0 ? 50 : 80));
      CHECK_TRUE("every materialized lookup retains a calculation", !field.calcFormula.empty());
      CHECK_TEXT("lookup calculations retain the original SID owner and User field",
                 field.calcFormula,
                 calculations[i - kFirstAuditLookup]);
    }
  }
}

template <agiru::TableType Type, bool Linked> void MaterializedHostProfiles(bool audit) {
  MaterializedProfile<agiru::SystemFieldProfile::Runtime17, Type, Linked>(audit);
  MaterializedProfile<agiru::SystemFieldProfile::Runtime18, Type, Linked>(audit);
}

template <bool Linked> void MaterializedKinds() {
  using agiru::TableType;
  MaterializedHostProfiles<TableType::Normal, Linked>(!Linked);
  MaterializedHostProfiles<TableType::Temporary, Linked>(!Linked);
  MaterializedHostProfiles<TableType::CRM, Linked>(false);
  MaterializedHostProfiles<TableType::CDS, Linked>(false);
  MaterializedHostProfiles<TableType::ExternalSQL, Linked>(false);
  MaterializedHostProfiles<TableType::Exchange, Linked>(false);
  MaterializedHostProfiles<TableType::MicrosoftGraph, Linked>(false);
}

template <typename Row> void NativeHasEveryEffectiveMember(int declaredCount) {
  const auto &table = agiru::TableTraits<Row>::kTable;
  const std::string identity(table.name);
  CHECK_TRUE(identity + " has its original declarations plus ten implicit fields",
             table.fields.size() == static_cast<std::size_t>(declaredCount) + 10);
  Row row;
  row.SystemRowVersion = kSampleRowVersion;
  row.SystemCreatedByUserName = "Created User";
  row.SystemCreatedByFullName = "Created full name";
  row.SystemModifiedByUserName = "Modified User";
  row.SystemModifiedByFullName = "Modified full name";
  agiru::RecordRef reference;
  reference.GetTable(row);
  CHECK_TRUE(identity + " indexes exactly its original declarations",
             reference.FieldCount() == declaredCount);
  constexpr std::array numbers{Row::Field_No::SystemRowVersion.Value(),
                               Row::Field_No::SystemCreatedByUserName.Value(),
                               Row::Field_No::SystemCreatedByFullName.Value(),
                               Row::Field_No::SystemModifiedByUserName.Value(),
                               Row::Field_No::SystemModifiedByFullName.Value()};
  constexpr std::array offsets{offsetof(Row, SystemRowVersion),
                               offsetof(Row, SystemCreatedByUserName),
                               offsetof(Row, SystemCreatedByFullName),
                               offsetof(Row, SystemModifiedByUserName),
                               offsetof(Row, SystemModifiedByFullName)};
  constexpr std::array sourceNames{"SystemRowVersion",
                                   "SystemCreatedByUserName",
                                   "SystemCreatedByFullName",
                                   "SystemModifiedByUserName",
                                   "SystemModifiedByFullName"};
  constexpr std::array calculations{
      R"(lookup(User."User Name" where("User Security ID" = field(SystemCreatedBy))))",
      R"(lookup(User."Full Name" where("User Security ID" = field(SystemCreatedBy))))",
      R"(lookup(User."User Name" where("User Security ID" = field(SystemModifiedBy))))",
      R"(lookup(User."Full Name" where("User Security ID" = field(SystemModifiedBy))))"};
  for (std::size_t i = 0; i < numbers.size(); ++i) {
    const int number = i == 0 ? 0 : 2000000004 + static_cast<int>(i);
    const auto *field = agiru::Field(table, agiru::FieldNo{number});
    CHECK_TRUE(identity + " exposes the original effective Field_No", numbers[i] == number);
    CHECK_TRUE(identity + " exposes every effective member", field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT(identity + " preserves the effective source name", field->name, sourceNames[i]);
    CHECK_TRUE(identity + " reaches the actual effective member", field->offset == offsets[i]);
    CHECK_TRUE(identity + " preserves the original effective type",
               field->type == (i == 0 ? agiru::FieldType::BigInteger : agiru::FieldType::Text));
    CHECK_TRUE(identity + " keeps effective members read-only", !field->editable);
    CHECK_TRUE(identity + " aliases only timestamp to SQL rowversion",
               field->sqlTimestamp == (i == 0));
    CHECK_TRUE(identity + " keeps user lookups nonstored",
               field->fieldClass ==
                   (i == 0 ? agiru::FieldClass::Normal : agiru::FieldClass::FlowField));
    CHECK_TRUE(identity + " preserves effective capacities",
               field->length == (i == 0       ? 0
                                 : i % 2 == 1 ? 50
                                              : 80));
    CHECK_TEXT(identity + " preserves original effective calculations",
               field->calcFormula,
               i == 0 ? "" : calculations[i - 1]);
  }
  CHECK_TEXT("timestamp reads the typed BigInteger member", reference.Field(0).ToText(), "123");
  CHECK_TEXT(
      "timestamp retains the original reflection alias", reference.Field(0).Name(), "timestamp");
  constexpr std::array names{
      "Created User", "Created full name", "Modified User", "Modified full name"};
  for (std::size_t i = 0; i < names.size(); ++i) {
    const auto field = reference.Field(2000000005 + static_cast<int>(i));
    CHECK_TEXT("native lookup reflection reaches its typed buffer", field.ToText(), names[i]);
  }
}

void NativeProfilesAndIdentityOnlyStorage() {
  CHECK_TRUE("original Table Metadata has 23 declared plus ten implicit fields",
             agiru::TableTraits<agiru::platform::TableMetadata>::kTable.fields.size() == 33);
  NativeHasEveryEffectiveMember<agiru::platform::AllObj>(OriginalSourceFields::AllObj);
  NativeHasEveryEffectiveMember<agiru::platform::AllObjWithCaption>(
      OriginalSourceFields::AllObjWithCaption);
  NativeHasEveryEffectiveMember<agiru::platform::AllProfile>(OriginalSourceFields::AllProfile);
  NativeHasEveryEffectiveMember<agiru::platform::Company>(OriginalSourceFields::Company);
  NativeHasEveryEffectiveMember<agiru::platform::Date>(OriginalSourceFields::Date);
  NativeHasEveryEffectiveMember<agiru::platform::FeatureKey>(OriginalSourceFields::FeatureKey);
  NativeHasEveryEffectiveMember<agiru::platform::Field>(OriginalSourceFields::Field);
  NativeHasEveryEffectiveMember<agiru::platform::Integer>(OriginalSourceFields::Integer);
  NativeHasEveryEffectiveMember<agiru::platform::ODataEdmType>(OriginalSourceFields::ODataEdmType);
  NativeHasEveryEffectiveMember<agiru::platform::ObjectOptions>(
      OriginalSourceFields::ObjectOptions);
  NativeHasEveryEffectiveMember<agiru::platform::PageMetadata>(OriginalSourceFields::PageMetadata);
  NativeHasEveryEffectiveMember<agiru::platform::PageTableField>(
      OriginalSourceFields::PageTableField);
  NativeHasEveryEffectiveMember<agiru::platform::PrivacyNotice>(
      OriginalSourceFields::PrivacyNotice);
  NativeHasEveryEffectiveMember<agiru::platform::PrivacyNoticeApproval>(
      OriginalSourceFields::PrivacyNoticeApproval);
  NativeHasEveryEffectiveMember<agiru::platform::RecordLink>(OriginalSourceFields::RecordLink);
  NativeHasEveryEffectiveMember<agiru::platform::TableMetadata>(
      OriginalSourceFields::TableMetadata);
  NativeHasEveryEffectiveMember<agiru::platform::User>(OriginalSourceFields::User);
  NativeHasEveryEffectiveMember<agiru::platform::UserPersonalization>(
      OriginalSourceFields::UserPersonalization);

  struct IdentityOnly {
    agiru::BigInteger SystemRowVersion{};
    agiru::Guid SystemId;
  };

  constexpr auto linked = agiru::WithImplicitFields<IdentityOnly,
                                                    agiru::SystemFieldProfile::Runtime18,
                                                    agiru::TableType::Normal,
                                                    true>(std::array<agiru::FieldDef, 0>{});
  CHECK_TRUE("linked materialization requires no audit or lookup storage", linked.size() == 2);
}

void InvalidImplicitStorageAndDeclarationsRefuse() {
  using Row = agiru::platform::TableMetadata;
  const auto refuses = [](auto operation, std::string_view reason) {
    bool refused = false;
    try {
      operation();
    } catch (const agiru::Error &error) {
      refused = std::string_view(error.what()).contains(reason);
    }
    CHECK_TRUE("invalid implicit storage/source declarations refuse explicitly", refused);
  };

  struct WrongIdentity {
    agiru::BigInteger SystemRowVersion{};
    agiru::DateTime SystemId;
  };

  refuses(
      [] {
        static_cast<void>(agiru::WithImplicitFields<WrongIdentity,
                                                    agiru::SystemFieldProfile::Runtime18,
                                                    agiru::TableType::Normal,
                                                    true>(std::array<agiru::FieldDef, 0>{}));
      },
      "original type or capacity");
  for (const auto number : {0, -1, 2000000000}) {
    refuses(
        [&] {
          const std::array declared{agiru::FieldDef{.no = agiru::FieldNo{number}}};
          static_cast<void>(agiru::WithImplicitFields<Row,
                                                      agiru::SystemFieldProfile::Runtime18,
                                                      agiru::TableType::Normal,
                                                      false>(declared));
        },
        "nonreserved source fields");
  }
  for (const auto second : {1, 2}) {
    refuses(
        [&] {
          const std::array declared{agiru::FieldDef{.no = agiru::FieldNo{2}},
                                    agiru::FieldDef{.no = agiru::FieldNo{second}}};
          static_cast<void>(agiru::WithImplicitFields<Row,
                                                      agiru::SystemFieldProfile::Runtime18,
                                                      agiru::TableType::Normal,
                                                      false>(declared));
        },
        "sorted, distinct");
  }
}
}

int main() {
  return gate::Run("PlatformSystemFields", [] {
    CompleteProfilesRespectVersionKindAndLinkedObject();
    MaterializedKinds<false>();
    MaterializedKinds<true>();
    NativeProfilesAndIdentityOnlyStorage();
    InvalidImplicitStorageAndDeclarationsRefuse();
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
