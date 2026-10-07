#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "meta/PermissionSetDef.h"
#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/NativePermissions.h"
#include "runtime/PermissionSets.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/TablePermissions.h"
#include "type/Guid.h"

#include "Check.h"
#include "OwnedDatabase.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

namespace {

using Level = agiru::PermissionLevel;
using Operation = agiru::PermissionOperation;
constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kOtherUser = "00000000-0000-0000-0000-000000000002";
constexpr std::string_view kApp = "10000000-0000-0000-0000-000000000001";
constexpr std::int32_t kTable = 2000000120;
constexpr agiru::PageDef kPage{
    .id = agiru::PageId{60020}, .name = "Accounts", .source = agiru::TableId{kTable}};
constexpr std::array kSystemEntries{agiru::PermissionEntry{
    .rights = {Level::Direct, Level::None, Level::Direct}, .object = kTable}};
constexpr agiru::PermissionSetDef kSystemSet{
    .identity = {.app = kApp, .role = "SYSTEM", .scope = agiru::PermissionScope::System},
    .permissions = kSystemEntries};

class Catalog final : public agiru::PermissionSetCatalog {
public:
  const agiru::PermissionSetDef *Find(const agiru::PermissionSetIdentity &identity) const override {
    return identity == kSystemSet.identity ? &kSystemSet : nullptr;
  }
};

void Refuses(std::string_view claim,
             const std::function<void()> &operation,
             std::string_view code = "PermissionPolicy") {
  bool refused = false;
  try {
    operation();
  } catch (const agiru::Error &error) { refused = error.Code() == code; }
  CHECK_TRUE(claim, refused);
}

class Fixture {
public:
  Fixture() : database_("native_permissions"), observer_(database_.Dsn()) {
    {
      const agiru::Session seed(database_.Dsn());
      agiru::CreateTable(seed.Database(), agiru::platform::kUserTable);
      agiru::platform::User row;
      row.UserSecurityID = agiru::Guid(kUser);
      row.UserName = "ACCOUNT";
      row.Insert();
      row.Init();
      row.UserSecurityID = agiru::Guid(kOtherUser);
      row.UserName = "OTHER";
      row.Insert();
      agiru::Commit();
    }
    Sql(R"(
CREATE TABLE "Access Control" (
 "User Security ID" uuid NOT NULL, "Role ID" varchar(20) NOT NULL,
 "Company Name" varchar(30) NOT NULL, "Scope" integer NOT NULL, "App ID" uuid NOT NULL,
 PRIMARY KEY("User Security ID","Role ID","Company Name","Scope","App ID"));
CREATE TABLE "Tenant Permission Set" (
 "App ID" uuid NOT NULL, "Role ID" varchar(20) NOT NULL, "Name" varchar(30) NOT NULL,
 "Assignable" boolean NOT NULL, PRIMARY KEY("App ID","Role ID"));
CREATE TABLE "Tenant Permission" (
 "App ID" uuid NOT NULL, "Role ID" varchar(20) NOT NULL, "Object Type" integer NOT NULL,
 "Object ID" integer NOT NULL, "Read Permission" integer NOT NULL,
 "Insert Permission" integer NOT NULL, "Modify Permission" integer NOT NULL,
 "Delete Permission" integer NOT NULL, "Execute Permission" integer NOT NULL,
 "Security Filter" text NOT NULL, "Type" integer NOT NULL,
 PRIMARY KEY("App ID","Role ID","Object Type","Object ID"));
CREATE TABLE "Tenant Permission Set Rel." (
 "App ID" uuid NOT NULL, "Role ID" varchar(30) NOT NULL, "Related App ID" uuid NOT NULL,
 "Related Role ID" varchar(30) NOT NULL, "Type" integer NOT NULL,
 "Related Scope" integer NOT NULL,
 PRIMARY KEY("App ID","Role ID","Related App ID","Related Role ID"));
)");
    Reset();
  }

  const std::string &Dsn() const { return database_.Dsn(); }

  const Catalog &System() const { return catalog_; }

  const agiru::Connection &Observer() const { return observer_; }

