#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/QueryDef.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Query.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/Transaction.h"
#include "type/BigInteger.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"

#include "Check.h"
#include "OwnedDatabase.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>

namespace {

struct VersionedRow : agiru::Table<VersionedRow> {
  static constexpr agiru::TableId kId{50196};
  static constexpr std::string_view kName = "Rowversion Record Gate";
  agiru::detail::StateHandle State_Block;
  agiru::Integer ID{};
  agiru::Decimal Amount;
  agiru::BigInteger Version{};
  agiru::BigInteger LookupVersion{};
  agiru::BigInteger SystemRowVersion{};
  agiru::Guid SystemId;
  agiru::DateTime SystemCreatedAt;
  agiru::Guid SystemCreatedBy;
  agiru::DateTime SystemModifiedAt;
  agiru::Guid SystemModifiedBy;
};

}

template <> struct agiru::TableTraits<VersionedRow> {
  static constexpr std::array kDeclared{
      agiru::Declare<&VersionedRow::ID>(agiru::FieldNo{1}, "ID", "ID", offsetof(VersionedRow, ID)),
      agiru::Declare<&VersionedRow::Amount>(
          agiru::FieldNo{2}, "Amount", "Amount", offsetof(VersionedRow, Amount)),
      agiru::Declare<&VersionedRow::Version>(
          agiru::FieldNo{3},
          "Version",
          "Version",
          offsetof(VersionedRow, Version),
          agiru::Declared{.editable = false, .sqlTimestamp = true}),
      agiru::Declare<&VersionedRow::LookupVersion>(
          agiru::FieldNo{4},
          "Lookup Version",
          "Lookup Version",
          offsetof(VersionedRow, LookupVersion),
          agiru::Declared{.fieldClass = agiru::FieldClass::FlowField,
                          .calcFormula =
                              "lookup(\"Rowversion Record Gate\".Version where(ID = field(ID)))"})};
  static constexpr auto kFields = agiru::WithImplicitFields<VersionedRow,
                                                            agiru::SystemFieldProfile::Runtime17,
                                                            agiru::TableType::Normal,
                                                            false>(kDeclared);
  static constexpr std::array kPrimary{agiru::FieldNo{1}};
  static constexpr std::array kVersionKey{agiru::FieldNo{3}};
  static constexpr std::array kKeys{agiru::KeyDef{.name = "Primary", .fields = kPrimary},
                                    agiru::KeyDef{.name = "Version", .fields = kVersionKey}};
  static constexpr agiru::TableDef kTable{
      .id = VersionedRow::kId, .name = VersionedRow::kName, .fields = kFields, .keys = kKeys};
};

