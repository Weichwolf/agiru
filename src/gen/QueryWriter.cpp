#include "QueryWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "Names.h"
#include "Scope.h"
#include "TableWriter.h"
#include "Token.h"

#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

namespace {

struct Column {
  std::string name;
  std::string caption;
  std::string field;
  std::size_t dataItem = 0;
  std::string method;
  bool returned = true;
  bool reverseSign = false;
  std::string columnFilter;
};

struct Link {
  std::string field;
  std::string dataItem;
  std::string reference;
};

struct DataItem {
  std::string name;
  std::string table;
  std::string join;
  std::vector<Link> links;
  std::string tableFilter;
};

struct Elements {
  std::vector<DataItem> dataItems;
  std::vector<Column> columns;
};

bool WordLike(const al::Token &token) {
  return token.kind == al::TokenKind::Identifier || token.kind == al::TokenKind::QuotedIdentifier ||
         token.kind == al::TokenKind::Integer || token.kind == al::TokenKind::Decimal ||
         token.kind == al::TokenKind::String;
}

std::string Rendered(const std::vector<al::Token> &tokens) {
  std::string out;
  const al::Token *previous = nullptr;
  for (const al::Token &token : tokens) {
    if (previous != nullptr && WordLike(*previous) && WordLike(token)) { out += ' '; }
    switch (token.kind) {
      case al::TokenKind::QuotedIdentifier: out += "\"" + token.text + "\""; break;
      case al::TokenKind::String: out += "'" + token.text + "'"; break;
      default: out += token.text;
    }
    previous = &token;
  }
  return out;
}

std::string SourceName(const std::vector<al::Token> &source) {
  std::size_t from = 0;
  for (std::size_t i = 0; i < source.size(); ++i) {
    if (source[i].kind == al::TokenKind::Punctuation && source[i].text == ".") { from = i + 1; }
  }
  std::string out;
  for (std::size_t i = from; i < source.size(); ++i) {
    if (!out.empty()) { out += ' '; }
    out += source[i].text;
  }
  return out;
}

std::string PropertyText(const std::vector<al::Property> &properties, std::string_view name) {
  const al::Property *found = al::Find(properties, name);
  return found == nullptr ? std::string{} : Rendered(found->value);
}

std::string FilterPart(const std::vector<al::Token> &value) {
  std::size_t equals = value.size();
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i].kind == al::TokenKind::Punctuation && value[i].text == "=") {
      equals = i;
      break;
    }
  }
  if (equals == value.size()) { return Rendered(value); }
  return Rendered(
      std::vector<al::Token>(value.begin() + static_cast<std::ptrdiff_t>(equals) + 1, value.end()));
}

std::string ColumnFilterOf(const std::vector<al::Property> &properties) {
  const al::Property *found = al::Find(properties, "ColumnFilter");
  if (found == nullptr) { return {}; }
  const std::string rest = FilterPart(found->value);
  const std::size_t open = rest.find('(');
  const std::size_t close = rest.rfind(')');
  if (open == std::string::npos || close == std::string::npos || close < open) { return rest; }
  std::string how = rest.substr(0, open);
  while (!how.empty() && std::isspace(static_cast<unsigned char>(how.back())) != 0) {
    how.pop_back();
  }
  std::string inside = rest.substr(open + 1, close - open - 1);
  while (!inside.empty() && std::isspace(static_cast<unsigned char>(inside.front())) != 0) {
    inside.erase(inside.begin());
  }
  while (!inside.empty() && std::isspace(static_cast<unsigned char>(inside.back())) != 0) {
    inside.pop_back();
  }
  if (LowerKey(how) == "const") {
    if (inside.size() >= 2 && inside.front() == '\'' && inside.back() == '\'') {
      inside = inside.substr(1, inside.size() - 2);
    }
    return "'" + inside + "'";
  }
  return inside;
}

