#include "meta/ModuleDef.h"
#include "meta/PermissionSetDef.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/User.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/NativePermissions.h"
#include "runtime/PermissionSetRegistry.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Guid.h"
#include "type/TableFilter.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "PlatformModule.h"
#include "core/codeunit/StoredRecordAccess.h"
#include "system/security/access_control/table/AccessControl.h"
#include "system/security/access_control/table/TenantPermission.h"
#include "system/security/access_control/table/TenantPermissionSet.h"
#include "system/security/access_control/table/TenantPermissionSetRel.h"
#include "system/text/table/EntityText.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace original = agiru::System::Security::AccessControl;
using Sets = original::TenantPermissionSet_Table;
using Permissions = original::TenantPermission_Table;
using Relations = original::TenantPermissionSetRel_Table;
using Assignments = original::AccessControl_Table;
using EntityText = agiru::System::Text::EntityText_Table;
constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::int32_t kUserTable = 2000000120;
constexpr std::int32_t kPage = 21;
constexpr std::string_view kBinaryProbe{"\0\xff\x80"
                                        "A",
                                        4};

std::string_view Cell(const agiru::Result &result, std::size_t column) {
  if (result.Rows() != 1 || column >= result.Columns()) {
    throw agiru::Error("qualification SQL returned an unexpected population", "NativeStorageTest");
  }
  const auto value = result.Value(0, column);
  if (!value.has_value()) {
    throw agiru::Error("qualification SQL returned NULL", "NativeStorageTest");
  }
  return *value;
}

void Declarations() {
  constexpr std::array declarations{&agiru::TableTraits<Sets>::kTable,
                                    &agiru::TableTraits<Permissions>::kTable,
                                    &agiru::TableTraits<Relations>::kTable,
                                    &agiru::TableTraits<Assignments>::kTable};
  for (const auto *table : declarations) {
    CHECK_TRUE("original permission tables retain shared storage", !table->dataPerCompany);
    CHECK_TRUE("original permission tables retain replication policy", !table->replicateData);
    CHECK_TEXT("original AL namespace retained", table->nameSpace, "System.Security.AccessControl");
    CHECK_TEXT("original System module retained",
               table->module->id,
               "8874ed3a-0643-4247-9ced-7a7002f7135d");
    CHECK_TEXT("Cloud availability is not a product exclusion", table->scope, "Cloud");
    CHECK_TRUE("generated source is installed in the one runtime catalogue",
               agiru::FindTable(table->id) != nullptr &&
                   agiru::FindTable(table->id)->table == table);
  }
}

std::string Diagnostic(auto &&operation) {
  try {
    operation();
  } catch (const agiru::Error &error) {
    CHECK_TEXT("TableFilter checks preserve TestField error identity", error.Code(), "TestField");
    return error.what();
  }
  return {};
}

void FilterChecks() {
  constexpr std::string_view kExpression = "Unicode Ω|雪 & <>";
  const std::string owned = agiru::TableFilter{kExpression}.ToText();
  CHECK_TEXT("filter diagnostics own the original Unicode expression", owned, kExpression);
  Permissions record;
  CHECK_SILENT("equal empty TableFilter fields pass TestField",
               Diagnostic([&] { record.TestField(record.SecurityFilter, agiru::TableFilter{}); }));
  const auto blank = Diagnostic([&] { record.TestField(record.SecurityFilter); });
  CHECK_TRUE("empty TableFilter uses the ordinary missing-value diagnostic",
             blank.starts_with("Security Filter must have a value") &&
                 blank.ends_with("It cannot be zero or empty."));
  record.SecurityFilter = agiru::TableFilter{kExpression};
  CHECK_SILENT("nonempty TableFilter passes the single-argument TestField",
               Diagnostic([&] { record.TestField(record.SecurityFilter); }));
  CHECK_SILENT("equal nonempty TableFilter fields pass TestField", Diagnostic([&] {
                 record.TestField(record.SecurityFilter, agiru::TableFilter{kExpression});
               }));
  const auto mismatch = Diagnostic(
      [&] { record.TestField(record.SecurityFilter, agiru::TableFilter{"Name=OTHER"}); });
  CHECK_TRUE("mismatched filters retain both exact expressions in the ordinary diagnostic",
             mismatch.starts_with("Security Filter must be equal to 'Name=OTHER'") &&
                 mismatch.ends_with("Current value is 'Unicode Ω|雪 & <>'."));
  record.Init();
  CHECK_SILENT("Init restores empty equality rather than parsing a security predicate",
               Diagnostic([&] { record.TestField(record.SecurityFilter, agiru::TableFilter{}); }));
}

