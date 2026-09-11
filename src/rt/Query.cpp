#include "runtime/Query.h"

#include "meta/Ids.h"
#include "meta/QueryDef.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Table.h"

#include "Cursor.h"
#include "Filter.h"
#include "Selection.h"
#include "Where.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

struct ColumnFilter {
  std::size_t column;
  std::string text;
};

struct QueryState {
  std::vector<ColumnFilter> filters;
  std::int32_t top = 0;
  Cursor *cursor = nullptr;
  std::vector<std::size_t> returned;
};

QueryState *MakeQueryState() {
  return new QueryState;
}

void FreeQueryState(QueryState *state) noexcept {
  if (state == nullptr) { return; }
  delete state->cursor;
  delete state;
}

QueryState *CopyQueryState(const QueryState *state) {
  if (state == nullptr) { return nullptr; }
  auto *copy = new QueryState;
  copy->filters = state->filters;
  copy->top = state->top;
  return copy;
}

namespace {

bool SameName(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

std::string Alias(std::size_t dataItem) {
  return "d" + std::to_string(dataItem);
}

const FieldDef &FieldIn(const QueryDef &def, const QueryDataItem &item, FieldNo no) {
  for (const FieldDef &field : item.table->fields) {
    if (field.no == no) { return field; }
  }
  throw Error("the query " + std::string(def.name) + " names field " + std::to_string(no.Value()) +
              " of " + std::string(item.table->name) + ", which the table does not declare");
}

const FieldDef &FieldNamed(const QueryDef &def, const QueryDataItem &item, std::string_view name) {
  for (const FieldDef &field : item.table->fields) {
    if (SameName(field.name, name)) { return field; }
  }
  throw Error("the query " + std::string(def.name) + " filters " + std::string(item.table->name) +
              " on " + std::string(name) + ", which the table does not declare");
}

std::string SourceOf(const QueryDef &def, const QueryColumn &column) {
  const QueryDataItem &item = def.dataItems[column.dataItem];
  return Alias(column.dataItem) + "." + Quoted(FieldIn(def, item, column.field).name);
}

bool IsIntegral(const FieldDef &field) {
  return field.type == FieldType::Integer || field.type == FieldType::BigInteger;
}

std::string ColumnSql(const QueryDef &def, const QueryColumn &column) {
  const std::string source = SourceOf(def, column);
  const FieldDef &field = FieldIn(def, def.dataItems[column.dataItem], column.field);
  std::string expr;
  switch (column.method) {
    case QueryMethod::None: expr = source; break;
    case QueryMethod::Sum: expr = "SUM(" + source + ")"; break;
    case QueryMethod::Average:
      expr = IsIntegral(field) ? "(SUM(" + source + ") / COUNT(" + source + "))"
                               : "AVG(" + source + ")";
      break;
    case QueryMethod::Min: expr = "MIN(" + source + ")"; break;
    case QueryMethod::Max: expr = "MAX(" + source + ")"; break;
    case QueryMethod::Count: expr = "COUNT(*)"; break;
    case QueryMethod::Day: expr = "EXTRACT(DAY FROM " + source + ")::int"; break;
    case QueryMethod::Month: expr = "EXTRACT(MONTH FROM " + source + ")::int"; break;
    case QueryMethod::Year: expr = "EXTRACT(YEAR FROM " + source + ")::int"; break;
  }
  return column.reverseSign ? "-(" + expr + ")" : expr;
}

FieldDef DefOf(const QueryDef &def, const QueryColumn &column) {
  FieldDef made = FieldIn(def, def.dataItems[column.dataItem], column.field);
  made.offset = column.offset;
  made.name = column.name;
  made.caption = column.caption;
  if (column.method == QueryMethod::Count || column.method == QueryMethod::Day ||
      column.method == QueryMethod::Month || column.method == QueryMethod::Year) {
    made.type = FieldType::Integer;
    made.values = {};
    made.length = 0;
  }
  return made;
}

std::string_view JoinWord(QueryJoin join) {
  switch (join) {
    case QueryJoin::LeftOuter: return "LEFT OUTER JOIN";
    case QueryJoin::Inner: return "INNER JOIN";
    case QueryJoin::RightOuter: return "RIGHT OUTER JOIN";
    case QueryJoin::Full: return "FULL OUTER JOIN";
    case QueryJoin::Cross: return "CROSS JOIN";
  }
  return "LEFT OUTER JOIN";
}

struct TableTerm {
  std::string field;
  std::string filter;
};

class TableFilterReader {
public:
  explicit TableFilterReader(std::string_view text) : text_(text) {}

  std::vector<TableTerm> Read() {
    std::vector<TableTerm> terms;
    Space();
    while (at_ < text_.size()) {
      TableTerm term;
      term.field = Name();
      Space();
      Expect('=');
      Space();
      const std::string how = Word();
      Space();
      Expect('(');
      const std::string inside = Balanced();
      if (SameName(how, "const")) {
        term.filter = Literally(ObjectNumberOr(Unquoted(Trimmed(inside))));
      } else if (SameName(how, "filter")) {
        term.filter = Trimmed(inside);
      } else {
        throw Error("a DataItemTableFilter of the kind " + how + " has no translation");
      }
      terms.push_back(std::move(term));
      Space();
      if (at_ < text_.size() && text_[at_] == ',') {
        ++at_;
        Space();
      }
    }
    return terms;
  }

private:
  static std::string ObjectNumberOr(std::string value) {
    static constexpr std::string_view kDatabase = "Database::";
    if (!value.starts_with(kDatabase)) { return value; }
    std::string named = Trimmed(value.substr(kDatabase.size()));
    if (named.size() >= 2 && named.front() == '"' && named.back() == '"') {
      named = named.substr(1, named.size() - 2);
    }
    const TableEntry *entry = FindTable(std::string_view(named));
    if (entry == nullptr) {
      throw Error("a DataItemTableFilter names the table " + named +
                  ", which this build does not carry");
    }
    return std::to_string(entry->table->id.Value());
  }

  void Space() {
    while (at_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[at_])) != 0) {
      ++at_;
    }
  }