namespace {

const agiru::RegisterTable<VersionedRow> kRegister;

agiru::BigInteger Scalar(const agiru::Connection &connection, std::string_view sql) {
  const auto result = connection.Execute(sql);
  if (result.Rows() != 1 || result.Columns() != 1) {
    throw agiru::Error("SQL rowversion gate requires an exact non-null scalar");
  }
  const auto value = result.Value(0, 0);
  if (!value) { throw agiru::Error("SQL rowversion gate refuses a null scalar"); }
  agiru::BigInteger parsed = 0;
  const auto [end, error] = std::from_chars(value->data(), value->data() + value->size(), parsed);
  if (error != std::errc{} || end != value->data() + value->size()) {
    throw agiru::Error("SQL rowversion gate requires an exact bigint");
  }
  return parsed;
}

const agiru::TableDef &Declaration() {
  return agiru::TableTraits<VersionedRow>::kTable;
}

void CheckVersion(const VersionedRow &row, agiru::BigInteger expected) {
  CHECK_TRUE("SystemRowVersion is the stored platform version", row.SystemRowVersion == expected);
  CHECK_TRUE("SqlTimestamp is an alias, not an independently stored number",
             row.Version == expected);
}

VersionedRow Inserted(agiru::Integer id) {
  VersionedRow row;
  row.ID = id;
  row.Amount = agiru::Decimal{1} / agiru::Decimal{3};
  row.Version = -1;
  row.SystemRowVersion = -1;
  row.Insert();
  return row;
}

void PhysicalStorageAndWrites() {
  const gate::OwnedDatabase database("sql_writes");
  const agiru::Session session(database.Dsn());
  const auto &connection = session.Database();
  agiru::CreateTable(connection, Declaration());
  constexpr auto physicalColumns = 8;
  CHECK_TRUE("aliases and FlowFields do not create extra physical columns",
             Scalar(connection,
                    "SELECT count(*) FROM information_schema.columns "
                    "WHERE table_name = 'Rowversion Record Gate'") == physicalColumns);
  CHECK_TRUE("source SqlTimestamp names never create ordinary columns",
             Scalar(connection,
                    "SELECT count(*) FROM information_schema.columns "
                    "WHERE table_name = 'Rowversion Record Gate' AND "
                    "column_name IN ('Version', 'SystemRowVersion', 'Lookup Version')") == 0);
  auto row = Inserted(1);
  CheckVersion(row, 1);
  CHECK_TEXT("INSERT's returned platform values do not quantize the Decimal buffer",
             row.Amount.ToInvariantString(),
             "0.3333333333333333333333333333");
  const auto identity = row.SystemId;
  const auto created = row.SystemCreatedAt;
  row.Version = -1;
  row.SystemRowVersion = -1;
  row.Modify();
  CheckVersion(row, 2);
  CHECK_TRUE("modification preserves the original identity and creation audit",
             row.SystemId == identity && row.SystemCreatedAt == created);
  CHECK_TEXT("UPDATE's returned platform values preserve calculation scale",
             row.Amount.ToInvariantString(),
             "0.3333333333333333333333333333");
  row.Rename(2);
  CheckVersion(row, 3);
  CHECK_TRUE("Rename preserves SystemId", row.SystemId == identity);
  VersionedRow read;
  CHECK_TRUE("Get reads the renamed row", read.Get(2));
  CheckVersion(read, 3);
  CHECK_TRUE("GetBySystemId reads the same physical version", read.GetBySystemId(identity));
  CheckVersion(read, 3);
  CHECK_TEXT("ordinary data is quantized only when actually read from SQL",
             read.Amount.ToInvariantString(),
             "0.33333333333333333333");
  read.CalcFields(read.LookupVersion);
  CHECK_TRUE("FlowField lookup resolves a timestamp alias", read.LookupVersion == 3);
  CHECK_TRUE("DELETE does not allocate a new rowversion", read.Delete());
  CHECK_TRUE("last used remains the last INSERT/UPDATE allocation",
             Scalar(connection, "SELECT agiru_platform.last_rowversion_v1()") == 3);
}

void SystemIdLookupsShareOptionalResultsAndCursorPosition() {
  const gate::OwnedDatabase database("sql_system_id");
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), Declaration());
  const auto first = Inserted(1);
  const auto second = Inserted(2);
  VersionedRow typed;
  typed.SetRange(typed.ID, 1);
  CHECK_TRUE("a typed SystemId lookup ignores ordinary filters",
             typed.GetBySystemId(second.SystemId));
  CHECK_TRUE("typed lookup loads the requested identity and exact rowversion",
             typed.ID == 2 && typed.SystemId == second.SystemId && typed.SystemRowVersion == 2);
  CHECK_TEXT("typed SystemId lookup leaves its filters unchanged", typed.GetFilter(typed.ID), "1");
  CHECK_TRUE("typed SystemId lookup captures the stored image", typed.StoredImage().ID == 2);

  agiru::RecordRef reference;
  reference.GetTable(typed);
  reference.Field(1).SetFilter("2");
  CHECK_TRUE("RecordRef uses the same SystemId reader", reference.GetBySystemId(first.SystemId));
  VersionedRow reflected;
  reference.SetTable(reflected);
  CHECK_TRUE("RecordRef loads exact identity, primary key and version",
             reflected.ID == 1 && reflected.SystemId == first.SystemId &&
                 reflected.SystemRowVersion == 1);
  CHECK_TEXT("RecordRef SystemId lookup preserves the record's filters",
             reflected.GetFilter(reflected.ID),
             "2");
  typed.GetBySystemId(first.SystemId);
  CHECK_TEXT("typed and RecordRef SystemId reads agree on exact Decimal data",
             typed.Amount.ToInvariantString(),
             reflected.Amount.ToInvariantString());
  CHECK_TRUE("SystemId lookup records a current position",
             reflected.State_Block.Peek()->positioned);
  reflected.SetRange(reflected.ID);
  CHECK_TRUE("navigation continues after the reflected SystemId position",
             reflected.Next() == 1 && reflected.ID == 2);

  const agiru::Guid missing;
  CHECK_TRUE("a consumed typed missing SystemId lookup returns false",
             !typed.GetBySystemId(missing));
  CHECK_TRUE("a consumed RecordRef missing SystemId lookup returns false",
             !reference.GetBySystemId(missing));
  CHECK_TRUE("a missing typed SystemId read retains the previous record",
             typed.ID == 1 && typed.SystemId == first.SystemId);
  reference.SetTable(reflected);
  CHECK_TRUE("a missing reflected SystemId read retains the previous record",
             reflected.ID == 1 && reflected.SystemId == first.SystemId);
  std::string expected =
      "The Rowversion Record Gate does not exist. Identification fields and values: SystemId='";
  expected += missing.ToText();
  expected += '\'';
  std::string typedError;
  try {
    typed.GetBySystemId(missing);
  } catch (const agiru::Error &error) { typedError = error.what(); }
  CHECK_TEXT("discarding a typed missing SystemId result raises its searched identity",
             typedError,
             expected);
  std::string reflectedError;
  try {
    reference.GetBySystemId(missing);
  } catch (const agiru::Error &error) { reflectedError = error.what(); }
  CHECK_TEXT("discarding a reflected missing SystemId result raises the same diagnostic",
             reflectedError,
             expected);
  agiru::RecordRef closed;
  bool refused = false;
  try {
    static_cast<void>(static_cast<bool>(closed.GetBySystemId(first.SystemId)));
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("a SystemId lookup requires an already open RecordRef", refused);
  session.Database().Run(R"(ALTER TABLE "Rowversion Record Gate" DROP COLUMN "SystemId")");
  bool typedStorageError = false;
  try {
    static_cast<void>(static_cast<bool>(typed.GetBySystemId(first.SystemId)));
  } catch (const agiru::DatabaseError &) { typedStorageError = true; }
  CHECK_TRUE("a consumed typed SystemId lookup does not hide a storage failure", typedStorageError);
  bool reflectedStorageError = false;
  try {
    static_cast<void>(static_cast<bool>(reference.GetBySystemId(first.SystemId)));
  } catch (const agiru::DatabaseError &) { reflectedStorageError = true; }
  CHECK_TRUE("a consumed reflected SystemId lookup does not hide a storage failure",
             reflectedStorageError);
}

