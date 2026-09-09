#include "runtime/Storage.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/Error.h"

#include "Rows.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <print>
#include <set>
#include <span>
#include <string>
#include <string_view>

namespace agiru {

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
    out += Quoted(table.fields[IndexOf(table, no)].name) + " = " + Placeholder(n);
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

namespace {

std::string ColumnZero(const FieldDef &def) {
  switch (def.type) {
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

void CreateTable(const Connection &connection, const TableDef &table) {
  std::string sql = "CREATE TABLE " + Quoted(table.name) + " (";
  bool written = false;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (written) { sql += ", "; }
    written = true;
    sql += Quoted(field.name) + " " + ColumnType(field) + " NOT NULL DEFAULT " + ColumnZero(field);
  }
  if (!table.keys.empty()) {
    sql += ", PRIMARY KEY (";
    bool first = true;
    for (const FieldNo no : table.keys[0].fields) {
      if (!first) { sql += ", "; }
      first = false;
      sql += Quoted(table.fields[IndexOf(table, no)].name);
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
      index += Quoted(table.fields[IndexOf(table, no)].name);
    }
    index += ")";
    connection.Run(index);
  }
}

void DropTable(const Connection &connection, const TableDef &table) {
  connection.Run("DROP TABLE IF EXISTS " + Quoted(table.name));
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
    columns += Quoted(field.name);
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

bool InsertRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values) {
  if (values.size() != StoredCount(table)) {
    throw Error("Insert: the value count does not match the declaration");
  }
  const std::string columns = StoredColumns(table);
  std::string placeholders;
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i != 0) { placeholders += ", "; }
    placeholders += Placeholder(i + 1);
  }
  const Result written =
      connection.Execute("INSERT INTO " + Quoted(table.name) + " (" + columns + ") VALUES (" +
                             placeholders + ") ON CONFLICT DO NOTHING",
                         values);
  return written.Affected() == 1;
}

std::optional<FieldValues> GetRow(const Connection &connection,
                                  const TableDef &table,
                                  std::span<const std::optional<std::string>> key) {
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
  const std::string columns = StoredColumns(table);
  const std::optional<std::string> bound{std::string(value)};
  const Result result =
      connection.Execute("SELECT " + columns + " FROM " + Quoted(table.name) + " WHERE " +
                             Quoted(column.name) + " = " + Placeholder(1),
                         std::span<const std::optional<std::string>>(&bound, 1));
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

bool ModifyRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values) {
  if (values.size() != StoredCount(table)) {
    throw Error("Modify: the value count does not match the declaration");
  }
  std::string assignments;
  std::size_t at = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (at != 0) { assignments += ", "; }
    assignments += Quoted(field.name) + " = " + Placeholder(at + 1);
    ++at;
  }

  FieldValues bound(values.begin(), values.end());
  for (const FieldNo no : table.keys[0].fields) {
    bound.push_back(values[StoredIndexOf(table, no)]);
  }

  const Result result =
      connection.Execute("UPDATE " + Quoted(table.name) + " SET " + assignments + " WHERE " +
                             KeyPredicate(table, values.size() + 1) + " RETURNING 1",
                         bound);
  return result.Rows() != 0;
}

bool RenameRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values,
               std::span<const std::optional<std::string>> oldKey) {
  if (values.size() != StoredCount(table)) {
    throw Error("Rename: the value count does not match the declaration");
  }
  std::string assignments;
  std::size_t at = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (at != 0) { assignments += ", "; }
    assignments += Quoted(field.name) + " = " + Placeholder(at + 1);
    ++at;
  }
  FieldValues bound(values.begin(), values.end());
  bound.insert(bound.end(), oldKey.begin(), oldKey.end());
  const Result result =
      connection.Execute("UPDATE " + Quoted(table.name) + " SET " + assignments + " WHERE " +
                             KeyPredicate(table, values.size() + 1) + " RETURNING 1",
                         bound);
  return result.Rows() != 0;
}

bool DeleteRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> key) {
  const Result result = connection.Execute("DELETE FROM " + Quoted(table.name) + " WHERE " +
                                               KeyPredicate(table, 1) + " RETURNING 1",
                                           key);
  return result.Rows() != 0;
}

std::string_view Required(const std::optional<std::string> &value, const FieldDef &def) {
  if (!value.has_value()) {
    throw Error("the column for field '" + std::string(def.name) + "' is null");
  }
  return *value;
}

void ProvisionInstalled(const Connection &into) {
  const Result standing =
      into.Execute("SELECT tablename FROM pg_tables WHERE schemaname = 'public'");
  std::set<std::string> there;
  for (std::size_t row = 0; row < standing.Rows(); ++row) {
    const std::optional<std::string_view> name = standing.Value(row, 0);
    if (name.has_value()) { there.emplace(*name); }
  }
  std::size_t made = 0;
  for (const TableEntry *entry : InstalledTables()) {
    if (there.contains(std::string(entry->table->name))) { continue; }
    CreateTable(into, *entry->table);
    ++made;
  }
  if (made != 0) { std::println("{} table(s) created in the runner's database", made); }
}

}