void Seed(const std::string &dsn) {
  const agiru::Session session(dsn);
  const auto &db = session.Database();
  agiru::CreateTable(db, agiru::platform::kUserTable);
  for (const auto *table : {&agiru::TableTraits<Sets>::kTable,
                            &agiru::TableTraits<Permissions>::kTable,
                            &agiru::TableTraits<Relations>::kTable,
                            &agiru::TableTraits<Assignments>::kTable}) {
    agiru::CreateTable(db, *table);
  }
  agiru::platform::User user;
  user.UserSecurityID = agiru::Guid(kUser);
  user.UserName = "STORAGE TEST";
  user.Insert();
  Sets root;
  CHECK_TRUE("original Assignable InitValue is true", static_cast<bool>(root.Assignable));
  root.AppID = agiru::Guid(agiru::app::Platform::kModule.id);
  root.RoleID = "ROOT";
  root.Name = "Qualification";
  root.Insert();
  Sets leaf;
  leaf.AppID = root.AppID;
  leaf.RoleID = "LEAF";
  leaf.Assignable = false;
  leaf.Insert();
  Permissions permission;
  CHECK_TRUE("original permissions initialize R/I/M/D/X to Yes",
             permission.ReadPermission_7.AsInteger() == 1 &&
                 permission.InsertPermission.AsInteger() == 1 &&
                 permission.ModifyPermission.AsInteger() == 1 &&
                 permission.DeletePermission.AsInteger() == 1 &&
                 permission.ExecutePermission.AsInteger() == 1);
  permission.AppID = root.AppID;
  permission.RoleID = "LEAF";
  permission.ObjectID = kUserTable;
  permission.ReadPermission_7 = 1;
  permission.InsertPermission = 0;
  permission.ModifyPermission = 2;
  permission.DeletePermission = 0;
  permission.ExecutePermission = 0;
  permission.SecurityFilter = agiru::TableFilter{"Unicode Ω|雪 & <>"};
  permission.Insert();
  Permissions stored;
  CHECK_TRUE("generated permission records read back their original SQL key",
             static_cast<bool>(stored.Get(root.AppID, "LEAF", 0, kUserTable)));
  CHECK_TEXT("security filter storage retains exact Unicode expression",
             stored.SecurityFilter.Value(),
             "Unicode Ω|雪 & <>");
  stored.Init();
  CHECK_TRUE("Init clears a security filter without a value coercion",
             stored.SecurityFilter.IsEmpty());
  stored.ReadPermission_7 = 1;
  stored.InsertPermission = 0;
  stored.ModifyPermission = 2;
  stored.DeletePermission = 0;
  stored.ExecutePermission = 0;
  stored.Modify();
  permission.Init();
  permission.AppID = root.AppID;
  permission.RoleID = "LEAF";
  permission.ObjectType = static_cast<std::int32_t>(agiru::PermissionObject::Page);
  permission.ObjectID = kPage;
  permission.Insert();
  Relations relation;
  relation.AppID = root.AppID;
  relation.RoleID = "ROOT";
  relation.RelatedAppID = root.AppID;
  relation.RelatedRoleID = "LEAF";
  relation.RelatedScope = 1;
  CHECK_TRUE("original relations default to Include", relation.Type.AsInteger() == 0);
  relation.Insert();
  Assignments assignment;
  assignment.UserSecurityID = user.UserSecurityID;
  assignment.RoleID = "ROOT";
  assignment.CompanyName = "Qualification";
  assignment.Scope = 1;
  assignment.AppID = root.AppID;
  assignment.Insert();
  agiru::Commit();
}

void Sql(const agiru::Connection &observer) {
  const auto widths = observer.Execute(R"(SELECT column_name,character_maximum_length::text
FROM information_schema.columns WHERE table_name='Tenant Permission Set' AND column_name='Role ID')");
  CHECK_TRUE("one original Role ID SQL column exists", widths.Rows() == 1);
  CHECK_TEXT("SQL storage retains original Code[20] width", Cell(widths, 1), "20");
  const auto flow = observer.Execute(R"(SELECT count(*)::text FROM information_schema.columns
WHERE table_name IN ('Tenant Permission','Access Control')
AND column_name IN ('Role Name','Object Name','User Name','App Name'))");
  CHECK_TEXT("FlowFields do not become physical permission columns", Cell(flow, 0), "0");
  const auto rights = observer.Execute(R"(SELECT "Object Type"::text,"Read Permission"::text,
"Insert Permission"::text,"Modify Permission"::text,"Delete Permission"::text,
"Execute Permission"::text,"Security Filter", "Type"::text
FROM "Tenant Permission" WHERE "Object ID"=2000000120)");
  constexpr std::array expected{"0", "1", "0", "2", "0", "0", "", "0"};
  CHECK_TRUE("one original TableData permission row committed", rights.Rows() == 1);
  for (std::size_t index = 0; index < expected.size(); ++index) {
    CHECK_TEXT("independent SQL preserves exact original typed permissions",
               Cell(rights, index),
               expected[index]);
  }
  const auto audit = observer.Execute(R"(SELECT count(*)::text FROM "Tenant Permission"
WHERE "timestamp">0 AND "SystemId"<>'00000000-0000-0000-0000-000000000000'::uuid)");
  CHECK_TEXT("ordinary runtime owns permission rowversions and system IDs", Cell(audit, 0), "2");
}

