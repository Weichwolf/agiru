#include "Selection.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"

#include "Filter.h"
#include "Where.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

namespace {

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
    return named;
  }
  if (table.keys.empty()) { return named; }
  named.assign(table.keys[0].fields.begin(), table.keys[0].fields.end());
  return named;
}

void Narrow(Selection &made, const RecordState *state, const TableDef &table) {
  if (state == nullptr) { return; }
  for (const FieldFilter &filter : state->filters) {
    const Clause clause =
        Where(FieldOf(table, filter.field), ParseFilter(filter.text), made.binds.size() + 1);
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
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (i != 0) { columns += ", "; }
    columns += Quoted(table.fields[i].name);
  }
  return columns;
}

Selection Select(const RecordState *state, const TableDef &table) {
  Selection made;
  Narrow(made, state, table);
  const bool ascending = state == nullptr || state->ascending;
  for (const FieldNo no : OrderedBy(state, table)) {
    if (!made.order.empty()) { made.order += ", "; }
    made.order += Quoted(FieldOf(table, no).name);
    if (!ascending) { made.order += " DESC"; }
  }
  return made;
}

}
