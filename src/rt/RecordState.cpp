#include "runtime/RecordState.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <algorithm>
#include <string>
#include <vector>

namespace agiru::detail {

void Narrow(RecordState &state, ::agiru::FieldNo field, const std::string &text) {
  const auto same = [field, &state](const FieldFilter &one) {
    return one.field == field && one.group == state.group;
  };
  std::erase_if(state.filters, same);
  if (text.empty()) { return; }
  state.filters.push_back(FieldFilter{.field = field, .group = state.group, .text = text});
}

std::string Literally(const std::string &value) {
  if (value.find_first_of("..|&<>=*?@'()") == std::string::npos) { return value; }
  std::string out = "'";
  for (const char c : value) {
    if (c == '\'') { out += '\''; }
    out += c;
  }
  out += '\'';
  return out;
}

bool KeyMatches(const TableDef &table, const std::vector<SortField> &key) {
  if (key.empty()) { return true; }
  for (const KeyDef &declared : table.keys) {
    if (declared.fields.size() < key.size()) { continue; }
    bool prefix = true;
    for (std::size_t i = 0; i < key.size(); ++i) {
      if (declared.fields[i] != key[i].field) {
        prefix = false;
        break;
      }
    }
    if (prefix) { return true; }
  }
  return false;
}

}
