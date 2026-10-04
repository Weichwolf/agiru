#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"

#include "Cursor.h"
#include "RecordOrder.h"
#include "Selection.h"
#include "Temporary.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
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
  std::string sql = "SELECT " + Columns(table) + " FROM " + made.from;
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  if (!made.order.empty()) { sql += " ORDER BY " + made.order; }
  return sql;
}

bool ReadInto(void *record, const TableDef &table, const Cursor &cursor) {
  std::size_t column = 0;
  for (const FieldDef &def : table.fields) {
    if (!Stored(def)) { continue; }
    const std::optional<std::string_view> value = cursor.Value(column);
    ++column;
    if (!value.has_value()) {
      throw Error("the column " + std::string(def.name) +
                  " came back null, and an AL field has no null");
    }
    SetFieldText(record, def, *value);
  }
  return true;
}

}

namespace {

std::string Reversed(const RecordOrder &by, bool descending) {
  std::string order;
  for (const RecordOrder::Column &column : by.Columns()) {
    if (!order.empty()) { order += ", "; }
    order += Quoted(column.field->name);
    if (descending == column.ascending) { order += " DESC"; }
  }
  return order;
}

std::string TuplePredicate(Selection &made,
                           std::span<const RecordOrder::Column> order,
                           const void *record,
                           std::string_view op) {
  std::string columns;
  std::string values;
  for (const RecordOrder::Column &column : order) {
    if (!columns.empty()) {
      columns += ", ";
      values += ", ";
    }
    columns += Quoted(column.field->name);
    made.binds.emplace_back(StorageText(record, *column.field));
    values += "$" + std::to_string(made.binds.size());
  }
  const std::string_view comparison =
      op == "=" || order.front().ascending ? op : (op == ">" ? "<" : ">");
  return "(" + columns + ") " + std::string(comparison) + " (" + values + ")";
}

std::string MixedPredicate(Selection &made,
                           std::span<const RecordOrder::Column> order,
                           const void *record,
                           std::string_view op) {
  std::string prefix;
  std::string predicate;
  for (const RecordOrder::Column &column : order) {
    const FieldDef &def = *column.field;
    const std::string name = Quoted(def.name);
    made.binds.emplace_back(StorageText(record, def));
    const std::string value = "$" + std::to_string(made.binds.size());
    if (!predicate.empty()) { predicate += " OR "; }
    const std::string_view comparison = column.ascending ? op : (op == ">" ? "<" : ">");
    predicate.append("(")
        .append(prefix)
        .append(name)
        .append(" ")
        .append(comparison)
        .append(" ")
        .append(value)
        .append(")");
    prefix.append(name).append(" = ").append(value).append(" AND ");
  }
  return predicate;
}

void Compare(Selection &made, const RecordOrder &by, const void *record, std::string_view op) {
  const auto order = by.Columns();
  if (order.empty()) { return; }
  const bool uniform = std::ranges::all_of(order, [&](const RecordOrder::Column &column) {
    return column.ascending == order.front().ascending;
  });
  const std::string predicate = op == "=" || uniform ? TuplePredicate(made, order, record, op)
                                                     : MixedPredicate(made, order, record, op);
  if (!made.where.empty()) { made.where += " AND "; }
  made.where += "(" + predicate + ")";
}

bool ReadOne(void *record, const TableDef &table, const Selection &made, const std::string &order) {
  std::string sql = "SELECT " + Columns(table) + " FROM " + made.from;
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  if (!order.empty()) { sql += " ORDER BY " + order; }
  sql += " LIMIT 1";
  const Result result = Session::Current().Database().Execute(sql, made.binds);
  if (result.Rows() == 0) { return false; }
  std::size_t column = 0;
  for (const FieldDef &def : table.fields) {
    if (!Stored(def)) { continue; }
    const std::optional<std::string_view> value = result.Value(0, column);
    ++column;
    if (!value.has_value()) {
      throw Error("the column " + std::string(def.name) +
                  " came back null, and an AL field has no null");
    }
    SetFieldText(record, def, *value);
  }
  return true;
}

}

bool RuntimeFind(void *record, const TableDef &table, std::string_view which) {
  if (TempOf(record) != nullptr) { return TempFind(record, table, which); }
  RequireTableProvider(table);

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
    const RecordOrder by(table, state->key, state->ascending);
    switch (step) {
      case '+': break;
      case '=': Compare(made, by, record, "="); break;
      case '>': Compare(made, by, record, ">"); break;
      case '<': Compare(made, by, record, "<"); break;
      default:
        throw Error("Record.Find: '" + std::string(1, step) +
                    "' is not one of the characters record-find-method.md declares");
    }
    const bool backwards = step == '+' || step == '<';
    if (ReadOne(record, table, made, Reversed(by, backwards))) {
      state->positioned = true;
      return true;
    }
  }
  return false;
}

bool RuntimeFindSet(void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempFindSet(record, table); }
  RequireTableProvider(table);

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
  if (steps == 0) { return 0; }
  if (TempOf(record) != nullptr) { return TempNext(record, table, steps); }
  RequireTableProvider(table);

  RecordState *state = StateOf(record);
  OpenCursor *open = state->open.Held();
  if (!state->positioned) { return 0; }
  const std::int32_t wanted = steps;
  if (wanted < 0 || open == nullptr) {
    const std::string_view direction = wanted < 0 ? "<" : ">";
    const std::int64_t count = wanted < 0 ? -std::int64_t{wanted} : wanted;
    std::int64_t moved = 0;
    for (std::int64_t taken = 0; taken < count; ++taken) {
      if (!RuntimeFind(record, table, direction)) {
        state->positioned = true;
        break;
      }
      ++moved;
    }
    return static_cast<std::int32_t>(wanted < 0 ? -moved : moved);
  }
  for (std::int32_t taken = 0; taken < wanted; ++taken) {
    if (!open->cursor.Step()) {
      if (taken != 0) { ReadInto(record, table, open->cursor); }
      return taken;
    }
    ++state->stepped;
  }
  ReadInto(record, table, open->cursor);
  return wanted;
}

std::int32_t RuntimeCount(const void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempCount(const_cast<void *>(record), table); }

  const Selection made = Select(PeekOf(record), table);
  std::string sql = "SELECT count(*) FROM " + made.from;
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  const Result result = Session::Current().Database().Execute(sql, made.binds);
  if (result.Rows() == 0) { return 0; }
  const std::optional<std::string_view> value = result.Value(0, 0);
  return value.has_value() ? static_cast<std::int32_t>(std::stoll(std::string(*value))) : 0;
}

bool RuntimeIsEmpty(const void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempIsEmpty(const_cast<void *>(record), table); }

  const Selection made = Select(PeekOf(record), table);
  std::string sql = "SELECT 1 FROM " + made.from;
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  sql += " LIMIT 1";
  return Session::Current().Database().Execute(sql, made.binds).Rows() == 0;
}

std::int32_t RuntimeDeleteAll(const void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempDeleteAll(const_cast<void *>(record), table); }

  const Selection made = Select(PeekOf(record), table);
  std::string sql = "DELETE FROM " + Name(table);
  if (!made.where.empty()) { sql += " WHERE " + made.where; }
  Session::Current().Database().Run(sql, made.binds);
  return 0;
}

}
