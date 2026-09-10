#include "runtime/Relation.h"

#include "Filter.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

std::string_view Trim(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
    text.remove_prefix(1);
  }
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
    text.remove_suffix(1);
  }
  return text;
}

bool SameName(std::string_view a, std::string_view b) {
  return a.size() == b.size() && std::ranges::equal(a, b, [](unsigned char x, unsigned char y) {
           return std::tolower(x) == std::tolower(y);
         });
}

std::string_view Unquoted(std::string_view text) {
  text = Trim(text);
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    return text.substr(1, text.size() - 2);
  }
  return text;
}

bool StartsWithWord(std::string_view text, std::string_view word) {
  if (text.size() < word.size() || !SameName(text.substr(0, word.size()), word)) { return false; }
  return text.size() == word.size() ||
         std::isalnum(static_cast<unsigned char>(text[word.size()])) == 0;
}

std::size_t MatchingClose(std::string_view text, std::size_t open) {
  int depth = 0;
  bool quoted = false;
  for (std::size_t i = open; i < text.size(); ++i) {
    if (text[i] == '"') { quoted = !quoted; }
    if (quoted) { continue; }
    if (text[i] == '(') { ++depth; }
    if (text[i] == ')') {
      --depth;
      if (depth == 0) { return i; }
    }
  }
  return std::string_view::npos;
}

std::vector<std::string_view> SplitTop(std::string_view text, char separator) {
  std::vector<std::string_view> parts;
  int depth = 0;
  bool quoted = false;
  std::size_t start = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '"') { quoted = !quoted; }
    if (quoted) { continue; }
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

std::size_t TopLevelWord(std::string_view text, std::string_view word, std::size_t from) {
  int depth = 0;
  bool quoted = false;
  for (std::size_t i = from; i < text.size(); ++i) {
    if (text[i] == '"') { quoted = !quoted; }
    if (quoted) { continue; }
    if (text[i] == '(') { ++depth; }
    if (text[i] == ')') { --depth; }
    if (depth == 0 && (i == 0 || std::isspace(static_cast<unsigned char>(text[i - 1])) != 0) &&
        StartsWithWord(text.substr(i), word)) {
      return i;
    }
  }
  return std::string_view::npos;
}

const FieldDef *FieldNamed(const TableDef &table, std::string_view name) {
  const std::string_view wanted = Unquoted(name);
  for (const FieldDef &field : table.fields) {
    if (SameName(field.name, wanted)) { return &field; }
  }
  return nullptr;
}

struct Term {
  std::string_view field;
  std::string_view kind;
  std::string_view inner;
};

std::optional<Term> TermOf(std::string_view clause) {
  const std::size_t equals = clause.find('=');
  if (equals == std::string_view::npos) { return std::nullopt; }
  const std::string_view rhs = Trim(clause.substr(equals + 1));
  const std::size_t open = rhs.find('(');
  const std::size_t close = rhs.rfind(')');
  if (open == std::string_view::npos || close == std::string_view::npos || close < open) {
    return Term{.field = Trim(clause.substr(0, equals)), .kind = {}, .inner = rhs};
  }
  return Term{.field = Trim(clause.substr(0, equals)),
              .kind = Trim(rhs.substr(0, open)),
              .inner = Trim(rhs.substr(open + 1, close - open - 1))};
}

std::string FilterTextOf(const Term &term, const void *record, const TableDef &table) {
  if (term.kind.empty() || SameName(term.kind, "const")) {
    return Literally(Unquoted(term.inner));
  }
  if (SameName(term.kind, "filter")) { return std::string(term.inner); }
  if (SameName(term.kind, "field")) {
    const FieldDef *source = FieldNamed(table, term.inner);
    if (source == nullptr) {
      throw Error("TableRelation: `" + std::string(Unquoted(term.inner)) + "` is no field of " +
                  std::string(table.name));
    }
    return Literally(FieldText(record, *source));
  }
  throw Error("TableRelation: `" + std::string(term.kind) + "(...)` is not const, filter or field");
}

