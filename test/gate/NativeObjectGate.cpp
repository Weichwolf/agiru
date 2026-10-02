#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"
#include "platform/AllProfile.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/Integer.h"
#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"
#include "platform/RecordLink.h"
#include "platform/TableMetadata.h"
#include "platform/User.h"
#include "runtime/Catalogue.h"
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
#include <utility>

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
constexpr std::array<int, 6> kAllObjFieldNumbers{1, 3, 4, 60, 61, 62};
constexpr std::array<int, 4> kAllObjUndeclaredFields{7, 20, 21, 63};
constexpr int kAllObjNamespaceField = 62;
constexpr int kAllObjNamespaceLength = 500;
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

void AllObjUsesItsOwnDeclaration() {
  const auto &table = agiru::TableTraits<agiru::platform::AllObj>::kTable;
  CHECK_TRUE("AllObj retains six source fields", table.fields.size() == kAllObjFieldNumbers.size());
  CHECK_TEXT("AllObj retains its original permissions", table.inherentPermissions, "rX");
  CHECK_TRUE("AllObj declares only its source primary key", table.keys.size() == 1);
  if (!table.keys.empty()) {
    CHECK_TEXT("AllObj retains the source key name", table.keys.front().name, "pk");
    CHECK_TRUE("AllObj primary key retains both source fields",
               table.keys.front().fields.size() == kPrimaryKey.size());
    const auto count = std::min(table.keys.front().fields.size(), kPrimaryKey.size());
    for (std::size_t i = 0; i < count; ++i) {
      CHECK_TRUE("AllObj primary key retains source order",
                 table.keys.front().fields[i].Value() == kPrimaryKey[i]);
    }
  }
  agiru::platform::AllObj record;
  record.ObjectName = "Acc. Schedule Name";
  record.AppPackageID = agiru::Guid(kPackage);
  record.AppRuntimePackageID = agiru::Guid(kRuntimePackage);
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("AllObj reflection sees only the source fields",
             reflected.FieldCount() == static_cast<int>(kAllObjFieldNumbers.size()));
  for (const int number : kAllObjFieldNumbers) {
    CHECK_TRUE("AllObj reflection uses original field numbers", reflected.FieldExist(number));
  }
  for (const int number : kAllObjUndeclaredFields) {
    CHECK_TRUE("caption-only and invented AllObj fields stay absent",
               !reflected.FieldExist(number));
  }
  CHECK_TEXT("AllObj preserves original name case and punctuation",
             record.FieldFormat(agiru::platform::AllObj::Field_No::ObjectName),
             "Acc. Schedule Name");
  const std::string full(kAllObjNamespaceLength, 'x');
  const bool hasNamespace = reflected.FieldExist(kAllObjNamespaceField);
  if (hasNamespace) {
    reflected.Field(kAllObjNamespaceField).Value(agiru::Variant(full));
    reflected.SetTable(record);
  }
  CHECK_TRUE("AllObj namespace retains its full declared length",
             hasNamespace && reflected.Field(kAllObjNamespaceField).ToText() == full);
  CHECK_TRUE("AllObj field 62 is Text, not the captioned table's Guid",
             hasNamespace && record.FieldFormat(agiru::FieldNo{kAllObjNamespaceField}) == full);
  CHECK_TRUE("AllObj namespace cannot replace the package ID",
             hasNamespace && record.AppPackageID.ToText() == "{" + std::string(kPackage) + "}");
  CHECK_TRUE("AllObj namespace cannot replace the runtime package ID",
             hasNamespace &&
                 record.AppRuntimePackageID.ToText() == "{" + std::string(kRuntimePackage) + "}");
}