std::vector<Link> LinksOf(const std::vector<al::Property> &properties) {
  std::vector<Link> links;
  const al::Property *found = al::Find(properties, "DataItemLink");
  if (found == nullptr) { return links; }
  std::vector<std::vector<al::Token>> parts(1);
  for (const al::Token &token : found->value) {
    if (token.kind == al::TokenKind::Punctuation && token.text == ",") {
      parts.emplace_back();
      continue;
    }
    parts.back().push_back(token);
  }
  for (const std::vector<al::Token> &part : parts) {
    std::vector<std::string> words;
    for (const al::Token &token : part) {
      if (token.kind == al::TokenKind::Punctuation) { continue; }
      words.push_back(token.text);
    }
    if (words.size() != 3) {
      throw std::runtime_error("a DataItemLink is Field = DataItem.Field, and this one reads \"" +
                               Rendered(part) + "\"");
    }
    links.push_back(Link{.field = words[0], .dataItem = words[1], .reference = words[2]});
  }
  return links;
}

void Gather(const std::vector<al::PageControl> &controls, Elements &into, std::size_t owner) {
  for (const al::PageControl &control : controls) {
    const std::string kind = LowerKey(control.kind);
    if (kind == "dataitem") {
      into.dataItems.push_back(
          DataItem{.name = control.name,
                   .table = SourceName(control.source),
                   .join = PropertyText(control.properties, "SqlJoinType"),
                   .links = LinksOf(control.properties),
                   .tableFilter = PropertyText(control.properties, "DataItemTableFilter")});
      Gather(control.children, into, into.dataItems.size() - 1);
      continue;
    }
    if (kind == "column" || kind == "filter") {
      const al::Property *caption = al::Find(control.properties, "Caption");
      const al::Property *reverse = al::Find(control.properties, "ReverseSign");
      into.columns.push_back(
          Column{.name = control.name,
                 .caption = caption == nullptr ? control.name : caption->text,
                 .field = SourceName(control.source),
                 .dataItem = owner,
                 .method = LowerKey(PropertyText(control.properties, "Method")),
                 .returned = kind == "column",
                 .reverseSign = reverse != nullptr && LowerKey(reverse->text) == "true",
                 .columnFilter = ColumnFilterOf(control.properties)});
      continue;
    }
    Gather(control.children, into, owner);
  }
}

std::string JoinOf(const std::string &join) {
  const std::string key = LowerKey(join);
  if (key.empty() || key == "leftouterjoin") { return "LeftOuter"; }
  if (key == "innerjoin") { return "Inner"; }
  if (key == "rightouterjoin") { return "RightOuter"; }
  if (key == "fullouterjoin" || key == "fulljoin") { return "Full"; }
  if (key == "crossjoin") { return "Cross"; }
  throw std::runtime_error("a SqlJoinType of " + join + " has no translation");
}

std::string MethodOf(const std::string &method) {
  if (method.empty()) { return "None"; }
  static const std::map<std::string, std::string> kMethods{
      {"sum", "Sum"},
      {"average", "Average"},
      {"min", "Min"},
      {"max", "Max"},
      {"count", "Count"},
      {"day", "Day"},
      {"month", "Month"},
      {"year", "Year"},
  };
  const auto found = kMethods.find(method);
  if (found == kMethods.end()) {
    throw std::runtime_error("a column Method of " + method + " has no translation");
  }
  return found->second;
}

bool IntegerValued(const std::string &method) {
  return method == "Count" || method == "Day" || method == "Month" || method == "Year";
}

std::string TableSymbol(const std::string &identifier) {
  const std::size_t colons = identifier.rfind("::");
  const std::string space = colons == std::string::npos ? "" : identifier.substr(0, colons + 2);
  std::string name = colons == std::string::npos ? identifier : identifier.substr(colons + 2);
  constexpr std::string_view kSuffix = "_Table";
  if (name.ends_with(kSuffix)) { name.resize(name.size() - kSuffix.size()); }
  return space + "k" + name + "Table";
}

struct Orders {
  std::vector<std::pair<std::string, bool>> terms;
};