void RecordIdReadsPreserveOptionalResultsOnSql() {
  const gate::OwnedDatabase database("sql_record_id");
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), Declaration());
  const auto first = Inserted(1);
  VersionedRow typed;
  typed.SetRange(typed.ID, 2);
  agiru::RecordRef reference;
  reference.GetTable(typed);
  CHECK_TRUE("RecordRef.Get ignores normal SQL filters", reference.Get(first.RecordId()));
  reference.SetTable(typed);
  CHECK_TRUE("RecordRef.Get preserves exact SQL identity and rowversion",
             typed.ID == 1 && typed.SystemId == first.SystemId && typed.SystemRowVersion == 1);
  CHECK_TEXT("RecordRef.Get preserves exact SQL Decimal values",
             typed.Amount.ToInvariantString(),
             "0.33333333333333333333");
  CHECK_TEXT("RecordRef.Get leaves the SQL filter unchanged", typed.GetFilter(typed.ID), "2");
  constexpr agiru::Integer kAbsentId = 999;
  VersionedRow absent;
  absent.ID = kAbsentId;
  const auto missing = absent.RecordId();
  CHECK_TRUE("consumed SQL RecordRef.Get misses return false", !reference.Get(missing));
  std::string diagnostic;
  try {
    reference.Get(missing);
  } catch (const agiru::Error &error) { diagnostic = error.what(); }
  CHECK_TEXT(
      "discarded SQL RecordRef.Get misses identify the searched key",
      diagnostic,
      "The Rowversion Record Gate does not exist. Identification fields and values: ID='999'");
  CHECK_TRUE("RecordRef.Get is independently read-only in SQL",
             Scalar(session.Database(), "SELECT count(*) FROM \"Rowversion Record Gate\"") == 1);
  bool malformed = false;
  try {
    static_cast<void>(static_cast<bool>(reference.Get(agiru::RecordId{})));
  } catch (const agiru::Error &) { malformed = true; }
  CHECK_TRUE("consumed RecordRef.Get does not hide a malformed identity", malformed);
  session.Database().Run(R"(ALTER TABLE "Rowversion Record Gate" DROP COLUMN "Amount")");
  bool storageFailure = false;
  try {
    static_cast<void>(static_cast<bool>(reference.Get(first.RecordId())));
  } catch (const agiru::DatabaseError &) { storageFailure = true; }
  CHECK_TRUE("consumed RecordRef.Get does not hide a SQL provider failure", storageFailure);
}

