#include "meta/PageDef.h"
#include "platform/Company.h"
#include "platform/User.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageDispatcher.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/TablePermissions.h"
#include "type/Decimal.h"
#include "type/Guid.h"

#include "Check.h"
#include "NativePermissionFixture.h"
#include "OwnedDatabase.h"
#include "PrivateAuthFile.h"
#include "fixture/table/NavigationRow.h"
#include "fixture/table/RestrictedRow.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using Row = agiru::Fixture::NavigationRow_Table;
using Restricted = agiru::Fixture::RestrictedRow_Table;
constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kOtherUser = "00000000-0000-0000-0000-000000000002";
constexpr std::string_view kCompany = "Fixture + Company";
constexpr int kInitialValueStep = 11;
constexpr int kRestrictedValue = 999;
constexpr std::string_view kExactAmount = "0.12345678901234567890";
constexpr std::int64_t kExactInteger = 9223372036854775807LL;
constexpr auto kFixtureDialogTimeout = std::chrono::seconds(5);

void NativeGrants(const agiru::Connection &connection) {
  agiru::CreateTable(connection, agiru::platform::kCompanyTable);
  agiru::platform::Company company;
  company.Name = kCompany;
  company.Insert();
  gate::InstallNativePermissionFixture(connection);
  connection.Run(R"(INSERT INTO "Tenant Permission Set" VALUES
    ('10000000-0000-0000-0000-000000000001','EDITOR','Editor',true),
    ('10000000-0000-0000-0000-000000000001','READER','Reader',true))");
  connection.Run(R"(INSERT INTO "Access Control" VALUES
    ('00000000-0000-0000-0000-000000000001','EDITOR','Fixture + Company',1,
      '10000000-0000-0000-0000-000000000001'),
    ('00000000-0000-0000-0000-000000000002','READER','Fixture + Company',1,
      '10000000-0000-0000-0000-000000000001'))");
  connection.Run(R"(INSERT INTO "Tenant Permission" VALUES
    ('10000000-0000-0000-0000-000000000001','EDITOR',0,50340,1,1,1,1,0,'',0),
    ('10000000-0000-0000-0000-000000000001','READER',0,50340,1,0,0,0,0,'',0),
    ('10000000-0000-0000-0000-000000000001','EDITOR',8,50340,0,0,0,0,1,'',0),
    ('10000000-0000-0000-0000-000000000001','EDITOR',8,50341,0,0,0,0,1,'',0),
    ('10000000-0000-0000-0000-000000000001','EDITOR',8,50347,0,0,0,0,1,'',0),
    ('10000000-0000-0000-0000-000000000001','READER',8,50340,0,0,0,0,1,'',0),
    ('10000000-0000-0000-0000-000000000001','READER',8,50341,0,0,0,0,1,'',0))");
}

void Seed(const std::string &dsn, const std::string &authPath) {
  const agiru::Session seed(dsn);
  const auto &connection = seed.Database();
  agiru::CreateTable(connection, agiru::TableTraits<agiru::platform::User>::kTable);
  agiru::CreateTable(connection, agiru::TableTraits<Row>::kTable);
  agiru::CreateTable(connection, agiru::TableTraits<Restricted>::kTable);
  NativeGrants(connection);
  Restricted restricted;
  restricted.ID = 1;
  restricted.Value = kRestrictedValue;
  restricted.Insert();
  for (const auto identity : {kUser, kOtherUser}) {
    agiru::platform::User user;
    user.UserSecurityID = agiru::Guid(identity);
    user.UserName = identity == kUser ? "FIRST USER" : "SECOND USER";
    user.Insert();
  }
  for (int identity = 1; identity <= 2; ++identity) {
    Row row;
    row.ID = identity;
    row.Value = identity * kInitialValueStep;
    row.Label = "Grüezi <script> 東京 🔧";
    row.Amount = agiru::Decimal::FromInvariantString(kExactAmount);
    row.Exact = kExactInteger;
    row.Code = identity == 1 ? "0001" : "20";
    row.Insert();
  }
  connection.Run("CREATE TABLE ui_grants(user_security_id uuid,company text,readable boolean,"
                 "writable boolean)");
  connection.Run("INSERT INTO ui_grants VALUES "
                 "('00000000-0000-0000-0000-000000000001','Fixture + Company',true,true),"
                 "('00000000-0000-0000-0000-000000000002','Fixture + Company',true,false)");
  connection.Run("CREATE TABLE ui_writes(value integer,user_security_id uuid)");
  connection.Run(R"(CREATE FUNCTION ui_write_audit() RETURNS trigger LANGUAGE plpgsql AS $body$
    BEGIN INSERT INTO ui_writes VALUES(NEW."Value",NEW."SystemModifiedBy"); RETURN NEW; END
    $body$)");
  connection.Run(R"(CREATE TRIGGER ui_write_audit AFTER UPDATE ON "Navigation Row"
                    FOR EACH ROW EXECUTE FUNCTION ui_write_audit())");
  agiru::InstallClientCredentials(connection);
  agiru::InstallPageCommandHost(connection);
  gate::PrivateAuthFile(
      authPath,
      agiru::IssueClientCredential(connection, agiru::Guid(kUser), std::chrono::hours(1)));
  gate::PrivateAuthFile(
      authPath + ".second",
      agiru::IssueClientCredential(connection, agiru::Guid(kOtherUser), std::chrono::hours(1)));
  agiru::Commit();
  const auto name = connection.Execute("SELECT current_database()");
  const auto value = name.Value(0, 0);
  if (!value) { throw std::runtime_error("owned database did not return its identity"); }
  std::fputs("DATABASE ", stdout);
  std::fputs(std::string(*value).c_str(), stdout);
  std::fputc('\n', stdout);
}