Orders OrderOf(const std::vector<al::Property> &properties) {
  Orders orders;
  const al::Property *found = al::Find(properties, "OrderBy");
  if (found == nullptr) { return orders; }
  bool descending = false;
  for (const al::Token &token : found->value) {
    if (token.kind == al::TokenKind::Punctuation) { continue; }
    const std::string key = LowerKey(token.text);
    if (key == "ascending") {
      descending = false;
      continue;
    }
    if (key == "descending") {
      descending = true;
      continue;
    }
    orders.terms.emplace_back(token.text, descending);
  }
  return orders;
}

al::TableObject FacadeOf(const al::QueryObject &query, const Elements &elements) {
  al::TableObject facade;
  facade.id = query.id;
  facade.name = query.name;
  facade.nameSpace = query.nameSpace;
  facade.variables = query.variables;
  facade.labels = query.labels;
  facade.properties.push_back(al::Property{.name = "QueryColumns", .value = {}, .text = "true"});
  int number = 0;
  for (const Column &column : elements.columns) {
    al::FieldDecl field;
    field.number = ++number;
    field.name = column.name;
    field.type = "Text";
    facade.fields.push_back(std::move(field));
  }
  for (const al::ProcedureDecl &procedure : query.procedures) {
    if (procedure.isTrigger && LowerKey(procedure.name) != "onbeforeopen") { continue; }
    facade.procedures.push_back(procedure);
  }
  return facade;
}

}

std::string QueryHeaderPath(const al::QueryObject &query) {
  return OutputDirectory(query.nameSpace, ObjectKind::Query) + "/" + Identifier(query.name) + ".h";
}

std::map<std::string, std::string> QueryColumns(const al::QueryObject &query) {
  Elements elements;
  Gather(query.elements, elements, 0);
  const al::TableObject facade = FacadeOf(query, elements);
  std::map<std::string, std::string> named;
  for (const Column &column : elements.columns) {
    named.emplace(LowerKey(column.name), FieldIdentifier(facade, column.name));
  }
  return named;
}

std::map<std::string, std::pair<std::string, std::string>>
QueryColumnSources(const al::QueryObject &query) {
  Elements elements;
  Gather(query.elements, elements, 0);
  std::map<std::string, std::pair<std::string, std::string>> sources;
  for (const Column &column : elements.columns) {
    if (column.field.empty()) { continue; }
    sources.emplace(LowerKey(column.name),
                    std::make_pair(elements.dataItems[column.dataItem].table, column.field));
  }
  return sources;
}

