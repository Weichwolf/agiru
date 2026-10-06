#include "runtime/Storage.h"

#include "meta/Ids.h"
#include "meta/ProfileDef.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "platform/AllObj.h"
#include "platform/AllObjType.h"
#include "platform/AllObjWithCaption.h"
#include "platform/AllProfile.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/UserPersonalization.h"
#include "runtime/Catalogue.h"
#include "runtime/Codeunit.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/NumberSequenceStorage.h"
#include "runtime/RowVersionStorage.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"
#include "type/Date.h"
#include "type/FieldClass.h"
#include "type/Integer.h"

#include "RecordChanges.h"
#include "Rows.h"
#include "Selection.h"
#include "SqlColumn.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <print>
#include <set>
#include <span>
#include <string>
#include <string_view>

namespace agiru {

void RequireTableProvider(const TableDef &table) {
  if (!table.providerRefusal.empty()) {
    throw Error("table " + std::string(table.name) + " (" + std::to_string(table.id.Value()) +
                "): " + std::string(table.providerRefusal));
  }
}

namespace {

std::string Quoted(std::string_view identifier) {
  std::string out = "\"";
  for (const char c : identifier) {
    if (c == '"') { out += '"'; }
    out += c;
  }
  out += '"';
  return out;
}

std::string Placeholder(std::size_t oneBased) {
  return "$" + std::to_string(oneBased);
}

std::size_t IndexOf(const TableDef &table, FieldNo no) {
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (table.fields[i].no == no) { return i; }
  }
  throw Error("Storage: the table declares no such field");
}

std::string KeyPredicate(const TableDef &table, std::size_t firstPlaceholder) {
  if (table.keys.empty()) { throw Error("Storage: the table declares no key"); }
  std::string out;
  std::size_t n = firstPlaceholder;
  for (const FieldNo no : table.keys[0].fields) {
    if (!out.empty()) { out += " AND "; }
    out += detail::SqlColumn(table.fields[IndexOf(table, no)]) + " = " + Placeholder(n);
    ++n;
  }
  return out;
}

FieldValues RowOf(const Result &result, std::size_t row) {
  const std::size_t columns = result.Columns();
  FieldValues values;
  values.reserve(columns);
  for (std::size_t c = 0; c < columns; ++c) {
    const std::optional<std::string_view> v = result.Value(row, c);
    values.emplace_back(v.has_value() ? std::optional<std::string>(std::string(*v)) : std::nullopt);
  }
  return values;
}

}

std::string ColumnType(const FieldDef &def) {
  switch (def.type) {
    case FieldType::Boolean: return "boolean";
    case FieldType::Integer:
    case FieldType::Option:
    case FieldType::Enum: return "integer";
    case FieldType::BigInteger: return "bigint";
    case FieldType::Decimal: return "numeric(38,20)";
    case FieldType::Code:
    case FieldType::Text: return "varchar(" + std::to_string(def.length) + ")";
    case FieldType::Date: return "timestamp";
    case FieldType::Time: return "time";
    case FieldType::DateTime: return "timestamp";
    case FieldType::Guid: return "uuid";
    case FieldType::DateFormula: return "varchar(32)";
    case FieldType::RecordId:
    case FieldType::TableFilter: return "text";
    case FieldType::Duration: return "bigint";
    case FieldType::Blob: return "bytea";
    case FieldType::Media:
    case FieldType::MediaSet: return "uuid";
  }
  throw Error("ColumnType: no SQL type for this field type yet");
}

namespace detail {

std::string ColumnZero(const FieldDef &field) {
  switch (field.type) {
    case FieldType::Boolean: return "false";
    case FieldType::Option:
    case FieldType::Enum:
    case FieldType::Integer:
    case FieldType::BigInteger:
    case FieldType::Decimal:
    case FieldType::Duration: return "0";
    case FieldType::Date:
    case FieldType::DateTime: return "'1753-01-01 00:00:00'";
    case FieldType::Time: return "'00:00:00'";
    case FieldType::Guid:
    case FieldType::Media:
    case FieldType::MediaSet: return "'00000000-0000-0000-0000-000000000000'";
    case FieldType::Blob: return "''::bytea";
    default: return "''";
  }
}
}

