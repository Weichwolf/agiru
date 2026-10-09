#include "meta/ModuleDef.h"
#include "meta/PermissionSetDef.h"
#include "meta/TableDef.h"
#include "platform/User.h"
#include "runtime/ClientCredentials.h"
#include "runtime/ConnectionInfo.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/NativePermissions.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PermissionSetRegistry.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Guid.h"

#include "Check.h"
#include "PlatformModule.h"
#include "PrivateAuthFile.h"
#include "SystemModule.h"
#include "system/ai/codeunit/CopilotCapability.h"
#include "system/ai/enum/CopilotCapability.h"
#include "system/ai/table/CopilotSettings.h"
#include "system/security/access_control/table/AccessControl.h"
#include "system/security/access_control/table/TenantPermission.h"
#include "system/security/access_control/table/TenantPermissionSet.h"
#include "system/security/access_control/table/TenantPermissionSetRel.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

namespace original = agiru::System::Security::AccessControl;
using Permissions = original::TenantPermission_Table;
constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kDeniedUser = "00000000-0000-0000-0000-000000000002";
constexpr std::string_view kRoleApp = "1ca2b971-4f9d-4322-a9d9-5ac8643ece90";
constexpr std::string_view kRole = "ERP QUALIFY";
constexpr std::int32_t kCustomerTable = 18;
constexpr std::int32_t kCustomerList = 22;
constexpr std::array kKinds{agiru::PermissionObject::TableData,
                            agiru::PermissionObject::Table,
                            agiru::PermissionObject::Report,
                            agiru::PermissionObject::Codeunit,
                            agiru::PermissionObject::XmlPort,
                            agiru::PermissionObject::MenuSuite,
                            agiru::PermissionObject::Page,
                            agiru::PermissionObject::Query,
                            agiru::PermissionObject::System};

std::string_view Cell(const agiru::Result &result, std::size_t column) {
  if (result.Rows() != 1 || column >= result.Columns()) {
    throw std::runtime_error("ERP preparation requires one complete SQL row");
  }
  const auto value = result.Value(0, column);
  if (!value.has_value()) { throw std::runtime_error("ERP preparation SQL returned NULL"); }
  return *value;
}

void Schema(const agiru::Connection &connection) {
  for (const auto *table : {&agiru::TableTraits<original::AccessControl_Table>::kTable,
                            &agiru::TableTraits<original::TenantPermissionSet_Table>::kTable,
                            &agiru::TableTraits<Permissions>::kTable,
                            &agiru::TableTraits<original::TenantPermissionSetRel_Table>::kTable}) {
    CHECK_TRUE("permission storage has its original System owner",
               table->module != nullptr && table->module->id == agiru::app::Platform::kModule.id);
    agiru::ProvisionTable(connection, *table);
  }
  const auto &registry = agiru::TableTraits<agiru::System::AI::CopilotSettings_Table>::kTable;
  CHECK_TRUE("capability registry storage retains the original System Application owner",
             registry.module != nullptr && registry.module->id == agiru::app::System::kModule.id);
  CHECK_TRUE("capability registry retains shared-company storage and its composite key",
             !registry.dataPerCompany && !registry.replicateData && registry.keys.size() == 1 &&
                 registry.keys.front().fields.size() == 2);
  agiru::ProvisionTable(connection, registry);
}

void RegistryReadback(const agiru::Connection &connection) {
  using Capability = agiru::System::AI::CopilotCapability_Enum;
  agiru::System::AI::CopilotSettings_Table settings;
  CHECK_TRUE("the transferred capability registry has an original source row",
             settings.FindFirst());
  if (gate::failures != 0) { throw std::runtime_error("registry source readback refused"); }
  agiru::System::AI::CopilotCapability_Codeunit registry;
  CHECK_TRUE("original registry query finds the exact transferred capability and app",
             registry.IsCapabilityRegistered(settings.Capability, settings.AppId));
  CHECK_TRUE("a capability row belonging to another app is not registered for System Application",
             settings.AppId != agiru::Guid(agiru::app::System::kModule.id) &&
                 !registry.IsCapabilityRegistered(settings.Capability));
  CHECK_TRUE("Entity Text is unregistered according to transferred rows, not a provider constant",
             !registry.IsCapabilityRegistered(Capability::EntityText));
  CHECK_TRUE("matching the transferred app does not ignore a different capability",
             !registry.IsCapabilityRegistered(Capability::EntityText, settings.AppId));
  CHECK_TRUE("repeated registry queries replace both filters without stale state",
             registry.IsCapabilityRegistered(settings.Capability, settings.AppId));
  const auto count = connection.Execute(R"(SELECT count(*)::text FROM "Copilot Settings")");
  CHECK_TEXT("registry reads do not insert, delete or register a capability", Cell(count, 0), "1");
}

void Grant(agiru::PermissionObject kind) {
  Permissions permission;
  permission.AppID = agiru::Guid(kRoleApp);
  permission.RoleID = kRole;
  permission.ObjectType = static_cast<std::int32_t>(kind);
  permission.ObjectID = 0;
  const bool data = kind == agiru::PermissionObject::TableData;
  permission.ReadPermission_7 = data ? 1 : 0;
  permission.InsertPermission = data ? 1 : 0;
  permission.ModifyPermission = data ? 1 : 0;
  permission.DeletePermission = data ? 1 : 0;
  permission.ExecutePermission = data ? 0 : 1;
  permission.Insert();
}