  void Sql(std::string_view sql) const {
    while (sql.find_first_not_of(" \r\n\t") != std::string_view::npos) {
      const auto end = sql.find(';');
      observer_.Run(sql.substr(0, end));
      if (end == std::string_view::npos) { break; }
      sql.remove_prefix(end + 1);
    }
  }

  void Reset() const {
    Sql(R"(
TRUNCATE "Access Control","Tenant Permission Set","Tenant Permission","Tenant Permission Set Rel.";
INSERT INTO "Tenant Permission Set" VALUES
 ('10000000-0000-0000-0000-000000000001','ROOT','Root',true),
 ('10000000-0000-0000-0000-000000000001','LEAF','Leaf',false),
 ('10000000-0000-0000-0000-000000000001','SUPER','Empty role',true),
 ('10000000-0000-0000-0000-000000000002','ROOT','Different app',true);
INSERT INTO "Access Control" VALUES
 ('00000000-0000-0000-0000-000000000001','ROOT','Company',1,'10000000-0000-0000-0000-000000000001'),
 ('00000000-0000-0000-0000-000000000002','ROOT','Other',1,'10000000-0000-0000-0000-000000000001');
INSERT INTO "Tenant Permission" VALUES
 ('10000000-0000-0000-0000-000000000001','ROOT',0,2000000120,1,0,0,0,0,'',0),
 ('10000000-0000-0000-0000-000000000001','ROOT',8,60020,0,0,0,0,1,'',0);
)");
  }

private:
  gate::OwnedDatabase database_;
  agiru::Connection observer_;
  Catalog catalog_;
};

Level Right(const agiru::NativePermissions &authority, Operation operation = Operation::Read) {
  return authority.Level(kTable, agiru::PermissionObject::TableData, operation);
}

void Identity(Fixture &fixture) {
  agiru::Session session(fixture.Dsn(), agiru::Guid(kUser));
  session.CompanyName("Company");
  const agiru::NativePermissions authority(fixture.System());
  CHECK_TRUE("original tenant rows grant direct read", Right(authority) == Level::Direct);
  CHECK_TRUE("missing write rights never inherit page Execute",
             Right(authority, Operation::Modify) == Level::None);
  {
    agiru::Session other(fixture.Dsn(), agiru::Guid(kOtherUser));
    other.CompanyName("Company");
    CHECK_TRUE("another user does not inherit the first user's assignments",
               Right(authority) == Level::None);
    other.CompanyName("Other");
    CHECK_TRUE("one shared provider resolves the second user's own company",
               Right(authority) == Level::Direct);
  }
  CHECK_TRUE("worker reuse restores the first user's authority", Right(authority) == Level::Direct);
  session.CompanyName("Other");
  CHECK_TRUE("company-specific assignments never cross companies", Right(authority) == Level::None);
  fixture.Sql(R"(UPDATE "Access Control" SET "Company Name"='' WHERE "User Security ID"=')" +
              std::string(kUser) + "'");
  CHECK_TRUE("blank-company assignments apply to the active company",
             Right(authority) == Level::Direct);
  fixture.Sql(R"(UPDATE "Access Control" SET "App ID"='10000000-0000-0000-0000-000000000002'
 WHERE "User Security ID"='00000000-0000-0000-0000-000000000001')");
  CHECK_TRUE("same role in another app does not inherit rights", Right(authority) == Level::None);
  fixture.Reset();
  session.CompanyName("Company");
  fixture.Sql(R"(UPDATE "Access Control" SET "Scope"=0)");
  Refuses("same role in another scope cannot substitute tenant authority",
          [&] { static_cast<void>(Right(authority)); });
  fixture.Reset();
  fixture.Sql(R"(UPDATE "Access Control" SET "Role ID"='SUPER')");
  CHECK_TRUE("a role named SUPER grants nothing without declared rights",
             Right(authority) == Level::None);
  fixture.Reset();
}

void Effects(Fixture &fixture) {
  agiru::Session session(fixture.Dsn(), agiru::Guid(kUser));
  session.CompanyName("Company");
  const auto authority = std::make_shared<agiru::NativePermissions>(fixture.System());
  session.TablePermissions(authority);
  agiru::platform::User row;
  CHECK_TRUE("native authority permits the actual typed table read", row.Get(agiru::Guid(kUser)));
  row.UserName = "DENIED";
  Refuses("denied native modify refuses before SQL", [&] { row.Modify(); }, "Permission");
  const auto seen = fixture.Observer().Execute(
      R"(SELECT "User Name" FROM "User" WHERE "User Security ID"=')" + std::string(kUser) + "'");
  CHECK_TRUE("independent SQL retains the denied row's original name",
             seen.Rows() == 1 && seen.Value(0, 0) == "ACCOUNT");
  authority->RequirePage(kPage);
  CHECK_TRUE("page and source read together permit page exposure", true);
  fixture.Sql(R"(UPDATE "Tenant Permission" SET "Read Permission"=0 WHERE "Object Type"=0)");
  Refuses(
      "SQL read revocation blocks cached page exposure",
      [&] { authority->RequirePage(kPage); },
      "Permission");
  CHECK_TRUE("revocation is observed without a process authority cache", !row.ReadPermission());
  Refuses("revoked typed reads cannot expose SQL rows", [&] { row.FindFirst(); }, "Permission");
  fixture.Reset();
  fixture.Sql(R"(UPDATE "Tenant Permission" SET "Execute Permission"=0 WHERE "Object Type"=8)");
  Refuses(
      "TableData Read does not substitute Page Execute",
      [&] { authority->RequirePage(kPage); },
      "Permission");
  fixture.Reset();
}

void Composition(Fixture &fixture) {
  agiru::Session session(fixture.Dsn(), agiru::Guid(kUser));
  session.CompanyName("Company");
  const agiru::NativePermissions authority(fixture.System());
  fixture.Sql(R"(
INSERT INTO "Tenant Permission Set Rel." VALUES
 ('10000000-0000-0000-0000-000000000001','ROOT','10000000-0000-0000-0000-000000000001','SYSTEM',0,0);
)");
  CHECK_TRUE("tenant relations resolve exact installed system declarations",
             Right(authority, Operation::Modify) == Level::Direct);
  fixture.Reset();
  fixture.Sql(R"(
INSERT INTO "Tenant Permission" VALUES
 ('10000000-0000-0000-0000-000000000001','LEAF',0,0,1,1,1,1,0,'',0);
INSERT INTO "Tenant Permission Set Rel." VALUES
 ('10000000-0000-0000-0000-000000000001','ROOT','10000000-0000-0000-0000-000000000001','LEAF',0,1);
UPDATE "Tenant Permission" SET "Read Permission"=0,"Modify Permission"=2,"Delete Permission"=1,"Type"=1
 WHERE "Role ID"='ROOT' AND "Object Type"=0;
)");
  CHECK_TRUE("native wildcard include retains read", Right(authority) == Level::Direct);
  CHECK_TRUE("native inline exclusion reduces direct modify to indirect",
             Right(authority, Operation::Modify) == Level::Indirect);
  CHECK_TRUE("native inline exclusion removes wildcard delete",
             Right(authority, Operation::Delete) == Level::None);
  Refuses(
      "indirect rights never authorize unsupported execution contexts",
      [&] {
        static_cast<void>(
            authority.Allows(agiru::platform::kUserTable, agiru::TableOperation::Modify));
      },
      "PermissionIndirectUnsupported");
  fixture.Sql(R"(UPDATE "Tenant Permission Set Rel." SET "Type"=1)");
  CHECK_TRUE("excluded tenant sets never independently grant rights",
             Right(authority) == Level::None);
  fixture.Reset();
}

void Malformed(Fixture &fixture) {
  agiru::Session session(fixture.Dsn(), agiru::Guid(kUser));
  session.CompanyName("Company");
  const agiru::NativePermissions authority(fixture.System());
  fixture.Sql(R"(DELETE FROM "Tenant Permission Set" WHERE "Role ID"='ROOT')");
  Refuses("missing SQL set declarations refuse instead of losing assignments",
          [&] { static_cast<void>(Right(authority)); });
  fixture.Reset();
  fixture.Sql(
      R"(UPDATE "Tenant Permission" SET "Security Filter"='restricted' WHERE "Object Type"=0)");
  Refuses(
      "native security filters never become unrestricted grants",
      [&] { static_cast<void>(Right(authority)); },
      "PermissionFilterUnsupported");
  for (const std::string_view column : {"Read Permission", "Object Type", "Type"}) {
    fixture.Reset();
    fixture.Sql(R"(UPDATE "Tenant Permission" SET ")" + std::string(column) +
                R"("=99 WHERE "Object Type"=0)");
    Refuses("malformed native permission ordinals refuse explicitly",
            [&] { static_cast<void>(Right(authority)); });
  }
  fixture.Reset();
  fixture.Sql(R"(
INSERT INTO "Tenant Permission Set Rel." VALUES
 ('10000000-0000-0000-0000-000000000001','ROOT','10000000-0000-0000-0000-000000000001','ROOT',0,1);
)");
  Refuses("SQL cycles terminate and refuse explicitly",
          [&] { static_cast<void>(Right(authority)); });
  fixture.Reset();
}