namespace detail {

std::string SequenceName(const TableDef &table, const FieldDef &field) {
  return std::string(table.name) + "$" + std::string(field.name);
}

bool DrawsFromSequence(const FieldDef &field) {
  return field.autoIncrement && field.fieldClass == FieldClass::Normal &&
         (field.type == FieldType::Integer || field.type == FieldType::BigInteger);
}

}

namespace {

void EnsureSequences(const Connection &connection, const TableDef &table) {
  for (const FieldDef &field : table.fields) {
    if (!detail::DrawsFromSequence(field)) { continue; }
    const std::string sequence = Quoted(detail::SequenceName(table, field));
    connection.Run("CREATE SEQUENCE IF NOT EXISTS " + sequence);
    connection.Run("SELECT setval('" + sequence + "', (SELECT COALESCE(MAX(" + Quoted(field.name) +
                   "), 0) FROM " + Quoted(table.name) + ") + 1, false)");
  }
}

}

namespace {

std::string ColumnDefault(const FieldDef &field) {
  return field.sqlTimestamp ? std::string("agiru_platform.next_rowversion_v1()")
                            : detail::ColumnZero(field);
}

bool FirstPhysicalColumn(std::set<std::string_view> &physical, const FieldDef &field) {
  if (physical.insert(detail::ColumnName(field)).second) { return true; }
  if (!field.sqlTimestamp) { throw Error("Storage: duplicate physical column declaration"); }
  return false;
}

void CreateStorageTable(const Connection &connection, const TableDef &table) {
  RequireTableProvider(table);
  std::string sql = "CREATE TABLE " + Quoted(table.name) + " (";
  bool written = false;
  std::set<std::string_view> physical;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (!FirstPhysicalColumn(physical, field)) { continue; }
    if (written) { sql += ", "; }
    written = true;
    sql += detail::SqlColumn(field) + " " + ColumnType(field) + " NOT NULL DEFAULT " +
           ColumnDefault(field);
  }
  if (!table.keys.empty()) {
    sql += ", PRIMARY KEY (";
    bool first = true;
    for (const FieldNo no : table.keys[0].fields) {
      if (!first) { sql += ", "; }
      first = false;
      sql += detail::SqlColumn(table.fields[IndexOf(table, no)]);
    }
    sql += ")";
  }
  sql += ")";
  connection.Run(sql);

  for (std::size_t k = 1; k < table.keys.size(); ++k) {
    if (table.keys[k].fields.empty()) { continue; }
    if (!table.keys[k].enabled || !table.keys[k].maintainSqlIndex) { continue; }
    std::string index = "CREATE INDEX " +
                        Quoted(std::string(table.name) + "$" + std::string(table.keys[k].name)) +
                        " ON " + Quoted(table.name) + " (";
    bool first = true;
    for (const FieldNo no : table.keys[k].fields) {
      if (!first) { index += ", "; }
      first = false;
      index += detail::SqlColumn(table.fields[IndexOf(table, no)]);
    }
    index += ")";
    connection.Run(index);
  }
  EnsureSequences(connection, table);
}

}

void CreateTable(const Connection &connection, const TableDef &table) {
  RequireTableProvider(table);
  if (detail::HasRowVersion(table)) { ProvisionRowVersions(connection); }
  CreateStorageTable(connection, table);
}

void DropTable(const Connection &connection, const TableDef &table) {
  RequireTableProvider(table);
  connection.Run("DROP TABLE IF EXISTS " + Quoted(table.name));
  for (const FieldDef &field : table.fields) {
    if (!detail::DrawsFromSequence(field)) { continue; }
    connection.Run("DROP SEQUENCE IF EXISTS " + Quoted(detail::SequenceName(table, field)));
  }
}

namespace {

std::size_t StoredCount(const TableDef &table) {
  return static_cast<std::size_t>(std::ranges::count_if(table.fields, Stored));
}
}

namespace {

std::string StoredColumns(const TableDef &table) {
  std::string columns;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (!columns.empty()) { columns += ", "; }
    columns += detail::SqlColumn(field);
  }
  return columns;
}
}

namespace {

std::size_t StoredIndexOf(const TableDef &table, FieldNo no) {
  std::size_t column = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (field.no == no) { return column; }
    ++column;
  }
  throw Error("the key names a field the schema does not store");
}
}

namespace {

std::string OwnedColumns(const TableDef &table);

}