void Authority(const std::string &dsn, const agiru::Connection &observer) {
  agiru::Session session(dsn, agiru::Guid(kUser));
  session.CompanyName("Qualification");
  const agiru::InstalledPermissionSets installed;
  const auto authority = std::make_shared<agiru::NativePermissions>(installed);
  using Operation = agiru::PermissionOperation;
  using Level = agiru::PermissionLevel;
  const auto right = [&](Operation operation) {
    return authority->Level(kUserTable, agiru::PermissionObject::TableData, operation);
  };
  CHECK_TRUE("generated SQL include graph grants read", right(Operation::Read) == Level::Direct);
  CHECK_TRUE("generated SQL preserves indirect modify",
             right(Operation::Modify) == Level::Indirect);
  CHECK_TRUE("generated SQL denies insert", right(Operation::Insert) == Level::None);
  CHECK_TRUE("original Page ordinal grants execute independently",
             authority->Level(kPage, agiru::PermissionObject::Page, Operation::Execute) ==
                 Level::Direct);
  session.TablePermissions(authority);
  agiru::platform::User user;
  CHECK_TRUE("ordinary records honor generated native read authority",
             static_cast<bool>(user.FindFirst()));
  bool denied = false;
  try {
    user.UserSecurityID = agiru::Guid("00000000-0000-0000-0000-000000000002");
    user.Insert();
  } catch (const agiru::Error &error) { denied = error.Code() == "Permission"; }
  CHECK_TRUE("generated authority prevents an actual native SQL write", denied);
  const auto users = observer.Execute("SELECT count(*)::text FROM \"User\"");
  CHECK_TEXT("denied write has no independent SQL effect", Cell(users, 0), "1");
  session.CompanyName("Other");
  CHECK_TRUE("shared storage does not leak company grants", right(Operation::Read) == Level::None);
  session.CompanyName("Qualification");
  observer.Run(R"(UPDATE "Tenant Permission" SET "Security Filter"='Name=ACCOUNT'
WHERE "Object ID"=2000000120)");
  bool filtered = false;
  try {
    static_cast<void>(right(Operation::Read));
  } catch (const agiru::Error &error) { filtered = error.Code() == "PermissionFilterUnsupported"; }
  CHECK_TRUE("stored filters refuse until row security is implemented", filtered);
  observer.Run(R"(UPDATE "Tenant Permission" SET "Security Filter"='')");
  observer.Run(R"(DELETE FROM "Access Control")");
  CHECK_TRUE("grant revocation is read from generated SQL storage",
             right(Operation::Read) == Level::None);
}

void EntitySeed(const std::string &dsn, const agiru::Connection &observer) {
  const auto &table = agiru::TableTraits<EntityText>::kTable;
  CHECK_TRUE("original Entity Text retains company-local normal storage",
             table.dataPerCompany && table.tableType == agiru::TableType::Normal);
  CHECK_TRUE("original Entity Text is not replicated", !table.replicateData);
  CHECK_TEXT("Entity Text keeps its System module",
             table.module->id,
             "8874ed3a-0643-4247-9ced-7a7002f7135d");
  CHECK_TEXT("Entity Text keeps its AL namespace", table.nameSpace, "System.Text");
  const agiru::Session seed(dsn);
  agiru::CreateTable(seed.Database(), table);
  EntityText text;
  text.Company = "Qualification";
  text.SourceTableId = kUserTable;
  text.SourceSystemId = agiru::Guid(kUser);
  text.PreviewText = "Unicode Ω雪";
  text.Text.Set(std::vector<std::uint8_t>{kBinaryProbe.begin(), kBinaryProbe.end()});
  text.Insert();
  Permissions permission;
  permission.AppID = agiru::Guid(agiru::app::Platform::kModule.id);
  permission.RoleID = "LEAF";
  permission.ObjectID = table.id.Value();
  permission.Insert();
  agiru::Commit();
  const auto width = observer.Execute(R"(SELECT character_maximum_length::text
FROM information_schema.columns WHERE table_name='Entity Text' AND column_name='Preview Text')");
  CHECK_TEXT("Entity Text SQL preserves original Text[1024] width", Cell(width, 0), "1024");
  const auto stored = observer.Execute(R"(SELECT "Company","Source Table Id"::text,
"Source System Id"::text,"Scenario"::text,"Preview Text",encode("Text",'hex') FROM "Entity Text")");
  constexpr std::array<std::string_view, 6> expected{
      "Qualification", "2000000120", kUser, "0", "Unicode Ω雪", "00ff8041"};
  for (std::size_t index = 0; index < expected.size(); ++index) {
    CHECK_TEXT("independent SQL retains the Entity Text composite key and exact content",
               Cell(stored, index),
               expected[index]);
  }
}