void PrivacyDeclarationsMatchSystemSource() {
  const auto &notice = agiru::TableTraits<agiru::platform::PrivacyNotice>::kTable;
  const auto &approval = agiru::TableTraits<agiru::platform::PrivacyNoticeApproval>::kTable;
  CHECK_TRUE("notice ID comes from System symbols", notice.id.Value() == 2000000237);
  CHECK_TRUE("approval ID comes from System symbols", approval.id.Value() == 2000000238);
  CHECK_TEXT("original notice name", notice.name, "Privacy Notice");
  CHECK_TEXT("original approval name", approval.name, "Privacy Notice Approval");
  CHECK_TRUE("notices are shared across companies", !notice.dataPerCompany);
  CHECK_TRUE("approvals are shared across companies", !approval.dataPerCompany);
  CHECK_TRUE("source disables notice replication", !notice.replicateData);
  CHECK_TRUE("source disables approval replication", !approval.replicateData);
  constexpr std::array<FieldSpec, 6> fields{{
      {.number = 1, .name = "ID", .type = agiru::FieldType::Code, .length = 50},
      {.number = 2,
       .name = "Integration Service Name",
       .type = agiru::FieldType::Text,
       .length = 250},
      {.number = 3, .name = "Link", .type = agiru::FieldType::Text, .length = 2048},
      {.number = 4, .name = "User SID Filter", .type = agiru::FieldType::Guid},
      {.number = 5, .name = "Enabled", .type = agiru::FieldType::Boolean},
      {.number = 6, .name = "Disabled", .type = agiru::FieldType::Boolean},
  }};
  CHECK_TRUE("all six declared notice fields", notice.fields.size() == fields.size());
  for (const auto &expected : fields) {
    const auto *field = agiru::Field(notice, agiru::FieldNo{expected.number});
    CHECK_TRUE("each original notice number exists", field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT("original notice field name", field->name, expected.name);
    CHECK_TRUE("original notice field type", field->type == expected.type);
    CHECK_TRUE("original notice field length", field->length == expected.length);
    const auto fieldClass = expected.number == 4  ? agiru::FieldClass::FlowFilter
                            : expected.number > 4 ? agiru::FieldClass::FlowField
                                                  : agiru::FieldClass::Normal;
    CHECK_TRUE("original notice field class", field->fieldClass == fieldClass);
    CHECK_TRUE("only normal fields are stored", agiru::Stored(*field) == (expected.number < 4));
  }
  CHECK_TEXT("original ID caption", notice.fields[0].caption, "Privacy Notice ID");
  CHECK_TEXT("original Link caption", notice.fields[2].caption, "Privacy Link");
  CHECK_TEXT("filter relation retains the declared user field",
             notice.fields[3].relation,
             "User.\"User Security ID\"");
  CHECK_TEXT("Enabled is Exist over the source filter",
             notice.fields[4].calcFormula,
             "Exist(\"Privacy Notice Approval\" WHERE (ID = field(ID), \"User SID\" = "
             "field(\"User SID Filter\"), Approved = CONST(true)))");
  CHECK_TEXT("Disabled is Exist over the source filter",
             notice.fields[5].calcFormula,
             "Exist(\"Privacy Notice Approval\" WHERE (ID = field(ID), \"User SID\" = "
             "field(\"User SID Filter\"), Approved = CONST(false)))");
  CHECK_TRUE("both source notice keys", notice.keys.size() == 2);
  CHECK_TEXT(
      "notice primary key name", notice.keys.empty() ? "<missing>" : notice.keys[0].name, "Key1");
  CHECK_TRUE("notice primary key",
             !notice.keys.empty() && notice.keys[0].fields.size() == 1 &&
                 notice.keys[0].fields[0].Value() == 1 && notice.keys[0].clustered);
  CHECK_TEXT("notice secondary key name",
             notice.keys.size() < 2 ? "<missing>" : notice.keys[1].name,
             "Key2");
  CHECK_TRUE("notice secondary key is not clustered",
             notice.keys.size() > 1 && !notice.keys[1].clustered);
  CHECK_TRUE("notice secondary key names Integration Service Name",
             notice.keys.size() > 1 && notice.keys[1].fields.size() == 1 &&
                 notice.keys[1].fields[0].Value() == 2);
  constexpr std::array<FieldSpec, 4> approvalFields{{
      {.number = 1, .name = "ID", .type = agiru::FieldType::Code, .length = 50},
      {.number = 2, .name = "User SID", .type = agiru::FieldType::Guid},
      {.number = 3, .name = "Approver User SID", .type = agiru::FieldType::Guid},
      {.number = 4, .name = "Approved", .type = agiru::FieldType::Boolean},
  }};
  CHECK_TRUE("all four declared approval fields", approval.fields.size() == approvalFields.size());
  for (const auto &expected : approvalFields) {
    const auto *field = agiru::Field(approval, agiru::FieldNo{expected.number});
    CHECK_TRUE("each original approval number exists", field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT("original approval field name", field->name, expected.name);
    CHECK_TRUE("original approval field type", field->type == expected.type);
    CHECK_TRUE("original approval field length", field->length == expected.length);
    CHECK_TRUE("every approval field is stored", agiru::Stored(*field));
  }
  CHECK_TEXT("original approval ID caption", approval.fields[0].caption, "Privacy Notice ID");
  CHECK_TEXT("original approver caption", approval.fields[2].caption, "Approver User ID");
  CHECK_TEXT(
      "approval relation retains the notice", approval.fields[0].relation, "\"Privacy Notice\"");
  CHECK_TEXT("approval user relation", approval.fields[1].relation, "User.\"User Security ID\"");
  CHECK_TEXT("approver relation", approval.fields[2].relation, "User.\"User Security ID\"");
  CHECK_TRUE("one approval key", approval.keys.size() == 1);
  if (!approval.keys.empty()) {
    CHECK_TEXT("original approval key name", approval.keys[0].name, "Key1");
    CHECK_TRUE("approval key preserves notice and user order",
               approval.keys[0].fields.size() == 2 && approval.keys[0].fields[0].Value() == 1 &&
                   approval.keys[0].fields[1].Value() == 2 && approval.keys[0].clustered);
  }
}

void PrivacyReflectionAndCatalogueUseSourceIdentity() {
  agiru::platform::PrivacyNotice record;
  record.ID = "Mixed";
  record.IntegrationServiceName = " Mixed.Case Service ";
  const std::string link(2048, 'x');
  record.Link = link;
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("reflection retains the source notice ID", reflected.Number() == 2000000237);
  CHECK_TRUE("reflection retains the source field count", reflected.FieldCount() == 6);
  CHECK_TEXT("Link retains the full source length", reflected.Field(3).ToText(), link);
  const auto hasFilter = reflected.Field(4).Type() == agiru::FieldType::Guid;
  CHECK_TRUE("field 4 is Guid, not an approval Boolean", hasFilter);
  if (hasFilter) { reflected.Field(4).Value(agiru::Variant(agiru::Guid(kApplication))); }
  reflected.SetTable(record);
  CHECK_TRUE("reflection assigns the original filter slot",
             hasFilter && record.UserSIDFilter == agiru::Guid(kApplication));
  CHECK_TEXT("filter reflection does not change Link", record.Link, link);
  CHECK_TEXT("service name stays Text", record.IntegrationServiceName, " Mixed.Case Service ");
  const auto *notice = agiru::FindTable(agiru::TableId{2000000237});
  const auto *approval = agiru::FindTable(agiru::TableId{2000000238});
  CHECK_TRUE("runtime registers the source notice ID", notice != nullptr);
  CHECK_TRUE("runtime registers the source approval ID", approval != nullptr);
  CHECK_TRUE("notice name lookup shares numeric identity",
             notice == agiru::FindTable("Privacy Notice"));
  CHECK_TRUE("approval name lookup shares numeric identity",
             approval == agiru::FindTable("Privacy Notice Approval"));
  CHECK_TRUE("guessed notice ID is not registered",
             agiru::FindTable(agiru::TableId{1560}) == nullptr);
  CHECK_TRUE("guessed approval ID is not registered",
             agiru::FindTable(agiru::TableId{1561}) == nullptr);
}

void AllProfileDeclarationMatchesSystemSource() {
  const auto &table = agiru::TableTraits<agiru::platform::AllProfile>::kTable;
  constexpr std::array<FieldSpec, 17> fields{{
      {.number = 1, .name = "Scope", .type = agiru::FieldType::Option},
      {.number = 2, .name = "App ID", .type = agiru::FieldType::Guid},
      {.number = 3, .name = "Profile ID", .type = agiru::FieldType::Code, .length = 30},
      {.number = 4, .name = "Description", .type = agiru::FieldType::Text, .length = 2048},
      {.number = 5, .name = "Role Center ID", .type = agiru::FieldType::Integer},
      {.number = 6, .name = "Default Role Center", .type = agiru::FieldType::Boolean},
      {.number = 7, .name = "Use Comments", .type = agiru::FieldType::Boolean},
      {.number = 8, .name = "Use Notes", .type = agiru::FieldType::Boolean},
      {.number = 9, .name = "Use Record Notes", .type = agiru::FieldType::Boolean},
      {.number = 10, .name = "Record Notebook", .type = agiru::FieldType::Text, .length = 250},
      {.number = 11, .name = "Use Page Notes", .type = agiru::FieldType::Boolean},
      {.number = 12, .name = "Page Notebook", .type = agiru::FieldType::Text, .length = 250},
      {.number = 13, .name = "Disable Personalization", .type = agiru::FieldType::Boolean},
      {.number = 14, .name = "App Name", .type = agiru::FieldType::Text, .length = 250},
      {.number = 15, .name = "Enabled", .type = agiru::FieldType::Boolean},
      {.number = 16, .name = "Caption", .type = agiru::FieldType::Text, .length = 100},
      {.number = 17, .name = "Promoted", .type = agiru::FieldType::Boolean},
  }};
  CHECK_TRUE("all seventeen profile fields are retained", table.fields.size() == fields.size());
  CHECK_TRUE("source All Profile ID", table.id.Value() == 2000000178);
  CHECK_TEXT("source All Profile name", table.name, "All Profile");
  CHECK_TRUE("profile metadata is shared across companies", !table.dataPerCompany);
  CHECK_TEXT("source profile inherent permissions", table.inherentPermissions, "rX");
  const agiru::FieldDef missing{};
  for (const auto &expected : fields) {
    const auto *found = agiru::Field(table, agiru::FieldNo{expected.number});
    const auto &field = found == nullptr ? missing : *found;
    CHECK_TRUE("original profile number exists", found != nullptr);
    CHECK_TEXT("original profile field name", field.name, expected.name);
    CHECK_TEXT("default profile caption is the original name", field.caption, expected.name);
    CHECK_TRUE("original profile field type", field.type == expected.type);
    CHECK_TRUE("original profile field length", field.length == expected.length);
    CHECK_TRUE("profile fields are not invented FlowFields",
               found != nullptr && field.fieldClass == agiru::FieldClass::Normal);
    const bool pending = expected.number >= 7 && expected.number <= 12;
    CHECK_TEXT("source obsolescence state", field.obsoleteState, pending ? "Pending" : "");
    CHECK_TEXT("source obsolescence reason",
               field.obsoleteReason,
               pending ? "Capacity related to System profiles for which support has been removed."
                       : "");
  }
  CHECK_TRUE("only the source profile key", table.keys.size() == 1);
  CHECK_TEXT("source profile primary key name",
             table.keys.empty() ? "<missing>" : table.keys.front().name,
             "PK");
  CHECK_TRUE("source profile key order",
             !table.keys.empty() && table.keys[0].fields.size() == 3 &&
                 table.keys[0].fields[0].Value() == 1 && table.keys[0].fields[1].Value() == 2 &&
                 table.keys[0].fields[2].Value() == 3 && table.keys[0].clustered);
  CHECK_TRUE("invented profile fields remain absent",
             agiru::Field(table, agiru::FieldNo{18}) == nullptr);
}

void AllProfileReflectionRetainsIndependentSourceFields() {
  agiru::platform::AllProfile profile;
  const auto *description =
      agiru::Field(agiru::TableTraits<agiru::platform::AllProfile>::kTable, agiru::FieldNo{4});
  profile.Description = std::string(description != nullptr ? description->length : 0, 'd');
  profile.AppName = std::string(250, 'a');
  profile.Caption = std::string(100, 'c');
  profile.DisablePersonalization = true;
  profile.Enabled = true;
  profile.Promoted = true;
  agiru::RecordRef reflected;
  reflected.GetTable(profile);
  CHECK_TRUE("reflection sees every source profile field", reflected.FieldCount() == 17);
  for (const auto &entry : std::array<std::pair<int, std::string>, 6>{{{4, std::string(2048, 'd')},
                                                                       {13, "Yes"},
                                                                       {14, std::string(250, 'a')},
                                                                       {15, "Yes"},
                                                                       {16, std::string(100, 'c')},
                                                                       {17, "Yes"}}}) {
    CHECK_TRUE("reflection reads the original profile slot",
               reflected.FieldExist(entry.first) &&
                   reflected.Field(entry.first).ToText() == entry.second);
  }
  CHECK_TRUE("reflection reports only the source profile key", reflected.KeyCount() == 1);
  const auto &scope = agiru::OptionTraits<agiru::platform::PersonalizationScope>::kValues;
  CHECK_TRUE("the scope vocabulary stays dense",
             scope.size() == 2 && scope[0].ordinal == 0 && scope[1].ordinal == 1);
  CHECK_TEXT("source System scope name", scope[0].name, "System");
  CHECK_TEXT("source Tenant scope name", scope[1].name, "Tenant");
}

void RecordLinkDeclarationMatchesSystemSource() {
  const auto &table = agiru::TableTraits<agiru::platform::RecordLink>::kTable;
  constexpr std::array<FieldSpec, 14> fields{{
      {1, "Link ID", agiru::FieldType::Integer},
      {2, "Record ID", agiru::FieldType::RecordId},
      {3, "URL1", agiru::FieldType::Text, 2048},
      {4, "URL2", agiru::FieldType::Text, 250},
      {5, "URL3", agiru::FieldType::Text, 250},
      {6, "URL4", agiru::FieldType::Text, 250},
      {7, "Description", agiru::FieldType::Text, 250},
      {8, "Type", agiru::FieldType::Option},
      {9, "Note", agiru::FieldType::Blob},
      {10, "Created", agiru::FieldType::DateTime},
      {11, "User ID", agiru::FieldType::Text, 132},
      {12, "Company", agiru::FieldType::Text, 30},
      {13, "Notify", agiru::FieldType::Boolean},
      {14, "To User ID", agiru::FieldType::Text, 132},
  }};
  CHECK_TRUE("source record-link identity", table.id == agiru::TableId{2000000068});
  CHECK_TEXT("source record-link name", table.name, "Record Link");
  CHECK_TRUE("fourteen source fields plus five system fields", table.fields.size() == 19);
  CHECK_TRUE("record links are not company-scoped storage", !table.dataPerCompany);
  CHECK_TRUE("source record-link replication", !table.replicateData);
  CHECK_TEXT("source record-link inherent permissions", table.inherentPermissions, "rX");
  for (const auto &expected : fields) {
    const auto *declared = agiru::Field(table, agiru::FieldNo{expected.number});
    const agiru::FieldDef absent{};
    const auto &field = declared != nullptr ? *declared : absent;
    CHECK_TRUE("original record-link number exists", declared != nullptr);
    CHECK_TEXT("original record-link field name", field.name, expected.name);
    CHECK_TEXT("original record-link caption", field.caption, expected.name);
    CHECK_TRUE("original record-link field type", field.type == expected.type);
    CHECK_TRUE("original record-link field length", field.length == expected.length);
    CHECK_TRUE("only Link ID auto-increments", field.autoIncrement == (expected.number == 1));
    CHECK_TRUE("record-link fields are not invented FlowFields",
               field.fieldClass == agiru::FieldClass::Normal);
    const bool removed = expected.number >= 4 && expected.number <= 6;
    CHECK_TEXT("source record-link obsolescence", field.obsoleteState, removed ? "Removed" : "");
    CHECK_TEXT("source record-link obsolescence reason",
               field.obsoleteReason,
               removed ? "URL1 field size increased" : "");
  }
  const auto *note = agiru::Field(table, agiru::FieldNo{9});
  CHECK_TEXT("source note BLOB subtype", note != nullptr ? note->subtype : "", "Memo");
  const auto *company = agiru::Field(table, agiru::FieldNo{12});
  CHECK_TEXT("source company relation",
             company != nullptr ? company->relation : "",
             "System.Environment.Company.Name");
  CHECK_TRUE("all three source keys are retained", table.keys.size() == 3);
  constexpr std::array<std::array<int, 2>, 3> keys{{{1, 0}, {2, 0}, {12, 2}}};
  for (std::size_t i = 0; i < keys.size(); ++i) {
    const agiru::KeyDef absent{};
    const auto &key = i < table.keys.size() ? table.keys[i] : absent;
    CHECK_TEXT("source record-link key name", key.name, "Key" + std::to_string(i + 1));
    CHECK_TRUE("source record-link key arity", key.fields.size() == (i == 2 ? 2 : 1));
    CHECK_TRUE("source record-link key first field",
               !key.fields.empty() && key.fields[0] == agiru::FieldNo{keys[i][0]});
    CHECK_TRUE("source record-link company/record key order",
               i != 2 || (key.fields.size() == 2 && key.fields[1] == agiru::FieldNo{keys[i][1]}));
    CHECK_TRUE("only the source primary key is clustered", key.clustered == (i == 0));
  }
  agiru::platform::RecordLink link;
  link.UserID = "Creator.Mixed Case";
  link.ToUserID = "Recipient.Mixed Case";
  agiru::RecordRef reflected;
  reflected.GetTable(link);
  CHECK_TRUE("reflection reaches the source recipient field", reflected.FieldExist(14));
  for (const auto &system : agiru::kSystemFields) {
    CHECK_TRUE("reflection reaches each system field by its original number",
               reflected.FieldExist(system.no.Value()));
  }
  CHECK_TEXT(
      "source creator Text preserves case", reflected.Field(11).ToText(), "Creator.Mixed Case");
  CHECK_TEXT(
      "source recipient Text preserves case", reflected.Field(14).ToText(), "Recipient.Mixed Case");
  CHECK_TRUE("reflection sees all source record-link keys", reflected.KeyCount() == 3);
}

void TableMetadataDeclarationMatchesSystemSource() {
  const auto &table = agiru::TableTraits<agiru::platform::TableMetadata>::kTable;
  constexpr std::array<FieldSpec, 23> fields{{
      {.number = 1, .name = "ID", .type = agiru::FieldType::Integer},
      {.number = 2, .name = "Name", .type = agiru::FieldType::Text, .length = 30},
      {.number = 3, .name = "Caption", .type = agiru::FieldType::Text, .length = 80},
      {.number = 4, .name = "DataPerCompany", .type = agiru::FieldType::Boolean},
      {.number = 5, .name = "LookupPageID", .type = agiru::FieldType::Integer},
      {.number = 6, .name = "DrillDownPageId", .type = agiru::FieldType::Integer},
      {.number = 7, .name = "DataCaptionFields", .type = agiru::FieldType::Text, .length = 80},
      {.number = 8, .name = "PasteIsValid", .type = agiru::FieldType::Boolean},
      {.number = 9, .name = "LinkedObject", .type = agiru::FieldType::Boolean},
      {.number = 10, .name = "DataIsExternal", .type = agiru::FieldType::Boolean},
      {.number = 11, .name = "TableType", .type = agiru::FieldType::Option},
      {.number = 12, .name = "ExternalName", .type = agiru::FieldType::Text, .length = 248},
      {.number = 13, .name = "ObsoleteState", .type = agiru::FieldType::Option},
      {.number = 14, .name = "ObsoleteReason", .type = agiru::FieldType::Text, .length = 248},
      {.number = 15, .name = "DataClassification", .type = agiru::FieldType::Option},
      {.number = 16, .name = "ReplicateData", .type = agiru::FieldType::Boolean},
      {.number = 17, .name = "CompressionType", .type = agiru::FieldType::Option},
      {.number = 18, .name = "App ID", .type = agiru::FieldType::Guid},
      {.number = 19, .name = "InherentPermissions", .type = agiru::FieldType::Text, .length = 5},
      {.number = 20, .name = "InherentEntitlements", .type = agiru::FieldType::Text, .length = 5},
      {.number = 21, .name = "Scope", .type = agiru::FieldType::Option},
      {.number = 22, .name = "Access", .type = agiru::FieldType::Option},
      {.number = 23, .name = "AL Namespace", .type = agiru::FieldType::Text, .length = 500},
  }};
  CHECK_TRUE("all original Table Metadata fields", table.fields.size() == fields.size());
  CHECK_TRUE("original Table Metadata object ID", table.id.Value() == 2000000136);
  CHECK_TEXT("original Table Metadata AL name", table.name, "Table Metadata");
  CHECK_TEXT("original Table Metadata default caption", table.caption, "Table Metadata");
  CHECK_TRUE("Table Metadata is tenant-wide", !table.dataPerCompany);
  CHECK_TEXT("Table Metadata source inherent permissions", table.inherentPermissions, "rX");
  CHECK_TRUE("one effective default primary key", table.keys.size() == 1);
  const agiru::KeyDef absentKey{};
  const auto &key = table.keys.empty() ? absentKey : table.keys.front();
  CHECK_TEXT("default key name follows the shared declaration completion", key.name, "ID");
  CHECK_TRUE("default primary key is the first source field",
             key.fields.size() == 1 && key.fields.front() == agiru::FieldNo{1});
  CHECK_TRUE("effective primary key is clustered", key.clustered);
  for (const auto &expected : fields) {
    const auto *found = agiru::Field(table, agiru::FieldNo{expected.number});
    const agiru::FieldDef absent{};
    const auto &field = found != nullptr ? *found : absent;
    CHECK_TRUE("every original Table Metadata number", found != nullptr);
    CHECK_TEXT("original Table Metadata field name", field.name, expected.name);
    CHECK_TEXT("original Table Metadata field caption", field.caption, expected.name);
    CHECK_TRUE("original Table Metadata field type", field.type == expected.type);
    CHECK_TRUE("original Table Metadata field length", field.length == expected.length);
    CHECK_TRUE("Table Metadata has no invented FlowFields",
               field.fieldClass == agiru::FieldClass::Normal);
    CHECK_TRUE("Table Metadata has no invented auto-increment", !field.autoIncrement);
  }

  struct Options {
    int number;
    std::span<const std::string_view> names;
  };

  constexpr std::array<std::string_view, 7> kinds{
      "Normal", "CRM", "ExternalSQL", "Exchange", "MicrosoftGraph", "Query", "Temporary"};
  constexpr std::array<std::string_view, 3> obsolete{"No", "Pending", "Removed"};
  constexpr std::array<std::string_view, 7> classifiers{"CustomerContent",
                                                        "ToBeClassified",
                                                        "EndUserIdentifiableInformation",
                                                        "AccountData",
                                                        "EndUserPseudonymousIdentifiers",
                                                        "OrganizationIdentifiableInformation",
                                                        "SystemMetadata"};
  constexpr std::array<std::string_view, 4> compression{"Unspecified", "None", "Row", "Page"};
  constexpr std::array<std::string_view, 2> scope{"Cloud", "OnPrem"};
  constexpr std::array<std::string_view, 2> access{"Public", "Internal"};
  const std::array options{Options{11, kinds},
                           Options{13, obsolete},
                           Options{15, classifiers},
                           Options{17, compression},
                           Options{21, scope},
                           Options{22, access}};
  for (const auto &expected : options) {
    const auto *found = agiru::Field(table, agiru::FieldNo{expected.number});
    const agiru::FieldDef absent{};
    const auto &field = found != nullptr ? *found : absent;
    CHECK_TRUE("complete source Table Metadata option",
               field.values.size() == expected.names.size());
    for (std::size_t i = 0; i < expected.names.size(); ++i) {
      const agiru::EnumValueDef absentValue{.ordinal = -1, .name = {}, .caption = {}};
      const auto &value = i < field.values.size() ? field.values[i] : absentValue;
      CHECK_TRUE("Table Metadata option ordinal", value.ordinal == static_cast<int>(i));
      CHECK_TEXT("Table Metadata option name", value.name, expected.names[i]);
      CHECK_TEXT("Table Metadata option default caption", value.caption, expected.names[i]);
    }
  }
  agiru::platform::TableMetadata record;
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("Table Metadata reflection keeps all source fields", reflected.FieldCount() == 23);
  CHECK_TRUE("Table Metadata reflection keeps its effective key", reflected.KeyCount() == 1);
  for (const auto &expected : fields) {
    const bool exists = reflected.FieldExist(expected.number);
    CHECK_TRUE("Table Metadata reflected source number", exists);
    if (expected.type != agiru::FieldType::Text) { continue; }
    const auto *field = agiru::Field(table, agiru::FieldNo{expected.number});
    const bool typed = exists && field != nullptr && field->type == expected.type;
    const std::string value(static_cast<std::size_t>(expected.length), 'x');
    if (typed) {
      reflected.Field(expected.number).Value(agiru::Variant(value));
      reflected.SetTable(record);
    }
    CHECK_TRUE("Table Metadata full source Text length roundtrips",
               typed && record.FieldFormat(agiru::FieldNo{expected.number}) == value);
  }
}

}

int main() {
  return gate::Run("NativeObject", [] {
    DeclarationMatchesSystemSource();
    ReflectionUsesDeclaredNumbersAndTypes();
    NativeScopeMatchesSystemSource();
    AuthenticationEmailIsText();
    AllObjUsesItsOwnDeclaration();
    PrivacyDeclarationsMatchSystemSource();
    PrivacyReflectionAndCatalogueUseSourceIdentity();
    AllProfileDeclarationMatchesSystemSource();
    AllProfileReflectionRetainsIndependentSourceFields();
    RecordLinkDeclarationMatchesSystemSource();
    TableMetadataDeclarationMatchesSystemSource();
  });
}