  void Expect(char c) {
    if (at_ >= text_.size() || text_[at_] != c) {
      throw Error("a DataItemTableFilter expected '" + std::string(1, c) + "' in \"" +
                  std::string(text_) + "\"");
    }
    ++at_;
  }

  std::string Name() {
    if (at_ < text_.size() && text_[at_] == '"') {
      const std::size_t close = text_.find('"', at_ + 1);
      if (close == std::string_view::npos) { throw Error("an unclosed name in a table filter"); }
      std::string name(text_.substr(at_ + 1, close - at_ - 1));
      at_ = close + 1;
      return name;
    }
    return Word();
  }

  std::string Word() {
    const std::size_t start = at_;
    while (at_ < text_.size() &&
           (std::isalnum(static_cast<unsigned char>(text_[at_])) != 0 || text_[at_] == '_')) {
      ++at_;
    }
    if (start == at_) {
      throw Error("a table filter expected a name in \"" + std::string(text_) + "\"");
    }
    return std::string(text_.substr(start, at_ - start));
  }

  std::string Balanced() {
    std::size_t depth = 1;
    const std::size_t start = at_;
    while (at_ < text_.size()) {
      const char c = text_[at_];
      if (c == '(') { ++depth; }
      if (c == ')' && --depth == 0) {
        std::string inside(text_.substr(start, at_ - start));
        ++at_;
        return inside;
      }
      ++at_;
    }
    throw Error("an unclosed parenthesis in a table filter");
  }

  static std::string Trimmed(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
      value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
      value.remove_suffix(1);
    }
    return std::string(value);
  }

  static std::string Unquoted(std::string value) {
    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
      return value.substr(1, value.size() - 2);
    }
    return value;
  }

  std::string_view text_;
  std::size_t at_ = 0;
};

void And(std::string &where, const Clause &clause, std::vector<std::optional<std::string>> &binds) {
  if (clause.sql.empty()) { return; }
  if (!where.empty()) { where += " AND "; }
  where += clause.sql;
  binds.insert(binds.end(), clause.binds.begin(), clause.binds.end());
}

const std::string *FilterSet(const QueryState &state, std::size_t column) {
  for (const ColumnFilter &filter : state.filters) {
    if (filter.column == column) { return &filter.text; }
  }
  return nullptr;
}

struct Statement {
  std::string sql;
  std::vector<std::optional<std::string>> binds;
  std::vector<std::size_t> returned;
};

