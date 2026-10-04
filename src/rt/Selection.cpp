#include "Selection.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordState.h"
#include "runtime/Storage.h"
#include "type/FieldClass.h"

#include "Filter.h"
#include "RecordOrder.h"
#include "Where.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

std::string Quoted(std::string_view identifier) {
  std::string out = "\"";
  for (const char c : identifier) {
    if (c == '"') {
      out += "\"\"";
    } else {
      out += c;
    }
  }
  out += '"';
  return out;
}

namespace {

const FieldDef &FieldOf(const TableDef &table, FieldNo no) {
  for (const FieldDef &def : table.fields) {
    if (def.no == no) { return def; }
  }
  throw Error("the table carries no field " + std::to_string(no.Value()));
}

constexpr Interval kSeriesDomain{.low = -1000000000, .high = 1000000000};

constexpr std::int64_t kSeriesLimit = 1000000;

Intervals Both(const Intervals &left, const Intervals &right) {
  Intervals both;
  for (const Interval &one : left) {
    for (const Interval &other : right) {
      const std::int64_t low = std::max(one.low, other.low);
      const std::int64_t high = std::min(one.high, other.high);
      if (low <= high) { both.push_back(Interval{.low = low, .high = high}); }
    }
  }
  return both;
}

std::string SeriesColumns(const TableDef &table, const FieldDef &number) {
  std::string columns = "g::int AS " + Quoted(number.name);
  for (const auto &field : table.fields) {
    if (!Stored(field) || field.no == number.no) { continue; }
    const auto *const system = std::ranges::find_if(kSystemFields, [&](const auto &declared) {
      return declared.no == field.no &&
             ((declared.alType == "Guid" && field.type == FieldType::Guid) ||
              (declared.alType == "DateTime" && field.type == FieldType::DateTime));
    });
    if (system == kSystemFields.end()) {
      throw Error("sequence provider cannot synthesize field " + std::string(field.name));
    }
    columns += ", " + ColumnZero(field) + "::" + ColumnType(field) + " AS " + Quoted(field.name);
  }
  return columns;
}

std::string Series(const RecordState *state, const TableDef &table) {
  const FieldDef &field = FieldOf(table, table.sequenceField);
  Intervals admitted{{kSeriesDomain}};
  if (state == nullptr) { return {}; }
  for (const FieldFilter &filter : state->filters) {
    if (filter.field != table.sequenceField) { continue; }
    const std::optional<Intervals> one = IntegerIntervals(ParseFilter(filter.text), kSeriesDomain);
    if (!one.has_value()) { return {}; }
    admitted = Both(admitted, *one);
  }
  if (admitted.empty()) { return {}; }
  Intervals capped;
  std::int64_t left = kSeriesLimit;
  for (Interval one : admitted) {
    if (left <= 0) { break; }
    if (one.high - one.low + 1 > left) { one.high = one.low + left - 1; }
    left -= one.high - one.low + 1;
    capped.push_back(one);
  }
  admitted = capped;
  const auto columns = SeriesColumns(table, field);
  std::string series;
  for (const Interval &one : admitted) {
    if (!series.empty()) { series += " UNION ALL "; }
    series += "SELECT " + columns + " FROM generate_series(" + std::to_string(one.low) + ", " +
              std::to_string(one.high) + ") AS g";
  }
  return "(" + series + ") AS " + Quoted(table.name);
}

void Narrow(Selection &made, const RecordState *state, const TableDef &table) {
  if (state == nullptr) { return; }
  std::string crossColumn;
  const auto take = [&made, &crossColumn](const std::string &sql, int group) {
    std::string &into = group == kCrossColumnGroup ? crossColumn : made.where;
    if (!into.empty()) { into += group == kCrossColumnGroup ? " OR " : " AND "; }
    into += sql;
  };
  for (const FieldFilter &filter : state->filters) {
    const FieldDef &field = FieldOf(table, filter.field);
    if (field.fieldClass == FieldClass::FlowFilter) { continue; }
    if (field.fieldClass == FieldClass::FlowField) {
      const Clause column = FlowFieldColumn(table, field, state, made.binds.size() + 1);
      if (column.sql.empty()) { continue; }
      const Clause clause = Where(
          field, ParseFilter(filter.text), made.binds.size() + 1 + column.binds.size(), column.sql);
      if (clause.sql.empty()) { continue; }
      take(clause.sql, filter.group);
      made.binds.insert(made.binds.end(), column.binds.begin(), column.binds.end());
      made.binds.insert(made.binds.end(), clause.binds.begin(), clause.binds.end());
      continue;
    }
    const Clause clause = Where(field, ParseFilter(filter.text), made.binds.size() + 1);
    if (clause.sql.empty()) { continue; }
    take(clause.sql, filter.group);
    made.binds.insert(made.binds.end(), clause.binds.begin(), clause.binds.end());
  }
  if (crossColumn.empty()) { return; }
  if (!made.where.empty()) { made.where += " AND "; }
  made.where += "(" + crossColumn + ")";
}

std::string MarkClause(Selection &made,
                       const TableDef &table,
                       std::span<const FieldNo> key,
                       const std::string &mark) {
  std::vector<std::string> values;
  std::size_t start = 0;
  for (std::size_t at = mark.find('\x1f'); at != std::string::npos; at = mark.find('\x1f', start)) {
    values.push_back(mark.substr(start, at - start));
    start = at + 1;
  }
  values.push_back(mark.substr(start));
  if (values.size() != key.size()) { return {}; }
  std::string one;
  for (std::size_t i = 0; i < key.size(); ++i) {
    Atom atom;
    atom.value = values[i];
    const Clause clause =
        Where(FieldOf(table, key[i]), Expression{All{atom}}, made.binds.size() + 1);
    if (clause.sql.empty()) { continue; }
    if (!one.empty()) { one += " AND "; }
    one += clause.sql;
    made.binds.insert(made.binds.end(), clause.binds.begin(), clause.binds.end());
  }
  return one;
}

void NarrowMarks(Selection &made, const RecordState *state, const TableDef &table) {
  if (state == nullptr || !state->markedOnly) { return; }
  std::string marked;
  const std::span<const FieldNo> key =
      table.keys.empty() ? std::span<const FieldNo>{} : table.keys[0].fields;
  for (const std::string &mark : state->marks) {
    const std::string one = MarkClause(made, table, key, mark);
    if (one.empty()) { continue; }
    if (!marked.empty()) { marked += " OR "; }
    marked += "(" + one + ")";
  }
  if (!made.where.empty()) { made.where += " AND "; }
  made.where += marked.empty() ? std::string("FALSE") : "(" + marked + ")";
}

}

std::string Name(const TableDef &table) {
  RequireTableProvider(table);
  return Quoted(table.name);
}

std::string Columns(const TableDef &table) {
  std::string columns;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (!columns.empty()) { columns += ", "; }
    columns += Quoted(field.name);
  }
  return columns;
}

Selection Select(const RecordState *state, const TableDef &table) {
  Selection made;
  made.from = Name(table);
  if (table.sequenceField.Value() != 0) {
    const std::string series = Series(state, table);
    if (!series.empty()) { made.from = series; }
  }
  Narrow(made, state, table);
  NarrowMarks(made, state, table);
  const RecordOrder order(table,
                          state == nullptr ? std::span<const SortField>{} : state->key,
                          state == nullptr || state->ascending);
  for (const RecordOrder::Column &column : order.Columns()) {
    made.sorted.push_back(column.field->no);
    if (!made.order.empty()) { made.order += ", "; }
    made.order += Quoted(column.field->name);
    if (!column.ascending) { made.order += " DESC"; }
  }
  return made;
}

}