std::optional<FieldValues> InsertRow(const Connection &connection,
                                     const TableDef &table,
                                     std::span<const std::optional<std::string>> values) {
  RequireTableProvider(table);
  if (values.size() != StoredCount(table)) {
    throw Error("Insert: the value count does not match the declaration");
  }
  std::string columns;
  std::string placeholders;
  FieldValues bound;
  std::size_t column = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    const std::size_t index = column++;
    if (field.sqlTimestamp) { continue; }
    if (!columns.empty()) {
      columns += ", ";
      placeholders += ", ";
    }
    bound.push_back(values[index]);
    columns += detail::SqlColumn(field);
    placeholders += Placeholder(bound.size());
  }
  const std::string insertion = columns.empty()
                                    ? std::string(" DEFAULT VALUES")
                                    : " (" + columns + ") VALUES (" + placeholders + ")";
  const Result written =
      connection.Execute("INSERT INTO " + Quoted(table.name) + insertion +
                             " ON CONFLICT DO NOTHING RETURNING " + OwnedColumns(table),
                         bound);
  if (written.Rows() == 0) { return std::nullopt; }
  detail::RecordWritten(connection, table.id);
  return RowOf(written, 0);
}

std::optional<FieldValues> GetRow(const Connection &connection,
                                  const TableDef &table,
                                  std::span<const std::optional<std::string>> key) {
  RequireTableProvider(table);
  const std::string columns = StoredColumns(table);
  const Result result = connection.Execute("SELECT " + columns + " FROM " + Quoted(table.name) +
                                               " WHERE " + KeyPredicate(table, 1),
                                           key);
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

std::optional<FieldValues> GetRowWhere(const Connection &connection,
                                       const TableDef &table,
                                       const FieldDef &column,
                                       std::string_view value) {
  RequireTableProvider(table);
  const std::string columns = StoredColumns(table);
  const std::optional<std::string> bound{std::string(value)};
  const Result result =
      connection.Execute("SELECT " + columns + " FROM " + Quoted(table.name) + " WHERE " +
                             detail::SqlColumn(column) + " = " + Placeholder(1),
                         std::span<const std::optional<std::string>>(&bound, 1));
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

bool PlatformOwned(const FieldDef &field) {
  return field.sqlTimestamp || field.no == SystemFieldNumbers::SystemId ||
         field.no == SystemFieldNumbers::SystemCreatedAt ||
         field.no == SystemFieldNumbers::SystemCreatedBy;
}

namespace {

std::string WrittenAssignments(const TableDef &table,
                               std::span<const std::optional<std::string>> values,
                               FieldValues &bound) {
  std::string assignments;
  std::size_t column = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    const std::size_t index = column++;
    if (PlatformOwned(field)) { continue; }
    if (!bound.empty()) { assignments += ", "; }
    bound.push_back(values[index]);
    assignments += detail::SqlColumn(field) + " = " + Placeholder(bound.size());
  }
  if (detail::HasRowVersion(table)) {
    if (!assignments.empty()) { assignments += ", "; }
    assignments += Quoted(detail::kRowVersionColumn) + " = agiru_platform.next_rowversion_v1()";
  }
  return assignments;
}

std::string OwnedColumns(const TableDef &table) {
  std::string columns;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field) || !PlatformOwned(field)) { continue; }
    if (!columns.empty()) { columns += ", "; }
    columns += detail::SqlColumn(field);
  }
  return columns.empty() ? std::string("1") : columns;
}

std::optional<FieldValues> Updated(const Connection &connection,
                                   const TableDef &table,
                                   const std::string &assignments,
                                   std::size_t keyAt,
                                   const FieldValues &bound) {
  const Result result =
      connection.Execute("UPDATE " + Quoted(table.name) + " SET " + assignments + " WHERE " +
                             KeyPredicate(table, keyAt) + " RETURNING " + OwnedColumns(table),
                         bound);
  if (result.Rows() == 0) { return std::nullopt; }
  detail::RecordWritten(connection, table.id);
  return RowOf(result, 0);
}

}

std::optional<FieldValues> ModifyRow(const Connection &connection,
                                     const TableDef &table,
                                     std::span<const std::optional<std::string>> values) {
  RequireTableProvider(table);
  if (values.size() != StoredCount(table)) {
    throw Error("Modify: the value count does not match the declaration");
  }
  FieldValues bound;
  bound.reserve(values.size() + table.keys[0].fields.size());
  const std::string assignments = WrittenAssignments(table, values, bound);
  const std::size_t keyAt = bound.size() + 1;
  for (const FieldNo no : table.keys[0].fields) {
    bound.push_back(values[StoredIndexOf(table, no)]);
  }
  return Updated(connection, table, assignments, keyAt, bound);
}

