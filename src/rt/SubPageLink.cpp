#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
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

std::vector<std::string_view> SplitTop(std::string_view text, char separator) {
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

const FieldDef *FieldNamed(const TableDef &table, std::string_view name) {
  const std::string_view wanted = Unquoted(name);
  for (const FieldDef &field : table.fields) {
    if (SameName(field.name, wanted)) { return &field; }
  }
  return nullptr;
}

std::string TableNumberNamed(std::string_view name) {
  const std::string_view wanted = Unquoted(name);
  for (const TableEntry *entry : InstalledTables()) {
    if (SameName(entry->table->name, wanted)) { return std::to_string(entry->table->id.Value()); }
  }
  throw Error("SubPageLink: `Database::" + std::string(wanted) + "` names no table this build has");
}

std::string ConstText(std::string_view value) {
  const std::string_view trimmed = Trim(value);
  const std::size_t database = trimmed.find("::");
  if (database != std::string_view::npos && SameName(Trim(trimmed.substr(0, database)), "Database")) {
    return TableNumberNamed(trimmed.substr(database + 2));
  }
  return std::string(Unquoted(trimmed));
}

}

void ApplySubPageLink(void *sub,
                      const TableDef &subTable,
                      const void *parent,
                      const TableDef &parentTable,
                      std::string_view link) {
  RecordState &state = reinterpret_cast<StateHandle *>(sub)->Ensure();
  for (const std::string_view clause : SplitTop(link, ',')) {
    const std::size_t equals = clause.find('=');
    if (equals == std::string_view::npos) { continue; }
    const FieldDef *target = FieldNamed(subTable, clause.substr(0, equals));
    if (target == nullptr) {
      throw Error("SubPageLink: `" + std::string(Trim(clause.substr(0, equals))) + "` is no field of " +
                  std::string(subTable.name));
    }
    const std::string_view rhs = Trim(clause.substr(equals + 1));
    const std::size_t open = rhs.find('(');
    const std::size_t close = rhs.rfind(')');
    if (open == std::string_view::npos || close == std::string_view::npos || close < open) {
      Narrow(state, target->no, Literally(Unquoted(rhs)));
      continue;
    }
    const std::string_view kind = Trim(rhs.substr(0, open));
    const std::string_view inner = Trim(rhs.substr(open + 1, close - open - 1));
    if (SameName(kind, "field")) {
      const FieldDef *source = FieldNamed(parentTable, inner);
      if (source == nullptr) {
        throw Error("SubPageLink: `" + std::string(Unquoted(inner)) + "` is no field of " +
                    std::string(parentTable.name));
      }
      Narrow(state, target->no, Literally(FieldText(parent, *source)));
    } else if (SameName(kind, "const")) {
      Narrow(state, target->no, Literally(ConstText(inner)));
    } else if (SameName(kind, "filter")) {
      Narrow(state, target->no, std::string(inner));
    } else {
      throw Error("SubPageLink: `" + std::string(kind) + "(...)` is not field, const or filter");
    }
  }
}

}