void SelectionReflectionAndBulkWrites() {
  constexpr agiru::BigInteger firstBulkVersion = 5;
  constexpr agiru::BigInteger lastBulkVersion = firstBulkVersion + 2;
  const gate::OwnedDatabase database("sql_selection");
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), Declaration());
  static_cast<void>(Inserted(1));
  static_cast<void>(Inserted(2));
  auto row = Inserted(3);
  row.SetRange(row.Version, agiru::BigInteger{2});
  CHECK_TRUE("alias filters reach the physical timestamp", row.FindFirst());
  CHECK_TRUE("the filtered row is the second allocation", row.ID == 2);
  CheckVersion(row, 2);
  row.Reset();
  CHECK_TRUE("the alias's declared key is available", row.SetCurrentKey(row.Version));
  row.Ascending(false);
  CHECK_TRUE("alias ordering reaches the newest row", row.FindFirst());
  CheckVersion(row, 3);
  CHECK_TRUE("cursor Next reads the next physical version", row.Next() == 1);
  CheckVersion(row, 2);
  agiru::RecordRef reference;
  reference.GetTable(row);
  reference.Field(0).Value(agiru::BigInteger{-1});
  reference.Field(3).Value(agiru::BigInteger{-1});
  CHECK_TRUE("RecordRef Modify returns its database-owned version", reference.Modify());
  reference.SetTable(row);
  CheckVersion(row, 4);
  CHECK_TEXT("FieldRef timestamp reflects the returned version", reference.Field(0).ToText(), "4");
  CHECK_TEXT("FieldRef alias reflects the same returned version", reference.Field(3).ToText(), "4");
  row.Reset();
  row.ModifyAll(row.Amount, agiru::Decimal{2});
  CHECK_TRUE("ModifyAll allocates once for every affected row",
             Scalar(session.Database(), "SELECT agiru_platform.last_rowversion_v1()") == 7);
  CHECK_TRUE("ModifyAll never reuses a version",
             Scalar(session.Database(),
                    "SELECT count(DISTINCT \"timestamp\") FROM \"Rowversion Record Gate\"") == 3);
  row.SetRange(row.SystemRowVersion, firstBulkVersion, lastBulkVersion);
  CHECK_TRUE("system-field filters select the same physical versions", row.Count() == 3);
  row.Reset();
  row.SetRange(row.LookupVersion, firstBulkVersion, lastBulkVersion);
  CHECK_TRUE("correlated FlowField filters read the physical timestamp alias", row.Count() == 3);
}

