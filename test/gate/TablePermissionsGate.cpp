#include "meta/Ids.h"
#include "meta/QueryDef.h"
#include "meta/TableDef.h"
#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Query.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TablePermissions.h"
#include "type/Guid.h"

#include "Check.h"
#include "OwnedDatabase.h"

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kAdded = "00000000-0000-0000-0000-000000000002";
constexpr std::array kQueryItems{
    agiru::QueryDataItem{.name = "Accounts", .table = &agiru::platform::kUserTable}};
constexpr std::array kQueryColumns{
    agiru::QueryColumn{.name = "Name",
                       .offset = offsetof(agiru::platform::User, UserName),
                       .field = agiru::platform::User::Field_No::UserName}};
constexpr agiru::QueryDef kQuery{
    .name = "Fixture Accounts", .dataItems = kQueryItems, .columns = kQueryColumns};

class Authority final : public agiru::TablePermissionAuthority {
public:
  bool Allows(const agiru::TableDef &table, agiru::TableOperation operation) const override {
    const auto &session = agiru::Session::Current();
    const std::array<std::optional<std::string>, 4> binds{
        session.UserSecurityId().ToStorageText(),
        std::string(session.CompanyName()),
        std::to_string(table.id.Value()),
        std::to_string(static_cast<unsigned>(operation))};
    const auto rows = session.Database().Execute(
        "SELECT allowed FROM fixture_grants WHERE principal = $1::uuid AND company = $2 "
        "AND object = $3::integer AND operation = $4::integer",
        binds);
    return rows.Rows() == 1 && rows.Value(0, 0) == "t";
  }
};

void Seed(const std::string &dsn) {
  const agiru::Session seed(dsn);
  agiru::CreateTable(seed.Database(), agiru::platform::kUserTable);
  agiru::platform::User user;
  user.UserSecurityID = agiru::Guid(kUser);
  user.UserName = "ACCOUNT";
  user.Insert();
  seed.Database().Run(
      "CREATE TABLE fixture_grants(principal uuid,company text,object integer,"
      "operation integer,allowed boolean,PRIMARY KEY(principal,company,object,operation))");
  seed.Database().Run(
      "INSERT INTO fixture_grants SELECT '" + std::string(kUser) +
      "'::uuid,'Company',2000000120,operation,true FROM generate_series(0,3) operation");
  agiru::Commit();
}

void Refuses(std::string_view name,
             const std::function<void()> &operation,
             std::string_view code = "Permission") {
  bool refused = false;
  try {
    operation();
  } catch (const agiru::Error &error) { refused = error.Code() == code; }
  CHECK_TRUE(name, refused);
}

void Contracts() {
  const gate::OwnedDatabase database("table_permissions");
  Seed(database.Dsn());
  const agiru::Connection observer(database.Dsn());
  agiru::Session session(database.Dsn(), agiru::Guid(kUser));
  session.CompanyName("Company");
  agiru::platform::User row;
  Refuses(
      "authenticated ReadPermission does not default to SUPER",
      [&] { static_cast<void>(row.ReadPermission()); },
      "PermissionUnavailable");
  Refuses(
      "authenticated reads require explicit authority",
      [&] { static_cast<void>(row.Get(agiru::Guid(kUser))); },
      "PermissionUnavailable");
  session.TablePermissions(std::make_shared<Authority>());
  CHECK_TRUE("direct typed read/write getters resolve current SQL rights",
             row.ReadPermission() && row.WritePermission());
  agiru::RecordRef reflected;
  reflected.GetTable(row);
  CHECK_TRUE("reflected getters share typed authority",
             reflected.ReadPermission() && reflected.WritePermission());
  for (const auto operation : {agiru::TableOperation::Insert,
                               agiru::TableOperation::Modify,
                               agiru::TableOperation::Delete}) {
    observer.Run("UPDATE fixture_grants SET allowed = false WHERE operation = " +
                 std::to_string(static_cast<unsigned>(operation)));
    CHECK_TRUE("WritePermission requires every individual write kind",
               !row.WritePermission() && !reflected.WritePermission() && row.ReadPermission());
    row.UserSecurityID = agiru::Guid(kAdded);
    row.UserName = "ADDED";
    if (operation == agiru::TableOperation::Insert) {
      Refuses("denied insert refuses before SQL", [&] { row.Insert(); });
      reflected.GetTable(row);
      Refuses("reflected insert cannot bypass the authority", [&] { reflected.Insert(); });
    } else {
      row.Get(agiru::Guid(kUser));
      if (operation == agiru::TableOperation::Modify) {
        row.FullName = "DENIED";
        Refuses("denied modify refuses before SQL", [&] { row.Modify(); });
        reflected.GetTable(row);
        Refuses("reflected modify cannot bypass the authority", [&] { reflected.Modify(); });
      } else {
        Refuses("denied delete refuses before SQL", [&] { row.Delete(); });
        reflected.GetTable(row);
        Refuses("reflected delete cannot bypass the authority", [&] { reflected.Delete(); });
        Refuses("bulk delete cannot bypass the authority", [&] { row.DeleteAll(); });
      }
    }
    observer.Run("UPDATE fixture_grants SET allowed = true");
  }
  CHECK_TRUE(
      "independent SQL confirms all refused writes leave the original account intact",
      observer.Execute("SELECT count(*) FROM \"User\" WHERE \"Full Name\" = ''").Value(0, 0) ==
          "1");
  CHECK_TRUE("a readable cursor opens before revocation", row.FindSet());
  agiru::detail::QueryHandle query;
  CHECK_TRUE("a readable query opens before revocation",
             agiru::detail::QueryOpen(query.Ensure(), kQuery));
  observer.Run("UPDATE fixture_grants SET allowed = false WHERE operation = 0");
  CHECK_TRUE("read getters observe SQL revocation",
             !row.ReadPermission() && !reflected.ReadPermission());
  Refuses("Get refuses after read revocation",
          [&] { static_cast<void>(row.Get(agiru::Guid(kUser))); });
  Refuses("FindSet refuses after read revocation", [&] { static_cast<void>(row.FindSet()); });
  Refuses("reflected FindFirst cannot bypass the authority",
          [&] { static_cast<void>(reflected.FindFirst()); });
  Refuses("buffered Next rechecks permission before consuming rows",
          [&] { static_cast<void>(row.Next()); });
  Refuses("Count cannot disclose denied rows", [&] { static_cast<void>(row.Count()); });
  Refuses("IsEmpty cannot disclose denied presence", [&] { static_cast<void>(row.IsEmpty()); });
  Refuses("query construction requires all source-table rights", [&] {
    agiru::detail::QueryHandle denied;
    static_cast<void>(agiru::detail::QueryOpen(denied.Ensure(), kQuery));
  });
  Refuses("buffered Query.Read rechecks source-table rights",
          [&] { static_cast<void>(agiru::detail::QueryRead(query.Ensure(), kQuery, &row)); });
  agiru::detail::QueryClose(query.Ensure());
  agiru::Temporary<agiru::platform::User> temporary;
  temporary.UserSecurityID = agiru::Guid(kAdded);
  temporary.Insert();
  CHECK_TRUE("temporary data remains usable without physical table rights",
             temporary.Get(agiru::Guid(kAdded)));
  temporary.DeleteAll();
  CHECK_TRUE("temporary delete remains in memory", temporary.IsEmpty());
  session.CompanyName("Other");
  CHECK_TRUE("another company never inherits this company's grants", !row.ReadPermission());
}

}

int main() {
  return gate::Run("TablePermissions", Contracts);
}