bool Satisfies(std::string_view conditions, const void *record, const TableDef &table) {
  for (const std::string_view clause : SplitTop(conditions, ',')) {
    if (Trim(clause).empty()) { continue; }
    const std::optional<Term> term = TermOf(clause);
    if (!term.has_value()) {
      throw Error("TableRelation: the condition `" + std::string(Trim(clause)) + "` has no '='");
    }
    const FieldDef *own = FieldNamed(table, term->field);
    if (own == nullptr) {
      throw Error("TableRelation: `" + std::string(Unquoted(term->field)) + "` is no field of " +
                  std::string(table.name));
    }
    if (!Matches(ParseFilter(FilterTextOf(*term, record, table)), FieldText(record, *own), *own)) {
      return false;
    }
  }
  return true;
}

ResolvedRelation TargetOf(std::string_view text, const void *record, const TableDef &table) {
  ResolvedRelation out;
  std::string_view head = Trim(text);
  const std::size_t where = TopLevelWord(head, "where", 0);
  std::string_view filters;
  if (where != std::string_view::npos) {
    const std::size_t open = head.find('(', where);
    const std::size_t close = open == std::string_view::npos ? open : MatchingClose(head, open);
    if (close == std::string_view::npos) {
      throw Error("TableRelation: the where clause in `" + std::string(head) + "` is not closed");
    }
    filters = head.substr(open + 1, close - open - 1);
    head = Trim(head.substr(0, where));
  }
  std::string_view tableName = head;
  std::string_view fieldName;
  if (!head.empty() && head.front() == '"') {
    const std::size_t closeQuote = head.find('"', 1);
    if (closeQuote != std::string_view::npos && closeQuote + 1 < head.size() &&
        head[closeQuote + 1] == '.') {
      tableName = head.substr(0, closeQuote + 1);
      fieldName = head.substr(closeQuote + 2);
    }
  } else if (const std::size_t dot = head.find('.'); dot != std::string_view::npos) {
    tableName = head.substr(0, dot);
    fieldName = head.substr(dot + 1);
  }
  out.table = std::string(Unquoted(tableName));
  out.field = std::string(Unquoted(fieldName));
  for (const std::string_view clause : SplitTop(filters, ',')) {
    if (Trim(clause).empty()) { continue; }
    const std::optional<Term> term = TermOf(clause);
    if (!term.has_value()) {
      throw Error("TableRelation: the where term `" + std::string(Trim(clause)) + "` has no '='");
    }
    out.filters.push_back(RelationFilter{.field = std::string(Unquoted(term->field)),
                                         .text = FilterTextOf(*term, record, table)});
  }
  return out;
}

}

std::optional<ResolvedRelation>
ResolveRelation(const void *record, const TableDef &table, const FieldDef &def) {
  if (def.relation.empty()) {
    if (def.relationTable.empty()) { return std::nullopt; }
    return ResolvedRelation{.table = std::string(def.relationTable),
                            .field = std::string(def.relationField),
                            .filters = {}};
  }
  std::string_view rest = Trim(def.relation);
  while (!rest.empty()) {
    if (StartsWithWord(rest, "if")) {
      const std::size_t open = rest.find('(');
      const std::size_t close = open == std::string_view::npos ? open : MatchingClose(rest, open);
      if (close == std::string_view::npos) {
        throw Error("TableRelation: the condition in `" + std::string(rest) + "` is not closed");
      }
      const std::string_view conditions = rest.substr(open + 1, close - open - 1);
      std::string_view after = Trim(rest.substr(close + 1));
      const std::size_t elseAt = TopLevelWord(after, "else", 0);
      const std::string_view target =
          elseAt == std::string_view::npos ? after : Trim(after.substr(0, elseAt));
      if (Satisfies(conditions, record, table)) { return TargetOf(target, record, table); }
      if (elseAt == std::string_view::npos) { return std::nullopt; }
      rest = Trim(after.substr(elseAt + 4));
      continue;
    }
    return TargetOf(rest, record, table);
  }
  return std::nullopt;
}

}