void QueryAliasesAndLinks() {
  struct ResultRow {
    agiru::Integer id{};
    agiru::BigInteger version{};
    agiru::BigInteger linkedVersion{};
  };

  const gate::OwnedDatabase database("sql_query");
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), Declaration());
  static_cast<void>(Inserted(1));
  static_cast<void>(Inserted(2));
  static_cast<void>(Inserted(3));
  const std::array links{
      agiru::QueryLink{.field = agiru::FieldNo{0}, .dataItem = 0, .reference = agiru::FieldNo{3}}};
  const std::array items{agiru::QueryDataItem{.name = "Upper",
                                              .table = &Declaration(),
                                              .tableFilter = "Version = filter(>=2)"},
                         agiru::QueryDataItem{.name = "Lower",
                                              .table = &Declaration(),
                                              .join = agiru::QueryJoin::Inner,
                                              .links = links}};
  const std::array columns{agiru::QueryColumn{.name = "ID",
                                              .offset = offsetof(ResultRow, id),
                                              .dataItem = 0,
                                              .field = agiru::FieldNo{1}},
                           agiru::QueryColumn{.name = "Version",
                                              .offset = offsetof(ResultRow, version),
                                              .dataItem = 0,
                                              .field = agiru::FieldNo{3}},
                           agiru::QueryColumn{.name = "Linked",
                                              .offset = offsetof(ResultRow, linkedVersion),
                                              .dataItem = 1,
                                              .field = agiru::FieldNo{0}}};
  const std::array order{agiru::QueryOrder{.column = "Version"}};
  const agiru::QueryDef query{.id = agiru::QueryId{50197},
                              .name = "Rowversion Alias Query",
                              .dataItems = items,
                              .columns = columns,
                              .orderBy = order};
  agiru::detail::QueryHandle handle;
  CHECK_TRUE("a query with timestamp projection/filter/join opens",
             agiru::detail::QueryOpen(handle.Ensure(), query));
  ResultRow row;
  for (const auto expected : {2, 3}) {
    CHECK_TRUE("the query returns each filtered row",
               agiru::detail::QueryRead(handle.Ensure(), query, &row));
    CHECK_TRUE("query projection resolves the declared alias", row.version == expected);
    CHECK_TRUE("DataItemLink joins the alias to the implicit timestamp",
               row.linkedVersion == expected);
    CHECK_TRUE("query order and original primary key remain exact", row.id == expected);
  }
  CHECK_TRUE("the query does not materialize unrelated rows",
             !agiru::detail::QueryRead(handle.Ensure(), query, &row));
}

void RelativeAliasNavigation() {
  const gate::OwnedDatabase database("sql_navigation");
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), Declaration());
  static_cast<void>(Inserted(1));
  static_cast<void>(Inserted(2));
  auto row = Inserted(3);
  CHECK_TRUE("relative navigation selects the source timestamp key",
             row.SetCurrentKey(row.Version));
  row.Ascending(false);
  CHECK_TRUE("FindLast reverses timestamp alias ordering", row.FindLast());
  CheckVersion(row, 1);
  CHECK_TRUE("negative Next reopens a cursor through the timestamp tuple", row.Next(-1) == -1);
  CheckVersion(row, 2);
  CHECK_TRUE("relative Find uses the alias's physical column", row.Find(">"));
  CheckVersion(row, 1);
  CHECK_TRUE("exact Find uses the same timestamp/key tuple", row.Find("="));
  CheckVersion(row, 1);
  row.Ascending(true);
  row.SetAscending(row.Version, false);
  CHECK_TRUE("mixed-direction FindLast orders the physical timestamp", row.FindLast());
  CheckVersion(row, 1);
  CHECK_TRUE("mixed-direction relative Find maps timestamp predicates", row.Find("<"));
  CheckVersion(row, 2);
  CHECK_TRUE("mixed-direction negative Next preserves alias and primary-key directions",
             row.Next(-1) == -1);
  CheckVersion(row, 3);
}

void InvalidTimestampDeclarationsRefuse() {
  const gate::OwnedDatabase database("sql_invalid");
  const agiru::Connection connection(database.Dsn());
  auto fields = agiru::TableTraits<VersionedRow>::kFields;
  auto table = Declaration();
  table.fields = fields;
  for (const auto invalid : {agiru::FieldType::Integer, agiru::FieldType::Text}) {
    fields[0].type = invalid;
    bool refused = false;
    try {
      agiru::CreateTable(connection, table);
    } catch (const agiru::Error &error) {
      refused =
          std::string_view(error.what()).contains("SqlTimestamp requires a stored BigInteger");
    }
    CHECK_TRUE("a non-BigInteger timestamp refuses before creating storage", refused);
  }
  const auto refused = [&](std::string_view diagnostic) {
    try {
      agiru::CreateTable(connection, table);
    } catch (const agiru::Error &error) {
      return std::string_view(error.what()).contains(diagnostic);
    }
    return false;
  };
  fields = agiru::TableTraits<VersionedRow>::kFields;
  for (const auto invalid : {agiru::FieldClass::FlowField, agiru::FieldClass::FlowFilter}) {
    fields[0].fieldClass = invalid;
    CHECK_TRUE("a nonstored timestamp refuses before creating storage",
               refused("SqlTimestamp requires a stored BigInteger"));
  }
  fields = agiru::TableTraits<VersionedRow>::kFields;
  fields[0].autoIncrement = true;
  CHECK_TRUE("timestamp and AutoIncrement cannot compete for allocation",
             refused("SqlTimestamp requires a stored BigInteger"));
  fields = agiru::TableTraits<VersionedRow>::kFields;
  fields[1].name = "timestamp";
  CHECK_TRUE("an ordinary timestamp column cannot collide with the platform column",
             refused("SqlTimestamp collides with an ordinary timestamp column"));
  CHECK_TRUE("invalid declarations install neither counter nor application table",
             Scalar(connection,
                    "SELECT count(*) FROM pg_catalog.pg_class AS relation "
                    "JOIN pg_catalog.pg_namespace AS schema ON schema.oid = relation.relnamespace "
                    "WHERE schema.nspname IN ('public', 'agiru_platform')") == 0);
}