void EntityAuthority(const std::string &dsn, const agiru::Connection &observer) {
  agiru::Session session(dsn, agiru::Guid(kUser));
  session.CompanyName("Qualification");
  const agiru::InstalledPermissionSets installed;
  session.TablePermissions(std::make_shared<agiru::NativePermissions>(installed));
  EntityText text;
  agiru::StoredRecordAccess_Codeunit caller;
  CHECK_TRUE("stored Entity Text ReadPermission uses the original direct SQL grant",
             text.ReadPermission() && text.WritePermission() && caller.Called() &&
                 caller.Property());
  CHECK_TRUE("Entity Text reads its original composite key through ordinary records",
             text.Get("Qualification", kUserTable, agiru::Guid(kUser), text.Scenario));
  CHECK_TEXT(
      "Entity Text typed readback preserves Unicode", text.PreviewText.Value(), "Unicode Ω雪");
  text.CalcFields(text.Text);
  CHECK_TRUE("Entity Text typed BLOB readback preserves binary bytes",
             text.Text.Bytes() ==
                 std::vector<std::uint8_t>(kBinaryProbe.begin(), kBinaryProbe.end()));
  const std::string before(
      Cell(observer.Execute(R"(SELECT row_to_json(t)::text FROM "Entity Text" t)"), 0));
  observer.Run(R"(UPDATE "Tenant Permission" SET "Read Permission"=0,"Modify Permission"=0
WHERE "Object ID"=2000000132)");
  CHECK_TRUE("revoked Entity Text permission getters never return constant success",
             !text.ReadPermission() && !text.WritePermission() && !caller.Called() &&
                 !caller.Property());
  for (const bool writing : {false, true}) {
    bool denied = false;
    try {
      if (writing) {
        text.PreviewText = "DENIED";
        text.Modify();
      } else {
        static_cast<void>(text.Get("Qualification", kUserTable, agiru::Guid(kUser), text.Scenario));
      }
    } catch (const agiru::Error &error) { denied = error.Code() == "Permission"; }
    CHECK_TRUE("revoked Entity Text reads and modifications refuse", denied);
  }
  CHECK_TEXT("Entity Text denial preserves the full SQL row, audit and rowversion",
             Cell(observer.Execute(R"(SELECT row_to_json(t)::text FROM "Entity Text" t)"), 0),
             before);
  observer.Run(R"(UPDATE "Tenant Permission" SET "Read Permission"=1,"Modify Permission"=1
WHERE "Object ID"=2000000132)");
  session.CompanyName("Other");
  CHECK_TRUE("Entity Text never borrows another company's permission assignment",
             !text.ReadPermission());
  session.CompanyName("Qualification");
  text.Get("Qualification", kUserTable, agiru::Guid(kUser), text.Scenario);
  text.PreviewText = "Edited 雪";
  text.Modify();
  agiru::Commit();
  CHECK_TEXT("authorized Entity Text modification has an independent SQL effect",
             Cell(observer.Execute(R"(SELECT "Preview Text" FROM "Entity Text")"), 0),
             "Edited 雪");
  text.Delete();
  agiru::Commit();
  CHECK_TEXT("authorized Entity Text deletion has an independent SQL effect",
             Cell(observer.Execute(R"(SELECT count(*)::text FROM "Entity Text")"), 0),
             "0");
}

}

int main() {
  return gate::Run("NativeStorage", [] {
    Declarations();
    FilterChecks();
    const gate::OwnedDatabase database("native_storage");
    Seed(database.Dsn());
    const agiru::Connection observer(database.Dsn());
    Sql(observer);
    EntitySeed(database.Dsn(), observer);
    EntityAuthority(database.Dsn(), observer);
    Authority(database.Dsn(), observer);
  });
}
