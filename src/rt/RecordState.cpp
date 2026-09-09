#include "runtime/RecordState.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"

#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

namespace {

bool SameText(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

std::string_view Trimmed(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
    text.remove_prefix(1);
  }
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
    text.remove_suffix(1);
  }
  return text;
}

std::string_view Unquoted(std::string_view text) {
  text = Trimmed(text);
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    return text.substr(1, text.size() - 2);
  }
  return text;
}

const FieldDef &FieldOfView(const TableDef &table, std::string_view spelled) {
  const std::string_view wanted = Unquoted(spelled);
  for (const FieldDef &field : table.fields) {
    if (SameText(field.name, wanted) || SameText(field.caption, wanted)) { return field; }
  }
  if (wanted.starts_with("Field") || wanted.starts_with("field")) {
    const std::string_view digits = wanted.substr(5);
    if (!digits.empty() && std::isdigit(static_cast<unsigned char>(digits.front())) != 0) {
      const std::int32_t no = std::stoi(std::string(digits));
      for (const FieldDef &field : table.fields) {
        if (field.no.Value() == no) { return field; }
      }
    }
  }
  throw Error("The view names a field " + std::string(wanted) + " that " + std::string(table.name) +
              " does not have");
}

std::string ViewName(const FieldDef &field, bool useNames) {
  if (!useNames) { return "Field" + std::to_string(field.no.Value()); }
  return std::string(field.caption.empty() ? field.name : field.caption);
}

std::vector<std::string_view> SplitOutsideParentheses(std::string_view text, char separator) {
  std::vector<std::string_view> parts;
  int depth = 0;
  std::size_t start = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '(') { ++depth; }
    if (text[i] == ')') { --depth; }
    if (text[i] == separator && depth == 0) {
      parts.push_back(text.substr(start, i - start));
      start = i + 1;
    }
  }
  parts.push_back(text.substr(start));
  return parts;
}

struct Clause {
  std::string_view keyword;
  std::string_view inside;
};

std::vector<Clause> ClausesOf(std::string_view view) {
  std::vector<Clause> clauses;
  std::size_t at = 0;
  while (at < view.size()) {
    while (at < view.size() && std::isspace(static_cast<unsigned char>(view[at])) != 0) { ++at; }
    if (at >= view.size()) { break; }
    const std::size_t open = view.find('(', at);
    if (open == std::string_view::npos) {
      throw Error("The view " + std::string(view) + " has a clause without its parentheses");
    }
    int depth = 0;
    std::size_t close = open;
    for (; close < view.size(); ++close) {
      if (view[close] == '(') { ++depth; }
      if (view[close] == ')') {
        --depth;
        if (depth == 0) { break; }
      }
    }
    if (close >= view.size()) {
      throw Error("The view " + std::string(view) + " has a clause that never closes");
    }
    clauses.push_back(Clause{.keyword = Trimmed(view.substr(at, open - at)),
                             .inside = view.substr(open + 1, close - open - 1)});
    at = close + 1;
  }
  return clauses;
}

}

std::string ViewOf(const RecordState *state, const TableDef &table, bool useNames) {
  std::string out = "VERSION(1) SORTING(";
  bool first = true;
  const auto name = [&](FieldNo no) {
    const FieldDef *field = Field(table, no);
    return field == nullptr ? "Field" + std::to_string(no.Value()) : ViewName(*field, useNames);
  };
  if (state != nullptr && !state->key.empty()) {
    for (const SortField &one : state->key) {
      out += (first ? "" : ",") + name(one.field);
      first = false;
    }
  } else if (!table.keys.empty()) {
    for (const FieldNo no : table.keys.front().fields) {
      out += (first ? "" : ",") + name(no);
      first = false;
    }
  }
  out += ") ORDER(";
  out += (state == nullptr || state->ascending) ? "Ascending" : "Descending";
  out += ")";
  if (state == nullptr) { return out; }
  std::string where;
  for (const FieldFilter &one : state->filters) {
    if (one.group != state->group) { continue; }
    where += (where.empty() ? "" : ",") + name(one.field) + "=FILTER(" + one.text + ")";
  }
  if (!where.empty()) { out += " WHERE(" + where + ")"; }
  return out;
}

void ApplyView(RecordState &state, const TableDef &table, std::string_view view) {
  state.filters.clear();
  state.key.clear();
  state.ascending = true;
  if (Trimmed(view).empty()) { return; }
  for (const Clause &clause : ClausesOf(view)) {
    if (SameText(clause.keyword, "VERSION")) { continue; }
    if (SameText(clause.keyword, "SORTING")) {
      std::size_t named = 0;
      for (const std::string_view field : SplitOutsideParentheses(clause.inside, ',')) {
        if (Trimmed(field).empty()) { continue; }
        state.key.push_back(SortField{.field = FieldOfView(table, field).no, .ascending = true});
        ++named;
      }
      if (named == 0) {
        throw Error("The view clause SORTING() names no field of " + std::string(table.name));
      }
      continue;
    }
    if (SameText(clause.keyword, "ORDER")) {
      state.ascending = !SameText(Trimmed(clause.inside), "Descending");
      continue;
    }
    if (SameText(clause.keyword, "WHERE")) {
      for (const std::string_view term : SplitOutsideParentheses(clause.inside, ',')) {
        const std::size_t equals = term.find('=');
        if (equals == std::string_view::npos) {
          throw Error("The view term " + std::string(term) + " has no '='");
        }
        const FieldDef &field = FieldOfView(table, term.substr(0, equals));
        std::string_view value = Trimmed(term.substr(equals + 1));
        const std::size_t open = value.find('(');
        if (open != std::string_view::npos && value.ends_with(")")) {
          value = value.substr(open + 1, value.size() - open - 2);
        }
        Narrow(state, field.no, std::string(value));
      }
      continue;
    }
    throw Error("The view clause " + std::string(clause.keyword) +
                " is not one of VERSION, SORTING, ORDER, WHERE");
  }
}

void RuntimeCopyFilter(const RecordState *from, FieldNo source, void *target, FieldNo destination) {
  RecordState &into = reinterpret_cast<StateHandle *>(target)->Ensure();
  std::vector<FieldFilter> copied;
  if (from != nullptr) {
    for (const FieldFilter &one : from->filters) {
      if (one.field == source) {
        copied.push_back(FieldFilter{.field = destination, .group = one.group, .text = one.text});
      }
    }
  }
  std::erase_if(into.filters, [&](const FieldFilter &one) { return one.field == destination; });
  into.filters.insert(into.filters.end(), copied.begin(), copied.end());
}

void Narrow(RecordState &state, ::agiru::FieldNo field, const std::string &text) {
  const auto same = [field, &state](const FieldFilter &one) {
    return one.field == field && one.group == state.group;
  };
  std::erase_if(state.filters, same);
  if (text.empty()) { return; }
  state.filters.push_back(FieldFilter{.field = field, .group = state.group, .text = text});
}

std::string Literally(std::string_view value) {
  if (value.empty()) { return "''"; }
  if (value.find_first_of("..|&<>=*?@'()") == std::string_view::npos) { return std::string(value); }
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