Statement Build(const QueryDef &def, const QueryState &state) {
  if (def.dataItems.empty()) {
    throw Error("the query " + std::string(def.name) + " declares no dataitem");
  }
  Statement made;
  std::string select;
  for (std::size_t i = 0; i < def.columns.size(); ++i) {
    if (!def.columns[i].returned) { continue; }
    if (!select.empty()) { select += ", "; }
    select += ColumnSql(def, def.columns[i]) + " AS c" + std::to_string(i);
    made.returned.push_back(i);
  }
  if (select.empty()) { select = "1"; }

  const auto tableFilterOf = [&](std::size_t i, std::string &into) {
    const QueryDataItem &item = def.dataItems[i];
    if (item.tableFilter.empty()) { return; }
    for (const TableTerm &term : TableFilterReader(item.tableFilter).Read()) {
      const FieldDef &field = FieldNamed(def, item, term.field);
      And(into,
          Where(field,
                ParseFilter(term.filter),
                made.binds.size() + 1,
                Alias(i) + "." + Quoted(field.name)),
          made.binds);
    }
  };

  std::string where;
  std::string having;
  tableFilterOf(0, where);
  std::string from = Name(*def.dataItems[0].table) + " " + Alias(0);
  for (std::size_t i = 1; i < def.dataItems.size(); ++i) {
    const QueryDataItem &item = def.dataItems[i];
    from += " " + std::string(JoinWord(item.join)) + " " + Name(*item.table) + " " + Alias(i);
    if (item.join == QueryJoin::Cross) { continue; }
    std::string on;
    for (const QueryLink &link : item.links) {
      if (!on.empty()) { on += " AND "; }
      const QueryDataItem &upper = def.dataItems[link.dataItem];
      on += Alias(i) + "." + Quoted(FieldIn(def, item, link.field).name) + " = " +
            Alias(link.dataItem) + "." + Quoted(FieldIn(def, upper, link.reference).name);
    }
    tableFilterOf(i, on);
    from += " ON " + (on.empty() ? std::string("TRUE") : on);
  }
  for (std::size_t i = 0; i < def.columns.size(); ++i) {
    const QueryColumn &column = def.columns[i];
    const std::string *set = FilterSet(state, i);
    const std::string_view text = set != nullptr ? std::string_view(*set) : column.columnFilter;
    if (text.empty()) { continue; }
    const FieldDef field = DefOf(def, column);
    const Clause clause =
        Where(field, ParseFilter(text), made.binds.size() + 1, ColumnSql(def, column));
    And(Aggregates(column.method) ? having : where, clause, made.binds);
  }

  std::string group;
  if (Groups(def)) {
    for (const std::size_t i : made.returned) {
      if (Aggregates(def.columns[i].method)) { continue; }
      if (!group.empty()) { group += ", "; }
      group += ColumnSql(def, def.columns[i]);
    }
  }

  std::string order;
  for (const QueryOrder &by : def.orderBy) {
    std::size_t found = def.columns.size();
    for (std::size_t i = 0; i < def.columns.size(); ++i) {
      if (SameName(def.columns[i].name, by.column)) {
        found = i;
        break;
      }
    }
    if (found == def.columns.size()) {
      throw Error("the query " + std::string(def.name) + " orders by " + std::string(by.column) +
                  ", which is not one of its columns");
    }
    if (!order.empty()) { order += ", "; }
    order += ColumnSql(def, def.columns[found]);
    if (by.descending) { order += " DESC"; }
  }

  made.sql = "SELECT " + select + " FROM " + from;
  if (!where.empty()) { made.sql += " WHERE " + where; }
  if (!group.empty()) { made.sql += " GROUP BY " + group; }
  if (!having.empty()) { made.sql += " HAVING " + having; }
  if (!order.empty()) { made.sql += " ORDER BY " + order; }
  const std::int32_t top = state.top != 0 ? state.top : def.topNumberOfRows;
  if (top > 0) { made.sql += " LIMIT " + std::to_string(top); }
  return made;
}

}

bool QueryOpen(QueryState &state, const QueryDef &def) {
  QueryClose(state);
  Statement made = Build(def, state);
  const Connection &connection = Session::Current().Database();
  if (!connection.InTransaction()) { connection.Run("BEGIN"); }
  state.cursor = new Cursor(connection, made.sql, std::move(made.binds));
  state.returned = std::move(made.returned);
  return true;
}

bool QueryRead(QueryState &state, const QueryDef &def, void *self) {
  if (state.cursor == nullptr) {
    throw Error("Query.Read: the query " + std::string(def.name) + " is not open");
  }
  if (!state.cursor->Step()) {
    QueryClose(state);
    return false;
  }
  std::size_t at = 0;
  for (const std::size_t i : state.returned) {
    const std::optional<std::string_view> value = state.cursor->Value(at++);
    const FieldDef field = DefOf(def, def.columns[i]);
    SetFieldText(self, field, value.has_value() ? *value : std::string_view(BlankValueOf(field)));
  }
  return true;
}

void QueryClose(QueryState &state) noexcept {
  delete state.cursor;
  state.cursor = nullptr;
  state.returned.clear();
}

void QueryNarrow(QueryState &state, std::size_t column, const std::string &text) {
  std::erase_if(state.filters,
                [column](const ColumnFilter &filter) { return filter.column == column; });
  if (!text.empty()) { state.filters.push_back(ColumnFilter{.column = column, .text = text}); }
}

std::string QueryFilterOn(const QueryState &state, std::size_t column) {
  const std::string *set = FilterSet(state, column);
  return set == nullptr ? std::string{} : *set;
}

std::string QueryFilters(const QueryState &state, const QueryDef &def) {
  std::string out;
  for (const ColumnFilter &filter : state.filters) {
    if (!out.empty()) { out += ", "; }
    out += std::string(def.columns[filter.column].name) + ": " + filter.text;
  }
  return out;
}

std::int32_t QueryTop(QueryState &state, std::int32_t rows) {
  const std::int32_t was = state.top;
  state.top = rows;
  return was;
}

}