std::optional<FieldValues> RenameRow(const Connection &connection,
                                     const TableDef &table,
                                     std::span<const std::optional<std::string>> values,
                                     std::span<const std::optional<std::string>> oldKey) {
  RequireTableProvider(table);
  if (values.size() != StoredCount(table)) {
    throw Error("Rename: the value count does not match the declaration");
  }
  FieldValues bound;
  bound.reserve(values.size() + oldKey.size());
  const std::string assignments = WrittenAssignments(table, values, bound);
  const std::size_t keyAt = bound.size() + 1;
  bound.insert(bound.end(), oldKey.begin(), oldKey.end());
  return Updated(connection, table, assignments, keyAt, bound);
}

bool DeleteRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> key) {
  RequireTableProvider(table);
  const Result result = connection.Execute("DELETE FROM " + Quoted(table.name) + " WHERE " +
                                               KeyPredicate(table, 1) + " RETURNING 1",
                                           key);
  const bool deleted = result.Rows() != 0;
  if (deleted) { detail::RecordWritten(connection, table.id); }
  return deleted;
}

std::string_view Required(const std::optional<std::string> &value, const FieldDef &def) {
  if (!value.has_value()) {
    throw Error("the column for field '" + std::string(def.name) + "' is null");
  }
  return *value;
}

namespace {

constexpr unsigned char kUtf8ContinuationMask = 0xC0U;
constexpr unsigned char kUtf8ContinuationTag = 0x80U;
constexpr unsigned kDecember = 12;
constexpr unsigned kDecemberLastDay = 31;

std::string_view Fitted(std::string_view text, std::size_t length) {
  if (text.size() <= length) { return text; }
  std::size_t end = length;
  while (end > 0 &&
         (static_cast<unsigned char>(text[end]) & kUtf8ContinuationMask) == kUtf8ContinuationTag) {
    --end;
  }
  return text.substr(0, end);
}

}

namespace {

struct SchemaChanges {
  std::size_t added = 0;
  std::size_t widened = 0;
};

struct ExistingColumn {
  std::size_t length = 0;
  std::string type;
  std::string initial;
  bool nullable = false;
};

SchemaChanges EnsureColumns(const Connection &into, const TableDef &table) {
  const std::array<std::optional<std::string>, 1> named{Quoted(table.name)};
  const Result columns = into.Execute(
      "SELECT column_name, data_type, character_maximum_length, column_default, is_nullable "
      "FROM information_schema.columns "
      "WHERE (table_schema, table_name) = (SELECT schema.nspname, relation.relname "
      "FROM pg_catalog.pg_class AS relation JOIN pg_catalog.pg_namespace AS schema "
      "ON schema.oid = relation.relnamespace WHERE relation.oid = pg_catalog.to_regclass($1))",
      named);
  std::map<std::string, ExistingColumn, std::less<>> there;
  for (std::size_t row = 0; row < columns.Rows(); ++row) {
    const std::optional<std::string_view> name = columns.Value(row, 0);
    if (!name.has_value()) { continue; }
    const auto length = columns.Value(row, 2);
    const bool bounded = columns.Value(row, 1) == "character varying" && length.has_value();
    there.emplace(*name,
                  ExistingColumn{.length = bounded ? std::stoull(std::string(*length)) : 0,
                                 .type = std::string(columns.Value(row, 1).value_or("")),
                                 .initial = std::string(columns.Value(row, 3).value_or("")),
                                 .nullable = columns.Value(row, 4) == "YES"});
  }
  SchemaChanges changes;
  std::set<std::string_view> physical;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (!FirstPhysicalColumn(physical, field)) { continue; }
    const auto column = there.find(detail::ColumnName(field));
    if (column == there.end()) {
      into.Run("ALTER TABLE " + Quoted(table.name) + " ADD COLUMN " + detail::SqlColumn(field) +
               " " + ColumnType(field) + " NOT NULL DEFAULT " + ColumnDefault(field));
      ++changes.added;
    } else if (field.sqlTimestamp) {
      const auto &shape = column->second;
      if (shape.type != "bigint" || shape.nullable || shape.initial != ColumnDefault(field)) {
        throw Error("RowVersion: incompatible timestamp column; explicit migration required: " +
                    std::string(table.name));
      }
    } else if ((field.type == FieldType::Code || field.type == FieldType::Text) &&
               column->second.length != 0 && column->second.length < field.length) {
      into.Run("ALTER TABLE " + Quoted(table.name) + " ALTER COLUMN " + Quoted(field.name) +
               " TYPE " + ColumnType(field));
      ++changes.widened;
    }
  }
  return changes;
}

}

