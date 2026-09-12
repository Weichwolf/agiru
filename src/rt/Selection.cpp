#include "Selection.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"

#include "Filter.h"
#include "Where.h"

#include <algorithm>
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

std::vector<FieldNo> OrderedBy(const RecordState *state, const TableDef &table) {
  std::vector<FieldNo> named;
  if (state != nullptr && !state->key.empty()) {
    named.reserve(state->key.size());
    for (const SortField &one : state->key) { named.push_back(one.field); }
  }
  if (table.keys.empty()) { return named; }
  for (const FieldNo no : table.keys[0].fields) {
    if (std::ranges::find(named, no) == named.end()) { named.push_back(no); }
  }
  return named;
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
  if (admitted.empty() || CountOf(admitted) > kSeriesLimit) { return {}; }
  for (const Interval &one : admitted) {
    if (one.low == kSeriesDomain.low || one.high == kSeriesDomain.high) { return {}; }
  }
  std::string series;
  for (const Interval &one : admitted) {
    if (!series.empty()) { series += " UNION ALL "; }
    series += "SELECT g::int AS " + Quoted(field.name) + " FROM generate_series(" +
              std::to_string(one.low) + ", " + std::to_string(one.high) + ") AS g";
  }
  return "(" + series + ") AS " + Quoted(table.name);
}

void Narrow(Selection &made, const RecordState *state, const TableDef &table) {
  if (state == nullptr) { return; }
  for (const FieldFilter &filter : state->filters) {
    const FieldDef &field = FieldOf(table, filter.field);
    if (field.fieldClass == FieldClass::FlowFilter) { continue; }
    if (field.fieldClass == FieldClass::FlowField) {
      const Clause column = FlowFieldColumn(table, field, state, made.binds.size() + 1);
      if (column.sql.empty()) { continue; }
      const Clause clause = Where(
          field, ParseFilter(filter.text), made.binds.size() + 1 + column.binds.size(), column.sql);
      if (clause.sql.empty()) { continue; }
      if (!made.where.empty()) { made.where += " AND "; }
      made.where += clause.sql;
      made.binds.insert(made.binds.end(), column.binds.begin(), column.binds.end());
      made.binds.insert(made.binds.end(), clause.binds.begin(), clause.binds.end());
      continue;
    }
    const Clause clause = Where(field, ParseFilter(filter.text), made.binds.size() + 1);
    if (clause.sql.empty()) { continue; }
    if (!made.where.empty()) { made.where += " AND "; }
    made.where += clause.sql;
    made.binds.insert(made.binds.end(), clause.binds.begin(), clause.binds.end());
  }
}

}

std::string Name(const TableDef &table) {
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

bool Ascends(const RecordState *state, FieldNo no) {
  if (state == nullptr) { return true; }
  bool field = true;
  for (const SortField &one : state->key) {
    if (one.field == no) { field = one.ascending; }
  }
  return state->ascending == field;
}

Selection Select(const RecordState *state, const TableDef &table) {
  Selection made;
  made.from = Name(table);
  if (table.sequenceField.Value() != 0) {
    const std::string series = Series(state, table);
    if (!series.empty()) { made.from = series; }
  }
  Narrow(made, state, table);
  if (state != nullptr && state->markedOnly) {
    std::string marked;
    const std::span<const FieldNo> key =
        table.keys.empty() ? std::span<const FieldNo>{} : table.keys[0].fields;
    for (const std::string &mark : state->marks) {
      std::vector<std::string> values;
      std::size_t start = 0;
      for (std::size_t at = mark.find('\x1f'); at != std::string::npos; at = mark.find('\x1f', start)) {
        values.push_back(mark.substr(start, at - start));
        start = at + 1;
      }
      values.push_back(mark.substr(start));
      if (values.size() != key.size()) { continue; }
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
      if (one.empty()) { continue; }
      if (!marked.empty()) { marked += " OR "; }
      marked += "(" + one + ")";
    }
    if (!made.where.empty()) { made.where += " AND "; }
    made.where += marked.empty() ? std::string("FALSE") : "(" + marked + ")";
  }
  made.sorted = OrderedBy(state, table);
  for (const FieldNo no : made.sorted) {
    if (!made.order.empty()) { made.order += ", "; }
    made.order += Quoted(FieldOf(table, no).name);
    if (!Ascends(state, no)) { made.order += " DESC"; }
  }
  return made;
}

}
