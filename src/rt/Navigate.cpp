#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"

#include "Cursor.h"
#include "FieldMetadata.h"
#include "RecordChanges.h"
#include "RecordOrder.h"
#include "Selection.h"
#include "SqlColumn.h"
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
  RecordRead read;
  bool backwards;
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

OpenCursor *
OpenSelection(RecordState &state, const TableDef &table, const Selection &made, bool backwards) {
  state.open.Forget();
  const Connection &connection = Session::Current().Database();
  if (!connection.InTransaction()) { connection.Run("BEGIN"); }
  auto *open = new OpenCursor{.cursor = Cursor(connection, SelectFrom(made, table), made.binds),
                              .read = RecordRead(table.id),
                              .backwards = backwards};
  state.open.Hold(open);
  return open;
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
    order += SqlColumn(*column.field);
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
    columns += SqlColumn(*column.field);
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
    const std::string name = SqlColumn(def);
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
  if (const auto found = FindInstalledFields(record, table, which); found.has_value()) {
    return *found;
  }
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
  if (const auto found = FindInstalledFields(record, table, "-"); found.has_value()) {
    return *found;
  }
  RequireTableProvider(table);

  RecordState *state = StateOf(record);
  state->open.Forget();
  state->stepped = 0;
  state->positioned = false;
  const Selection made = Select(state, table);
  OpenCursor *open = OpenSelection(*state, table, made, false);
  if (!open->cursor.Step()) {
    state->open.Forget();
    return false;
  }
  ReadInto(record, table, open->cursor);
  state->positioned = true;
  return true;
}

std::int32_t RuntimeNext(void *record, const TableDef &table, std::int32_t steps) {
  if (steps == 0) { return 0; }
  if (TempOf(record) != nullptr) { return TempNext(record, table, steps); }
  if (const auto moved = NextInstalledField(record, table, steps); moved.has_value()) {
    return *moved;
  }
  RequireTableProvider(table);

  RecordState *state = StateOf(record);
  OpenCursor *open = state->open.Held();
  if (!state->positioned) { return 0; }
  if (open != nullptr && (!open->cursor.Current() || !open->read.Current())) {
    state->open.Forget();
    open = nullptr;
  }
  const bool backwards = steps < 0;
  if (open == nullptr || open->backwards != backwards) {
    Selection made = Select(state, table);
    const RecordOrder by(table, state->key, state->ascending);
    Compare(made, by, record, backwards ? "<" : ">");
    made.order = Reversed(by, backwards);
    open = OpenSelection(*state, table, made, backwards);
  }
  const std::int64_t count = backwards ? -std::int64_t{steps} : steps;
  std::int64_t moved = 0;
  while (moved < count && open->cursor.Step()) {
    ++moved;
    ++state->stepped;
  }
  if (moved != 0) { ReadInto(record, table, open->cursor); }
  return static_cast<std::int32_t>(backwards ? -moved : moved);
}

std::int32_t RuntimeCount(const void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempCount(const_cast<void *>(record), table); }
  if (const auto count = CountInstalledFields(record, table); count.has_value()) { return *count; }

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
  if (const auto count = CountInstalledFields(record, table, true); count.has_value()) {
    return *count == 0;
  }

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
  const Connection &connection = Session::Current().Database();
  const Result written = connection.Execute(sql, made.binds);
  if (written.Affected() != 0) { RecordWritten(connection, table.id); }
  return 0;
}

}
