#include "Selection.h"

#include "Filter.h"
#include "Where.h"

#include "runtime/Error.h"

#include <string>

namespace agiru::detail {

namespace {

std::string Quoted(std::string_view identifier) {
  std::string out = "\"";
  for (const char c : identifier) {
    if (c == '"') { out += "\"\""; } else { out += c; }
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
  if (state != nullptr) {
    for (const FieldFilter &filter : state->filters) {
      const FieldDef &def = FieldOf(table, filter.field);
      const Clause clause = Where(def, ParseFilter(filter.text), made.binds.size() + 1);
      if (clause.sql.empty()) { continue; }
      if (!made.where.empty()) { made.where += " AND "; }
      made.where += clause.sql;
      for (const std::optional<std::string> &bind : clause.binds) { made.binds.push_back(bind); }
    }
  }
  const std::vector<FieldNo> ordered = [&] {
    if (state != nullptr && !state->key.empty()) {
      std::vector<FieldNo> named;
      for (const SortField &one : state->key) { named.push_back(one.field); }
      return named;
    }
    return table.keys.empty() ? std::vector<FieldNo>{}
                              : std::vector<FieldNo>(table.keys[0].fields.begin(),
                                                     table.keys[0].fields.end());
  }();
  const bool ascending = state == nullptr || state->ascending;
  for (const FieldNo no : ordered) {
    if (!made.order.empty()) { made.order += ", "; }
    made.order += Quoted(FieldOf(table, no).name);
    if (!ascending) { made.order += " DESC"; }
  }
  return made;
}

}