void Prepare(const agiru::Session &session, std::string_view company, const std::string &auth) {
  const auto &connection = session.Database();
  const auto rows = connection.Execute(R"(SELECT "Name","Id"::text FROM "Company")");
  CHECK_TEXT("only the original transferred company is used", Cell(rows, 0), company);
  CHECK_TRUE("original company identity is not replaced by a blank GUID",
             !agiru::Guid(Cell(rows, 1)).IsNull());
  const auto users = connection.Execute(R"(SELECT count(*)::text FROM "User")");
  CHECK_TEXT("source has no imported users; fixture provisioning is explicit", Cell(users, 0), "0");
  if (gate::failures != 0) {
    throw std::runtime_error("ERP fixture preparation refused its inputs");
  }
  for (const auto identity : {kUser, kDeniedUser}) {
    agiru::platform::User user;
    user.UserSecurityID = agiru::Guid(identity);
    user.UserName = identity == kUser ? "AGIRU ERP QUALIFIER" : "AGIRU ERP DENIED";
    user.Insert();
  }
  original::TenantPermissionSet_Table role;
  role.AppID = agiru::Guid(kRoleApp);
  role.RoleID = kRole;
  role.Name = "ERP workflow fixture";
  CHECK_TRUE("the explicit tenant fixture role is assignable", static_cast<bool>(role.Assignable));
  role.Insert();
  for (const auto kind : kKinds) { Grant(kind); }
  original::AccessControl_Table assignment;
  assignment.UserSecurityID = agiru::Guid(kUser);
  assignment.RoleID = kRole;
  assignment.CompanyName = company;
  assignment.AppID = role.AppID;
  assignment.Scope = static_cast<std::int32_t>(agiru::PermissionScope::Tenant);
  assignment.Insert();
  agiru::InstallClientCredentials(connection);
  agiru::InstallPageCommandHost(connection);
  gate::PrivateAuthFile(
      auth, agiru::IssueClientCredential(connection, agiru::Guid(kUser), std::chrono::hours(1)));
  gate::PrivateAuthFile(
      auth + ".denied",
      agiru::IssueClientCredential(connection, agiru::Guid(kDeniedUser), std::chrono::hours(1)));
  agiru::Commit();
}

void Authority(const std::string &dsn, std::string_view company) {
  using Operation = agiru::PermissionOperation;
  using Level = agiru::PermissionLevel;
  const agiru::InstalledPermissionSets installed;
  const auto authority = std::make_shared<agiru::NativePermissions>(installed);
  for (const auto identity : {kUser, kDeniedUser}) {
    agiru::Session session(dsn, agiru::Guid(identity));
    session.CompanyName(company);
    session.TablePermissions(authority);
    if (identity == kUser) { RegistryReadback(session.Database()); }
    const auto expected = identity == kUser ? Level::Direct : Level::None;
    for (const auto operation :
         {Operation::Read, Operation::Insert, Operation::Modify, Operation::Delete}) {
      CHECK_TRUE("fixture assignment controls original Customer TableData operations",
                 authority->Level(kCustomerTable, agiru::PermissionObject::TableData, operation) ==
                     expected);
    }
    CHECK_TRUE("fixture assignment controls original Customer List execution",
               authority->Level(kCustomerList, agiru::PermissionObject::Page, Operation::Execute) ==
                   expected);
    session.CompanyName("Unassigned fixture company");
    CHECK_TRUE("the fixture assignment does not grant another company",
               authority->Level(kCustomerTable,
                                agiru::PermissionObject::TableData,
                                Operation::Read) == Level::None);
    session.CompanyName(company);
    if (identity == kDeniedUser) {
      bool refused = false;
      try {
        agiru::platform::User user;
        user.UserSecurityID = agiru::Guid("00000000-0000-0000-0000-000000000003");
        user.Insert();
      } catch (const agiru::Error &error) { refused = error.Code() == "Permission"; }
      CHECK_TRUE("unassigned authenticated user cannot write through the AL boundary", refused);
    }
  }
  const agiru::Connection observer(dsn);
  const auto users = observer.Execute(R"(SELECT count(*)::text FROM "User")");
  CHECK_TEXT("denied AL write has no independently observed SQL effect", Cell(users, 0), "2");
}

}

int main(int argc, char **argv) {
  return gate::Run("ERP preparation", [&] {
    constexpr int kSchemaArguments = 3;
    constexpr int kPrepareArguments = 4;
    if (argc != kSchemaArguments && argc != kPrepareArguments) {
      throw std::runtime_error(
          "ERP preparation requires an owned DSN and --schema or company/auth");
    }
    const agiru::ConnectionInfo info(argv[1]);
    if (!info.Database().starts_with("agiru_erp_gate_")) {
      throw std::runtime_error("ERP preparation refuses a non-fixture database");
    }
    {
      const agiru::Session session(argv[1]);
      Schema(session.Database());
      if (argc == kSchemaArguments && std::string_view(argv[2]) == "--schema") {
        agiru::Commit();
      } else if (argc == kPrepareArguments) {
        Prepare(session, argv[2], argv[3]);
      } else {
        throw std::runtime_error("unknown ERP preparation operation");
      }
    }
    if (argc == kPrepareArguments) { Authority(argv[1], argv[2]); }
  });
}
