#include "runtime/Storage.h"

#include "Rows.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/Error.h"

#include <cstddef>
#include <optional>
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

} // namespace

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
    // NOT `date`, AND THE DOCUMENTATION IS WHY. `date-data-type.md`: "For date fields, Business
    // Central uses only the date and uses a constant value for the time. For a normal date, this
    // constant value contains 00:00:00:000. For a closing date, it contains 23:59:59:000." A `date`
    // column has nowhere to put that, and a closing date would collapse onto its normal date --
    // which is the whole ordering a fiscal-year close depends on. The undefined date is "the
    // earliest valid date in SQL Server", 1753-01-01 00:00:00:000.
    case FieldType::Date: return "timestamp";
    case FieldType::Time: return "time";
    case FieldType::DateTime: return "timestamp";
    case FieldType::Guid: return "uuid";
    case FieldType::Blob: return "bytea";
  }
  throw Error("ColumnType: no SQL type for this field type yet");
}

void CreateTable(const Connection &connection, const TableDef &table) {
  std::string sql = "CREATE TABLE " + Quoted(table.name) + " (";
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (i != 0) { sql += ", "; }
    sql += Quoted(table.fields[i].name) + " " + ColumnType(table.fields[i]) + " NOT NULL";
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
}

void DropTable(const Connection &connection, const TableDef &table) {
  connection.Run("DROP TABLE IF EXISTS " + Quoted(table.name));
}

void InsertRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values) {
  if (values.size() != table.fields.size()) {
    throw Error("Insert: the value count does not match the declaration");
  }
  std::string columns;
  std::string placeholders;
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (i != 0) {
      columns += ", ";
      placeholders += ", ";
    }
    columns += Quoted(table.fields[i].name);
    placeholders += Placeholder(i + 1);
  }
  connection.Run("INSERT INTO " + Quoted(table.name) + " (" + columns + ") VALUES (" +
                     placeholders + ")",
                 values);
}

std::optional<FieldValues> GetRow(const Connection &connection,
                                  const TableDef &table,
                                  std::span<const std::optional<std::string>> key) {
  std::string columns;
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (i != 0) { columns += ", "; }
    columns += Quoted(table.fields[i].name);
  }
  const Result result = connection.Execute("SELECT " + columns + " FROM " + Quoted(table.name) +
                                               " WHERE " + KeyPredicate(table, 1),
                                           key);
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

bool ModifyRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values) {
  if (values.size() != table.fields.size()) {
    throw Error("Modify: the value count does not match the declaration");
  }
  std::string assignments;
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (i != 0) { assignments += ", "; }
    assignments += Quoted(table.fields[i].name) + " = " + Placeholder(i + 1);
  }

  FieldValues bound(values.begin(), values.end());
  for (const FieldNo no : table.keys[0].fields) { bound.push_back(values[IndexOf(table, no)]); }

  const Result result =
      connection.Execute("UPDATE " + Quoted(table.name) + " SET " + assignments + " WHERE " +
                             KeyPredicate(table, table.fields.size() + 1) + " RETURNING 1",
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

} // namespace agiru