void Bounds(Fixture &fixture) {
  agiru::Session session(fixture.Dsn(), agiru::Guid(kUser));
  session.CompanyName("Company");
  for (const auto limits :
       {agiru::NativePermissionLimits{.composition = {.sets = 0}},
        agiru::NativePermissionLimits{.assignments = 0},
        agiru::NativePermissionLimits{.assignments = std::numeric_limits<std::size_t>::max()},
        agiru::NativePermissionLimits{.bytes = 0}}) {
    Refuses("invalid native bounds refuse before SQL",
            [&] { const agiru::NativePermissions authority(fixture.System(), limits); });
  }
  const agiru::NativePermissions exact(
      fixture.System(), {.composition = {.sets = 1, .entries = 2}, .assignments = 1});
  CHECK_TRUE("exact native assignment/set/entry bounds preserve complete policy",
             Right(exact) == Level::Direct);
  for (const auto limits : {agiru::NativePermissionLimits{.composition = {.entries = 1}},
                            agiru::NativePermissionLimits{.bytes = 1}}) {
    const agiru::NativePermissions authority(fixture.System(), limits);
    Refuses("native row/text bounds refuse excess policy",
            [&] { static_cast<void>(Right(authority)); });
  }
  fixture.Sql(
      "INSERT INTO \"Access Control\" SELECT \"User Security ID\",\"Role ID\",'',\"Scope\",\"App "
      "ID\""
      " FROM \"Access Control\" WHERE \"User Security ID\"='00000000-0000-0000-0000-000000000001'");
  Refuses("native assignment bound retains excess applicable identities",
          [&] { static_cast<void>(Right(exact)); });
  fixture.Reset();
  fixture.Sql(R"(
INSERT INTO "Tenant Permission Set Rel." VALUES
 ('10000000-0000-0000-0000-000000000001','ROOT','10000000-0000-0000-0000-000000000001','LEAF',0,1);
)");
  Refuses("native set bound refuses an excess reachable definition",
          [&] { static_cast<void>(Right(exact)); });
  const agiru::NativePermissions edges(fixture.System(), {.composition = {.edges = 1}});
  Refuses("native edge bound includes the root assignment and relationship",
          [&] { static_cast<void>(Right(edges)); });
  fixture.Reset();
}

}

int main() {
  return gate::Run("NativePermissions", [] {
    Fixture fixture;
    Identity(fixture);
    Effects(fixture);
    Composition(fixture);
    Malformed(fixture);
    Bounds(fixture);
    const agiru::Session harness(fixture.Dsn());
    const agiru::NativePermissions authority(fixture.System());
    Refuses(
        "unaccounted SYSTEM harness does not authorize native client policy",
        [&] { static_cast<void>(Right(authority)); },
        "PermissionUnavailable");
  });
}
