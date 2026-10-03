#include "meta/Ids.h"
#include "meta/TableDef.h"
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
};

// devenv-table-system-fields.md: the base SystemId/audit contract, independent of Declare.h.
constexpr std::array<SystemSpec, 5> kSystemFields{{
    {.number = 2000000000, .name = "SystemId", .type = agiru::FieldType::Guid},
    {.number = 2000000001, .name = "SystemCreatedAt", .type = agiru::FieldType::DateTime},
    {.number = 2000000002, .name = "SystemCreatedBy", .type = agiru::FieldType::Guid},
    {.number = 2000000003, .name = "SystemModifiedAt", .type = agiru::FieldType::DateTime},
    {.number = 2000000004, .name = "SystemModifiedBy", .type = agiru::FieldType::Guid},
}};
constexpr auto kCreatedMilliseconds = 123456789;
constexpr auto kModifiedMilliseconds = 234567890;

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
    if (field.no.Value() < kSystemFields.front().number) { ++declared; }
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
    CHECK_TEXT(identity + " reflects the typed member",
               field == nullptr ? "<missing>" : reflected.Field(expected.number).ToText(),
               values[index]);
  }
}

}

int main() {
  return gate::Run("PlatformSystemFields", [] {
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