void ProvisionTable(const Connection &connection, const TableDef &table) {
  RequireTableProvider(table);
  if (detail::HasRowVersion(table)) { ProvisionRowVersions(connection); }
  const std::array<std::optional<std::string>, 1> named{Quoted(table.name)};
  const Result relation = connection.Execute("SELECT pg_catalog.to_regclass($1)", named);
  if (!relation.Value(0, 0).has_value()) {
    CreateStorageTable(connection, table);
    return;
  }
  static_cast<void>(EnsureColumns(connection, table));
  EnsureSequences(connection, table);
}

namespace {

void ProvisionSchema(const Connection &into) {
  const Result standing =
      into.Execute("SELECT tablename FROM pg_tables WHERE schemaname = 'public'");
  std::set<std::string> there;
  for (std::size_t row = 0; row < standing.Rows(); ++row) {
    const std::optional<std::string_view> name = standing.Value(row, 0);
    if (name.has_value()) { there.emplace(*name); }
  }
  std::size_t made = 0;
  std::size_t added = 0;
  std::size_t widened = 0;
  for (const TableEntry *entry : InstalledTables()) {
    if (!entry->table->providerRefusal.empty()) {
      std::println(
          "TABLE PROVIDER REFUSED: {}: {}", entry->table->name, entry->table->providerRefusal);
      continue;
    }
    static_cast<void>(detail::HasRowVersion(*entry->table));
    if (there.contains(std::string(entry->table->name))) {
      const auto changes = EnsureColumns(into, *entry->table);
      added += changes.added;
      widened += changes.widened;
      EnsureSequences(into, *entry->table);
      continue;
    }
    CreateStorageTable(into, *entry->table);
    ++made;
  }
  if (made != 0) { std::println("{} table(s) created in the runner's database", made); }
  if (added != 0) { std::println("{} column(s) added to tables the database already had", added); }
  if (widened != 0) {
    std::println("{} text/code column(s) widened to their declarations", widened);
  }
}

void ProvisionDates() {
  platform::Date anyPeriod;
  if (anyPeriod.FindFirst()) { return; }
  static constexpr int kFirstYear = 1980;
  static constexpr int kLastYear = 2079;
  static constexpr std::array<std::string_view, 7> kDays{
      "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
  static constexpr std::array<std::string_view, 12> kMonths{"January",
                                                            "February",
                                                            "March",
                                                            "April",
                                                            "May",
                                                            "June",
                                                            "July",
                                                            "August",
                                                            "September",
                                                            "October",
                                                            "November",
                                                            "December"};
  std::size_t periods = 0;
  const auto period = [&periods](platform::PeriodType type,
                                 ::agiru::Date start,
                                 ::agiru::Date end,
                                 ::agiru::Integer no,
                                 std::string_view name) {
    platform::Date row;
    row.PeriodType_ = type;
    row.PeriodStart = start;
    row.PeriodEnd = end.Closing();
    row.PeriodNo = no;
    row.PeriodName = name;
    row.PeriodInvariantName = name;
    row.Insert();
    ++periods;
  };
  const std::int32_t first = calendar::DaysFromCivil(kFirstYear, 1, 1);
  const std::int32_t last = calendar::DaysFromCivil(kLastYear, 12, 31);
  for (std::int32_t days = first; days <= last; ++days) {
    const calendar::Civil civil = calendar::CivilFromDays(days);
    const ::agiru::Date day = ::agiru::Date::FromYmd(civil.year, civil.month, civil.day);
    period(platform::PeriodType::Date, day, day, day.DayOfWeek(), kDays[day.DayOfWeek() - 1]);
    if (day.DayOfWeek() == 1) {
      const calendar::Civil sunday = calendar::CivilFromDays(days + 6);
      period(platform::PeriodType::Week,
             day,
             ::agiru::Date::FromYmd(sunday.year, sunday.month, sunday.day),
             day.WeekNo(),
             "Week " + std::to_string(day.WeekNo()));
    }
    if (civil.day == 1) {
      const std::int32_t next =
          calendar::DaysFromCivil(civil.month == 12 ? civil.year + 1 : civil.year,
                                  civil.month == 12 ? 1 : civil.month + 1,
                                  1);
      const calendar::Civil monthEnd = calendar::CivilFromDays(next - 1);
      period(platform::PeriodType::Month,
             day,
             ::agiru::Date::FromYmd(monthEnd.year, monthEnd.month, monthEnd.day),
             static_cast<::agiru::Integer>(civil.month),
             kMonths[civil.month - 1]);
      if (civil.month % 3 == 1) {
        const unsigned quarter = (civil.month - 1) / 3 + 1;
        const std::int32_t after = quarter == 4
                                       ? calendar::DaysFromCivil(civil.year + 1, 1, 1)
                                       : calendar::DaysFromCivil(civil.year, quarter * 3 + 1, 1);
        const calendar::Civil quarterEnd = calendar::CivilFromDays(after - 1);
        period(platform::PeriodType::Quarter,
               day,
               ::agiru::Date::FromYmd(quarterEnd.year, quarterEnd.month, quarterEnd.day),
               static_cast<::agiru::Integer>(quarter),
               "Quarter " + std::to_string(quarter));
      }
      if (civil.month == 1) {
        period(platform::PeriodType::Year,
               day,
               ::agiru::Date::FromYmd(civil.year, kDecember, kDecemberLastDay),
               civil.year,
               std::to_string(civil.year));
      }
    }
  }
  std::println("{} period(s) written into Date", periods);
}

}

void ProvisionInstalled(const Connection &into) {
  ProvisionNumberSequences(into);
  ProvisionRowVersions(into);
  ProvisionSchema(into);
  if (!Session::HasCurrent() || &Session::Current().Database() != &into) { return; }
  const std::string_view company = Session::Current().CompanyName();
  if (!company.empty()) {
    platform::Company standing;
    standing.SetRange(standing.Name, company);
    if (!standing.FindFirst()) {
      platform::Company row;
      row.Name = company;
      row.DisplayName = company;
      row.Insert();
      std::println("the company {} is written into Company", company);
    }
  }
  std::size_t profiles = 0;
  for (const ProfileDef *profile : InstalledProfiles()) {
    platform::AllProfile standing;
    standing.SetRange(standing.ProfileID, std::string_view(profile->profileId));
    if (standing.FindFirst()) { continue; }
    platform::AllProfile row;
    row.Scope = platform::PersonalizationScope::System;
    row.ProfileID = std::string_view(profile->profileId);
    row.Description = Fitted(profile->description, platform::AllProfile::kDescriptionLength);
    row.RoleCenterID = profile->roleCenter.Value();
    row.Caption = Fitted(profile->caption, platform::AllProfile::kCaptionLength);
    row.Enabled = profile->enabled;
    row.Promoted = profile->promoted;
    row.Insert();
    ++profiles;
  }
  if (profiles != 0) { std::println("{} profile(s) written into All Profile", profiles); }
  ProvisionDates();
  platform::AllObj anyObject;
  if (anyObject.FindFirst()) { return; }
  std::size_t objects = 0;
  const auto object = [&objects](platform::AllObjType type,
                                 std::int32_t id,
                                 std::string_view name,
                                 std::string_view caption) {
    platform::AllObj bare;
    bare.ObjectType = type;
    bare.ObjectID = id;
    bare.ObjectName = name;
    bare.Name = name;
    bare.Insert();
    platform::AllObjWithCaption captioned;
    captioned.ObjectType = type;
    captioned.ObjectID = id;
    captioned.ObjectName = name;
    captioned.Name = name;
    captioned.ObjectCaption = caption.empty() ? name : caption;
    captioned.Insert();
    ++objects;
  };
  for (const TableEntry *entry : InstalledTables()) {
    object(platform::AllObjType::Table,
           entry->table->id.Value(),
           entry->table->name,
           entry->table->caption);
  }
  for (const CodeunitEntry *entry : InstalledCodeunits()) {
    object(platform::AllObjType::Codeunit, entry->id.Value(), entry->name, entry->name);
  }
  for (const PageEntry *entry : InstalledPages()) {
    object(platform::AllObjType::Page,
           entry->page->id.Value(),
           entry->page->name,
           entry->page->caption);
  }
  if (objects != 0) { std::println("{} object(s) written into AllObj", objects); }
}

}
