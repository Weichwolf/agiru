#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/User.h"
#include "platform/UserPersonalization.h"
#include "runtime/Database.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Boolean.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <string_view>

using agiru::FieldClass;
using agiru::FieldType;
using agiru::platform::UserPersonalization;

namespace {

// System UserPersonalization.Table.al, not the runtime's previous nine-field subset.
struct FieldSpec {
  int number;
  std::string_view name;
  FieldType type;
  int length = 0;
  std::string_view formula{};
};

constexpr std::array<FieldSpec, 19> kFields{{
    {.number = 3, .name = "User SID", .type = FieldType::Guid},
    {.number = 6,
     .name = "User ID",
     .type = FieldType::Code,
     .length = 50,
     .formula = R"(Lookup(User."User Name" WHERE("User Security ID" = FIELD("User SID"))))"},
    {.number = 7,
     .name = "Full Name",
     .type = FieldType::Text,
     .length = 80,
     .formula = R"(Lookup(User."Full Name" WHERE("User Name" = FIELD("User ID"))))"},
    {.number = 9, .name = "Profile ID", .type = FieldType::Code, .length = 30},
    {.number = 10, .name = "App ID", .type = FieldType::Guid},
    {.number = 11, .name = "Scope", .type = FieldType::Option},
    {.number = 12, .name = "Language ID", .type = FieldType::Integer},
    {.number = 13,
     .name = "Language Name",
     .type = FieldType::Text,
     .length = 80,
     .formula = R"(Lookup("Windows Language".Name WHERE("Language ID" = FIELD("Language ID"))))"},
    {.number = 15, .name = "Company", .type = FieldType::Text, .length = 30},
    {.number = 18, .name = "Debugger Break On Error", .type = FieldType::Boolean},
    {.number = 21, .name = "Debugger Break On Rec Changes", .type = FieldType::Boolean},
    {.number = 24, .name = "Debugger Skip System Triggers", .type = FieldType::Boolean},
    {.number = 27, .name = "Locale ID", .type = FieldType::Integer},
    {.number = 28,
     .name = "Region",
     .type = FieldType::Text,
     .length = 80,
     .formula = R"(Lookup("Windows Language".Name WHERE("Language ID" = FIELD("Locale ID"))))"},
    {.number = 30, .name = "Time Zone", .type = FieldType::Text, .length = 180},
    {.number = 31,
     .name = "License Type",
     .type = FieldType::Option,
     .formula = R"(Lookup(User."License Type" WHERE("User Security ID" = FIELD("User SID"))))"},
    {.number = 32, .name = "Customization Status", .type = FieldType::Option},
    {.number = 33,
     .name = "Role",
     .type = FieldType::Text,
     .length = 100,
     .formula = "Lookup(\"All Profile\".Caption WHERE(\"Profile ID\" = FIELD(\"Profile ID\"), "
                "Scope = FIELD(Scope), \"App ID\" = FIELD(\"App ID\")))"},
    {.number = 34, .name = "Emit Version", .type = FieldType::Integer},
}};

const agiru::TableDef &Declaration() {
  return agiru::TableTraits<UserPersonalization>::kTable;
}

constexpr agiru::FieldNo kPersonalizationLicenseField{31};
constexpr int kBreakOnErrorField = 18;

void TheCompleteDeclarationRetainsItsProperties() {
  const auto &table = Declaration();
  CHECK_TRUE("all declared fields and five platform system fields",
             table.fields.size() == kFields.size() + agiru::kSystemFieldCount);
  CHECK_TRUE("personalization is tenant-wide", !table.dataPerCompany);
  CHECK_TRUE("personalization is not replicated", !table.replicateData);
  for (const auto &expected : kFields) {
    const auto *field = agiru::Field(table, agiru::FieldNo{expected.number});
    CHECK_TRUE(expected.name, field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT("declared name", field->name, expected.name);
    CHECK_TEXT(
        "declared caption", field->caption, expected.number == 13 ? "Language" : expected.name);
    CHECK_TRUE("declared type", field->type == expected.type);
    CHECK_TRUE("declared length", field->length == expected.length);
    CHECK_TEXT("declared formula", field->calcFormula, expected.formula);
    CHECK_TRUE("lookup fields are not normal columns",
               field->fieldClass ==
                   (expected.formula.empty() ? FieldClass::Normal : FieldClass::FlowField));
    const bool removed = expected.number == 18 || expected.number == 21 || expected.number == 24;
    CHECK_TEXT("only declared removed fields", field->obsoleteState, removed ? "Removed" : "");
    CHECK_TEXT("removed reason",
               field->obsoleteReason,
               removed ? "Support for the classic debugger engine has been removed." : "");
    CHECK_TRUE("normal fields remain stored even when marked Removed",
               agiru::Stored(*field) == expected.formula.empty());
    CHECK_TEXT(
        "only declared internal access", field->access, expected.number == 32 ? "Internal" : "");
    const bool initialized = expected.number == 18 || expected.number == 24;
    CHECK_TRUE("only declared InitValues", field->initValue.has_value() == initialized);
    if (initialized) { CHECK_TEXT("debugger InitValue", field->initValue.value_or(""), "true"); }
  }
  constexpr std::array<int, 3> keyFields{3, 9, 15};
  CHECK_TRUE("all three declared keys", table.keys.size() == keyFields.size());
  for (std::size_t i = 0; i < table.keys.size() && i < keyFields.size(); ++i) {
    CHECK_TRUE("declared key field",
               table.keys[i].fields.size() == 1 && table.keys[i].fields[0].Value() == keyFields[i]);
    CHECK_TRUE("only the primary key is clustered", table.keys[i].clustered == (i == 0));
  }
}

void LicenseMembersHaveOneSharedDeclaration() {
  const auto *user =
      agiru::Field(agiru::TableTraits<agiru::platform::User>::kTable, agiru::FieldNo{10});
  const auto *personalization = agiru::Field(Declaration(), kPersonalizationLicenseField);
  CHECK_TRUE("both declared license fields exist", user != nullptr && personalization != nullptr);
  if (user == nullptr || personalization == nullptr) { return; }
  CHECK_TRUE("both fields borrow the same immutable vocabulary",
             user->values.data() == personalization->values.data());
  CHECK_TRUE("all ten System license members", personalization->values.size() == 10);
  constexpr std::array<std::string_view, 10> members{"Full User",
                                                     "Limited User",
                                                     "Device Only User",
                                                     "Windows Group",
                                                     "External User",
                                                     "External Administrator",
                                                     "External Accountant",
                                                     "Application",
                                                     "AAD Group",
                                                     "Agent"};
  for (std::size_t i = 0; i < members.size() && i < personalization->values.size(); ++i) {
    CHECK_TEXT("declared license member", personalization->values[i].name, members[i]);
    CHECK_TEXT("declared license caption", personalization->values[i].caption, members[i]);
    CHECK_TRUE("declared license ordinal",
               personalization->values[i].ordinal == static_cast<int>(i));
  }
}

void CustomizationMembersAreDeclaredNotInvented() {
  constexpr agiru::FieldNo kCustomizationField{32};
  const auto *field = agiru::Field(Declaration(), kCustomizationField);
  CHECK_TRUE("customization field exists", field != nullptr);
  if (field == nullptr) { return; }
  constexpr std::array<std::string_view, 3> members{
      "Updated", "Recompilation Needed", "Recompilation Failed"};
  CHECK_TRUE("all three customization members", field->values.size() == members.size());
  for (std::size_t i = 0; i < members.size() && i < field->values.size(); ++i) {
    CHECK_TEXT("declared customization member", field->values[i].name, members[i]);
    CHECK_TEXT("declared customization caption", field->values[i].caption, members[i]);
    CHECK_TRUE("declared customization ordinal", field->values[i].ordinal == static_cast<int>(i));
  }
}

void MetadataDrivesSQLAndLookups() {
  agiru::Session session(AGIRU_TEST_DSN);
  const auto &connection = session.Database();
  const auto boundary = session.Transaction().Open(connection);
  connection.Run("CREATE SCHEMA agiru_gate_user_personalization");
  connection.Run("SET LOCAL search_path = agiru_gate_user_personalization");
  agiru::CreateTable(connection, Declaration());
  agiru::CreateTable(connection, agiru::TableTraits<agiru::platform::User>::kTable);
  connection.Run("CREATE TABLE \"All Profile\" (\"Scope\" integer, \"App ID\" uuid, \"Profile ID\" "
                 "varchar(30), \"Caption\" varchar(100))");
  connection.Run(
      "INSERT INTO \"User\" (\"User Security ID\", \"User Name\", \"Full Name\", \"License Type\") "
      "VALUES ('00000000-0000-0000-0000-000000000073', 'GATE USER', 'Gate Full Name', 9)");
  connection.Run(
      "INSERT INTO \"All Profile\" VALUES (0, '00000000-0000-0000-0000-000000000073', 'GATE "
      "PROFILE', 'Wrong Scope'), (1, '00000000-0000-0000-0000-000000000074', 'GATE PROFILE', "
      "'Wrong App'), (1, '00000000-0000-0000-0000-000000000073', 'GATE PROFILE', 'Correct Role')");

  const auto columns = connection.Execute(
      "SELECT column_name FROM information_schema.columns WHERE table_schema = "
      "'agiru_gate_user_personalization' AND table_name = 'User Personalization'");
  constexpr std::size_t kStoredDeclaredFields = 13;
  CHECK_TRUE("thirteen stored declared fields plus five system fields",
             columns.Rows() == kStoredDeclaredFields + agiru::kSystemFieldCount);
  for (const auto &expected : kFields) {
    bool found = false;
    for (std::size_t row = 0; row < columns.Rows(); ++row) {
      found = found || columns.Value(row, 0) == expected.name;
    }
    CHECK_TRUE("SQL excludes only lookup fields, preserving obsolete normal fields",
               found == expected.formula.empty());
  }

  UserPersonalization record;
  record.UserSID = "00000000-0000-0000-0000-000000000073";
  record.AppID = record.UserSID;
  record.ProfileID = "GATE PROFILE";
  record.Scope = agiru::platform::PersonalizationScope::Tenant;
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("RecordRef counts declared fields independently of system fields",
             reflected.FieldCount() == static_cast<int>(kFields.size()));
  for (const int field : std::array{6, 7, 31, 33}) {
    CHECK_TRUE("generic lookup calculation", reflected.Field(field).CalcField());
  }
  CHECK_TEXT("User ID lookup uses User SID", reflected.Field(6).ToText(), "GATE USER");
  CHECK_TEXT(
      "Full Name lookup follows calculated User ID", reflected.Field(7).ToText(), "Gate Full Name");
  CHECK_TRUE("license lookup retains ordinal nine",
             reflected.Field(31).Value().Get<agiru::OrdinalInVariant>().ordinal == 9);
  CHECK_TEXT("Role lookup constrains all three declared fields",
             reflected.Field(33).ToText(),
             "Correct Role");
  for (const int field : std::array{18, 24}) {
    CHECK_TRUE("new record uses declared debugger defaults",
               reflected.Field(field).Value().Get<agiru::Boolean>());
    reflected.Field(field).Value(agiru::Variant(false));
  }
  reflected.SetTable(record);
  record.Init();
  reflected.GetTable(record);
  for (const int field : std::array{18, 24}) {
    CHECK_TRUE("Init restores declared defaults",
               reflected.Field(field).Value().Get<agiru::Boolean>());
  }
  reflected.Field(kBreakOnErrorField).Value(agiru::Variant(false));
  reflected.SetTable(record);
  record.Insert();
  UserPersonalization read;
  CHECK_TRUE("declared record survives SQL roundtrip", static_cast<bool>(read.Get(record.UserSID)));
  reflected.GetTable(read);
  CHECK_TRUE("Removed normal field retains its stored false value",
             !reflected.Field(kBreakOnErrorField).Value().Get<agiru::Boolean>());
  CHECK_TRUE("second Removed normal field retains its initialized true value",
             reflected.Field(24).Value().Get<agiru::Boolean>());
  session.Transaction().Rollback(connection, boundary);
}

}

int main() {
  return gate::Run("UserPersonalization", [] {
    TheCompleteDeclarationRetainsItsProperties();
    LicenseMembersHaveOneSharedDeclaration();
    CustomizationMembersAreDeclaredNotInvented();
    MetadataDrivesSQLAndLookups();
  });
}
