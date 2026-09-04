#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Table.h"

#include "Cursor.h"
#include "Rows.h"
#include "Selection.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace agiru::detail {

struct OpenCursor {
  Cursor cursor;
};

void Close(OpenCursor *open) {
  delete open;
}

namespace {

RecordState *StateOf(void *record) {
  return &reinterpret_cast<StateHandle *>(record)->Ensure();
}

const RecordState *PeekOf(const void *record) {
  return reinterpret_cast<const StateHandle *>(record)->Peek();
}

std::string SelectFrom(const Selection &made, const TableDef &table) {
  std::string sql = "SELECT " + Columns(table) + " FROM " + Name(table);
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  if (!made.order.empty()) { sql += " ORDER BY " + made.order; }
  return sql;
}

bool ReadInto(void *record, const TableDef &table, const Cursor &cursor) {
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    const std::optional<std::string_view> value = cursor.Value(i);
    if (!value.has_value()) {
      throw Error("the column " + std::string(table.fields[i].name) +
                  " came back null, and an AL "
                  "field has no null");
    }
    SetFieldText(record, table.fields[i], *value);
  }
  return true;
}

}

namespace {

const FieldDef &Sorted(const TableDef &table, FieldNo no) {
  for (const FieldDef &def : table.fields) {
    if (def.no == no) { return def; }
  }
  throw Error("the sort path names a field the table lacks");
}

std::string Reversed(const Selection &made, const TableDef &table, bool descending) {
  std::string order;
  for (const FieldNo no : made.sorted) {
    if (!order.empty()) { order += ", "; }
    order += Quoted(Sorted(table, no).name);
    if (descending) { order += " DESC"; }
  }
  return order;
}

void Compare(Selection &made, const TableDef &table, const void *record, std::string_view op) {
  std::string columns;
  std::string values;
  for (const FieldNo no : made.sorted) {
    if (!columns.empty()) {
      columns += ", ";
      values += ", ";
    }
    const FieldDef &def = Sorted(table, no);
    columns += Quoted(def.name);
    made.binds.emplace_back(StorageText(record, def));
    values += "$" + std::to_string(made.binds.size());
  }
  if (columns.empty()) { return; }
  if (!made.where.empty()) { made.where += " AND "; }
  made.where += "(" + columns + ") " + std::string(op) + " (" + values + ")";
}

bool ReadOne(void *record, const TableDef &table, const Selection &made, const std::string &order) {
  std::string sql = "SELECT " + Columns(table) + " FROM " + Name(table);
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  if (!order.empty()) { sql += " ORDER BY " + order; }
  sql += " LIMIT 1";
  const Result result = Session::Current().Database().Execute(sql, made.binds);
  if (result.Rows() == 0) { return false; }
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    const std::optional<std::string_view> value = result.Value(0, i);
    if (!value.has_value()) {
      throw Error("the column " + std::string(table.fields[i].name) +
                  " came back null, and an AL field has no null");
    }
    SetFieldText(record, table.fields[i], *value);
  }
  return true;
}

}

bool RuntimeFind(void *record, const TableDef &table, std::string_view which) {
  RecordState *state = StateOf(record);
  if (which.empty()) { which = "="; }
  for (const char step : which) {
    if ((step == '-' || step == '+') && which.size() != 1) {
      throw Error("Record.Find: '-' and '+' can only be used alone, and this one reads \"" +
                  std::string(which) + "\"");
    }
    if (step == '-') { return RuntimeFindSet(record, table); }
    state->open.Forget();
    state->positioned = false;
    Selection made = Select(state, table);
    switch (step) {
      case '+': break;
      case '=': Compare(made, table, record, "="); break;
      case '>': Compare(made, table, record, ">"); break;
      case '<': Compare(made, table, record, "<"); break;
      default:
        throw Error("Record.Find: '" + std::string(1, step) +
                    "' is not one of the characters record-find-method.md declares");
    }
    const bool backwards = step == '+' || step == '<';
    if (ReadOne(record, table, made, Reversed(made, table, backwards))) { return true; }
  }
  return false;
}

bool RuntimeFindSet(void *record, const TableDef &table) {
  RecordState *state = StateOf(record);
  state->open.Forget();
  state->stepped = 0;
  state->positioned = false;
  const Connection &connection = Session::Current().Database();
  if (!connection.InTransaction()) { connection.Run("BEGIN"); }
  const Selection made = Select(state, table);
  auto *open = new OpenCursor{Cursor(connection, SelectFrom(made, table), made.binds)};
  if (!open->cursor.Step()) {
    Close(open);
    return false;
  }
  ReadInto(record, table, open->cursor);
  state->open.Hold(open);
  state->positioned = true;
  return true;
}

std::int32_t RuntimeNext(void *record, const TableDef &table, std::int32_t steps) {
  RecordState *state = StateOf(record);
  OpenCursor *open = state->open.Held();
  if (open == nullptr || !state->positioned) { return 0; }
  if (steps < 0) {
    throw Error("Record.Next with a negative step needs a scrollable cursor, which this one is not "
                "(board:0044)");
  }
  const std::int32_t wanted = steps == 0 ? 1 : steps;
  for (std::int32_t taken = 0; taken < wanted; ++taken) {
    if (!open->cursor.Step()) {
      state->positioned = false;
      return taken;
    }
  }
  ReadInto(record, table, open->cursor);
  ++state->stepped;
  return wanted;
}

std::int32_t RuntimeCount(const void *record, const TableDef &table) {
  const Selection made = Select(PeekOf(record), table);
  std::string sql = "SELECT count(*) FROM " + Name(table);
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  const Result result = Session::Current().Database().Execute(sql, made.binds);
  if (result.Rows() == 0) { return 0; }
  const std::optional<std::string_view> value = result.Value(0, 0);
  return value.has_value() ? static_cast<std::int32_t>(std::stoll(std::string(*value))) : 0;
}

bool RuntimeIsEmpty(const void *record, const TableDef &table) {
  const Selection made = Select(PeekOf(record), table);
  std::string sql = "SELECT 1 FROM " + Name(table);
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  sql += " LIMIT 1";
  return Session::Current().Database().Execute(sql, made.binds).Rows() == 0;
}

std::int32_t RuntimeDeleteAll(const void *record, const TableDef &table) {
  const Selection made = Select(PeekOf(record), table);
  std::string sql = "DELETE FROM " + Name(table);
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  Session::Current().Database().Run(sql, made.binds);
  return 0;
}

}