QueryWritten
WriteQuery(const al::QueryObject &query, const std::string &sourcePath, const Objects &objects) {
  QueryWritten written;
  Elements elements;
  Gather(query.elements, elements, 0);
  al::TableObject facade = FacadeOf(query, elements);
  for (const al::ProcedureDecl &procedure : query.procedures) {
    if (procedure.isTrigger && LowerKey(procedure.name) != "onbeforeopen") { ++written.triggers; }
  }
  const std::string identifier = Identifier(query.name);
  const std::string className = ClassName(identifier, ObjectKind::Query);
  const std::string space = NamespaceOf(query.nameSpace);
  const std::string symbol = "k" + identifier + "Query";
  const std::string number = std::to_string(query.id);
  const al::Property *caption = al::Find(query.properties, "Caption");
  const std::string captionText = caption == nullptr ? query.name : caption->text;
  const std::string top = PropertyText(query.properties, "TopNumberOfRows");

  std::vector<const TableRef *> tables;
  std::set<std::string> headers{"meta/Ids.h", "meta/QueryDef.h", "runtime/Query.h"};
  for (const DataItem &item : elements.dataItems) {
    const auto found = objects.tables.find(LowerKey(item.table));
    if (found == objects.tables.end() || found->second.header.empty()) {
      written.missing.push_back(item.table);
      tables.push_back(nullptr);
      continue;
    }
    tables.push_back(&found->second);
    headers.insert(found->second.header);
  }
  const auto memberOf = [&](const Column &column, std::string &member) {
    const TableRef *table = tables[column.dataItem];
    if (table == nullptr) { return false; }
    if (column.field.empty() && column.method == "count") {
      member.clear();
      return true;
    }
    const auto found = table->fields.find(LowerKey(column.field));
    if (found == table->fields.end()) {
      written.missing.push_back(elements.dataItems[column.dataItem].table + "." + column.field);
      return false;
    }
    member = found->second;
    return true;
  };
  for (const Column &column : elements.columns) {
    std::string member;
    static_cast<void>(memberOf(column, member));
  }
  if (!written.missing.empty()) { return written; }

  std::string h;
  h += "// Generated from " + sourcePath + ". Do not edit.\n\n";
  h += "#pragma once\n\n";
  for (const std::string &header : headers) { h += "#include \"" + header + "\"\n"; }
  h += SourceIncludesOf(facade.variables, facade.procedures, objects);
  h += kDoorMarker;
  h += "\n#include <cstddef>\n#include <string_view>\n\n";
  h += "namespace " + space + " {\n\n";
  h += "class " + className + ";\n\n";
  h += "extern const QueryDef " + symbol + ";\n\n";
  h += "class " + className + " : public ::agiru::Query<" + className + "> {\npublic:\n";
  h += "  static constexpr QueryId kId{" + number + "};\n";
  h += "  static constexpr std::string_view kName{" + Literal(query.name) + "};\n";
  h += "  detail::QueryHandle State_Block;\n";
  for (const Column &column : elements.columns) {
    std::string member;
    static_cast<void>(memberOf(column, member));
    const std::string method = MethodOf(column.method);
    const std::string type =
        IntegerValued(method)
            ? "::agiru::Integer"
            : "decltype(" + tables[column.dataItem]->identifier + "::" + member + ")";
    h += "  " + type + " " + FieldIdentifier(facade, column.name) + "{};\n";
  }
  const std::string members =
      MemberDeclarations(query.name, facade.variables, facade.labels, facade.procedures, objects);
  if (!members.empty()) { h += "\n" + members; }
  if (!facade.procedures.empty()) { h += "\n"; }
  for (const al::ProcedureDecl &procedure : facade.procedures) {
    h += "  " +
         ProcedureDeclaration(procedure, objects, query.name, Shadowed(facade), facade.procedures) +
         "\n";
  }
  h += "};\n\n";
  h += "} // namespace " + space + "\n\n";
  h += "template <> struct agiru::QueryTraits<" + space + "::" + className + "> {\n";
  h += "  static constexpr QueryId kId{" + number + "};\n";
  h += "  static constexpr std::string_view kName{" + Literal(query.name) + "};\n";
  h += "  static constexpr const QueryDef &kQuery = " + space + "::" + symbol + ";\n";
  h += "};\n";
  written.header = WithDoor(h, ObjectKind::Query);

  std::string s;
  s += "// Generated from " + sourcePath + ". Do not edit.\n\n";
  s += "#include \"" + identifier + ".h\"\n\n";
  s += kDoorMarker;
  s += "\n" + SourceIncludesOf(facade.variables, facade.procedures, objects);
  s += "\n#include <array>\n#include <cstddef>\n\n";
  s += "namespace " + space + " {\n\nnamespace {\n\n";
  for (std::size_t i = 0; i < elements.dataItems.size(); ++i) {
    const DataItem &item = elements.dataItems[i];
    if (item.links.empty()) { continue; }
    s += "constexpr std::array<QueryLink, " + std::to_string(item.links.size()) + "> k" +
         identifier + "Links" + std::to_string(i) + "{{\n";
    for (const Link &link : item.links) {
      std::size_t upper = elements.dataItems.size();
      for (std::size_t j = 0; j < i; ++j) {
        if (LowerKey(elements.dataItems[j].name) == LowerKey(link.dataItem)) { upper = j; }
      }
      if (upper == elements.dataItems.size()) {
        throw std::runtime_error("the query " + query.name + " links " + item.name + " to " +
                                 link.dataItem + ", which is not a dataitem above it");
      }
      const auto field = tables[i]->fields.find(LowerKey(link.field));
      const auto reference = tables[upper]->fields.find(LowerKey(link.reference));
      if (field == tables[i]->fields.end() || reference == tables[upper]->fields.end()) {
        throw std::runtime_error("the query " + query.name + " links " + link.field + " to " +
                                 link.dataItem + "." + link.reference +
                                 ", and one of them is not a field of its table");
      }
      s += "    QueryLink{.field = " + tables[i]->identifier + "::Field_No::" + field->second +
           ", .dataItem = " + std::to_string(upper) +
           ", .reference = " + tables[upper]->identifier + "::Field_No::" + reference->second +
           "},\n";
    }
    s += "}};\n\n";
  }
  s += "constexpr std::array<QueryDataItem, " + std::to_string(elements.dataItems.size()) + "> k" +
       identifier + "DataItems{{\n";
  for (std::size_t i = 0; i < elements.dataItems.size(); ++i) {
    const DataItem &item = elements.dataItems[i];
    s += "    QueryDataItem{.name = " + Literal(item.name) + ", .table = &" +
         TableSymbol(tables[i]->identifier) + ", .join = QueryJoin::" + JoinOf(item.join) +
         ", .links = " +
         (item.links.empty() ? std::string("{}") : "k" + identifier + "Links" + std::to_string(i)) +
         ", .tableFilter = " + Literal(item.tableFilter) + "},\n";
  }
  s += "}};\n\n";
  s += "constexpr std::array<QueryColumn, " + std::to_string(elements.columns.size()) + "> k" +
       identifier + "Columns{{\n";
  for (const Column &column : elements.columns) {
    std::string member;
    static_cast<void>(memberOf(column, member));
    s += "    QueryColumn{.name = " + Literal(column.name) +
         ", .caption = " + Literal(column.caption) + ", .offset = offsetof(" + className + ", " +
         FieldIdentifier(facade, column.name) +
         "), .dataItem = " + std::to_string(column.dataItem) + ", .field = " +
         (member.empty() ? std::string("::agiru::FieldNo{}")
                         : tables[column.dataItem]->identifier + "::Field_No::" + member) +
         ", .method = QueryMethod::" + MethodOf(column.method) +
         ", .returned = " + (column.returned ? "true" : "false") +
         ", .reverseSign = " + (column.reverseSign ? "true" : "false") +
         ", .columnFilter = " + Literal(column.columnFilter) + "},\n";
  }
  s += "}};\n\n";
  const Orders orders = OrderOf(query.properties);
  if (!orders.terms.empty()) {
    s += "constexpr std::array<QueryOrder, " + std::to_string(orders.terms.size()) + "> k" +
         identifier + "Order{{\n";
    for (const auto &[column, descending] : orders.terms) {
      s += "    QueryOrder{.column = " + Literal(column) +
           ", .descending = " + (descending ? "true" : "false") + "},\n";
    }
    s += "}};\n\n";
  }
  s += "}\n\n";
  s += "constexpr QueryDef " + symbol + "{\n";
  s += "    .id = " + className + "::kId,\n";
  s += "    .name = " + className + "::kName,\n";
  s += "    .caption = " + Literal(captionText) + ",\n";
  s += "    .dataItems = k" + identifier + "DataItems,\n";
  s += "    .columns = k" + identifier + "Columns,\n";
  s += "    .orderBy = " + (orders.terms.empty() ? std::string("{}") : "k" + identifier + "Order") +
       ",\n";
  s += "    .topNumberOfRows = " + (top.empty() ? std::string("0") : top) + ",\n";
  s += "};\n\n";
  s += QueryProcedureBodies(facade, className, objects);
  s += "static_assert(offsetof(" + className + ", State_Block) == 0, \"the state comes first\");\n";
  s += "static_assert(k" + identifier +
       "Columns.size() == " + std::to_string(elements.columns.size()) + ", \"query " + number +
       " declares " + std::to_string(elements.columns.size()) + " columns and filters\");\n\n";
  s += "} // namespace " + space + "\n";
  written.source = WithDoor(s, ObjectKind::Query);
  return written;
}

}