void Authorize(const agiru::PageDef &page,
               agiru::PageHostOperation operation,
               const agiru::PageControlCommand &command) {
  if (page.source != agiru::TableTraits<Row>::kTable.id) {
    throw agiru::Error("fixture source access denied", "PageHostPermission");
  }
  const auto &session = agiru::Session::Current();
  const std::array<std::optional<std::string>, 2> binds{session.UserSecurityId().ToStorageText(),
                                                        std::string(session.CompanyName())};
  const auto rows = session.Database().Execute(
      "SELECT readable,writable FROM ui_grants WHERE user_security_id = $1::uuid "
      "AND company = $2",
      binds);
  const bool writes = operation == agiru::PageHostOperation::OpenEdit ||
                      operation == agiru::PageHostOperation::OpenNew ||
                      operation == agiru::PageHostOperation::Save ||
                      operation == agiru::PageHostOperation::Close ||
                      command.operation == agiru::PageControlOperation::Set ||
                      command.operation == agiru::PageControlOperation::Action;
  if (rows.Rows() != 1 || rows.Value(0, 0) != "t" || (writes && rows.Value(0, 1) != "t")) {
    throw agiru::Error("fixture SQL grant revoked", "PageHostPermission");
  }
}

class TableAuthority final : public agiru::TablePermissionAuthority {
public:
  bool Allows(const agiru::TableDef &table, agiru::TableOperation operation) const override {
    if (table.id != agiru::TableTraits<Row>::kTable.id) { return false; }
    const auto &session = agiru::Session::Current();
    const std::array<std::optional<std::string>, 2> binds{session.UserSecurityId().ToStorageText(),
                                                          std::string(session.CompanyName())};
    const auto rows = session.Database().Execute(
        "SELECT readable,writable FROM ui_grants WHERE user_security_id = $1::uuid "
        "AND company = $2",
        binds);
    return rows.Rows() == 1 && rows.Value(0, 0) == "t" &&
           (operation == agiru::TableOperation::Read || rows.Value(0, 1) == "t");
  }
};

void Serve(const std::string &authPath, const std::string &origin, bool seedOnly) {
  const gate::OwnedDatabase database("page_host", gate::OwnedDatabase::Encoding::Utf8);
  Seed(database.Dsn(), authPath);
  if (seedOnly) {
    std::fputs("READY\n", stdout);
    std::fflush(stdout);
    while (true) {
      const int command = std::getchar();
      if (command == 'Q' || command == EOF) { return; }
    }
  }
  agiru::PageCommandHost host({.database = database.Dsn(),
                               .company = std::string(kCompany),
                               .origin = origin,
                               .dialogTimeout = kFixtureDialogTimeout},
                              Authorize,
                              std::make_shared<TableAuthority>());
  const agiru::HttpServer server([&](const auto &request) { return host.Handle(request); },
                                 {.workers = 1});
  std::fputs("READY\n", stdout);
  std::fflush(stdout);
  while (true) {
    const int command = std::getchar();
    if (command == 'Q' || command == EOF) { break; }
  }
}

}

int main(int argc, char **argv) {
  return gate::Run("Generated Page HTTP Host", [=] {
    if (argc != 3 && (argc != 4 || std::string_view(argv[3]) != "--seed-only")) {
      throw std::runtime_error("expected private auth path and external origin");
    }
    Serve(argv[1], argv[2], argc == 4);
  });
}