void SchemaMigrationAndRefusal() {
  const gate::OwnedDatabase database("sql_migration");
  const agiru::Session session(database.Dsn());
  const auto &connection = session.Database();
  constexpr std::size_t legacyCount = 2;
  constexpr std::array legacyFields{agiru::TableTraits<VersionedRow>::kDeclared[0],
                                    agiru::TableTraits<VersionedRow>::kDeclared[1]};
  auto legacy = Declaration();
  legacy.fields = legacyFields;
  legacy.keys = Declaration().keys.first(1);
  agiru::CreateTable(connection, legacy);
  connection.Run(
      R"(INSERT INTO "Rowversion Record Gate" ("ID", "Amount") VALUES (1, 10), (2, 20))");
  agiru::ProvisionTable(connection, Declaration());
  CHECK_TRUE("migration allocates one nonzero version per existing row",
             Scalar(connection,
                    "SELECT count(DISTINCT \"timestamp\") FROM \"Rowversion Record Gate\" "
                    "WHERE \"timestamp\" > 0") == legacyCount);
  CHECK_TRUE("migration preserves the existing business data",
             Scalar(connection, "SELECT SUM(\"Amount\")::bigint FROM \"Rowversion Record Gate\"") ==
                 30);
  const auto before = Scalar(connection, "SELECT agiru_platform.last_rowversion_v1()");
  agiru::ProvisionTable(connection, Declaration());
  CHECK_TRUE("repeated schema sync never restamps old rows",
             Scalar(connection, "SELECT agiru_platform.last_rowversion_v1()") == before);
  connection.Run(R"(ALTER TABLE "Rowversion Record Gate" ALTER COLUMN "timestamp" SET DEFAULT 0)");
  bool refused = false;
  try {
    agiru::ProvisionTable(connection, Declaration());
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).contains("incompatible timestamp column");
  }
  CHECK_TRUE("a zero/default timestamp column is not accepted as rowversion storage", refused);
}

void RollbackAndTwoSessions() {
  const gate::OwnedDatabase database("sql_transactions");
  const agiru::Connection observer(database.Dsn());
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), Declaration());
  {
    const agiru::detail::Scope scope;
    auto pending = Inserted(1);
    CheckVersion(pending, 1);
    CHECK_TRUE("the actual record writer holds the active minimum",
               Scalar(observer, "SELECT agiru_platform.minimum_rowversion_v1()") == 1);
    {
      const agiru::Session newer(database.Dsn());
      auto committed = Inserted(2);
      CheckVersion(committed, 2);
    }
    CHECK_TRUE("another session's commit cannot skip the pending record writer",
               Scalar(observer, "SELECT agiru_platform.minimum_rowversion_v1()") == 1);
  }
  CHECK_TRUE("record rollback removes only its uncommitted row",
             Scalar(observer, "SELECT count(*) FROM \"Rowversion Record Gate\"") == 1);
  CHECK_TRUE("rollback does not reuse either record's consumed version",
             Scalar(observer, "SELECT agiru_platform.last_rowversion_v1()") == 2);
  CheckVersion(Inserted(3), 3);
}

}

int main() {
  return gate::Run("SqlRowVersion", [] {
    PhysicalStorageAndWrites();
    SystemIdLookupsShareOptionalResultsAndCursorPosition();
    RecordIdReadsPreserveOptionalResultsOnSql();
    SelectionReflectionAndBulkWrites();
    SchemaMigrationAndRefusal();
    QueryAliasesAndLinks();
    RelativeAliasNavigation();
    InvalidTimestampDeclarationsRefuse();
    RollbackAndTwoSessions();
  });
}
