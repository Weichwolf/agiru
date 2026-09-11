#include "Where.h"

#include "meta/TableDef.h"
#include "runtime/Record.h"
#include "type/StringValue.h"

#include "Filter.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::detail {

namespace {

std::string Quoted(std::string_view identifier) {
  std::string out = "\"";
  for (const char c : identifier) {
    if (c == '"') { out += '"'; }
    out += c;
  }
  out += '"';
  return out;
}

std::string Placeholder(std::size_t oneBased) {
  return "$" + std::to_string(oneBased);
}

std::string LikePattern(std::string_view value) {
  std::string out;
  for (const char c : value) {
    switch (c) {
      case '*': out += '%'; break;
      case '?': out += '_'; break;
      case '%':
      case '_':
      case '\\':
        out += '\\';
        out += c;
        break;
      default: out += c;
    }
  }
  return out;
}

}

std::string BlankValueOf(const FieldDef &def) {
  switch (def.type) {
    case FieldType::Boolean: return "false";
    case FieldType::Option:
    case FieldType::Enum: return detail::MemberOrdinal(def, "");
    case FieldType::Integer:
    case FieldType::BigInteger:
    case FieldType::Decimal:
    case FieldType::Duration: return "0";
    case FieldType::Date:
    case FieldType::DateTime: return "1753-01-01 00:00:00";
    case FieldType::Time: return "00:00:00";
    case FieldType::Guid:
    case FieldType::Media:
    case FieldType::MediaSet: return "00000000-0000-0000-0000-000000000000";
    default: return {};
  }
}

namespace {

void One(const Atom &atom,
         const FieldDef &def,
         Clause &into,
         std::size_t &next,
         const std::string &column) {
  const bool byMember = def.type == FieldType::Option || def.type == FieldType::Enum;
  const bool asCode = def.type == FieldType::Code;
  const auto bind = [&into, &next, &def, byMember, asCode](const std::string &value) {
    if (value.empty()) {
      into.binds.emplace_back(BlankValueOf(def));
    } else {
      into.binds.emplace_back(byMember ? detail::MemberOrdinal(def, value)
                              : asCode ? NormaliseCode(value)
                                       : value);
    }
    return Placeholder(next++);
  };
  switch (atom.compare) {
    case Compare::Equal: into.sql += column + " = " + bind(atom.value); break;
    case Compare::NotEqual: into.sql += column + " <> " + bind(atom.value); break;
    case Compare::Less: into.sql += column + " < " + bind(atom.value); break;
    case Compare::LessOrEqual: into.sql += column + " <= " + bind(atom.value); break;
    case Compare::Greater: into.sql += column + " > " + bind(atom.value); break;
    case Compare::GreaterEqual: into.sql += column + " >= " + bind(atom.value); break;
    case Compare::Like: into.sql += column + "::text ILIKE " + bind(LikePattern(atom.value)); break;
    case Compare::NotLike:
      into.sql += column + "::text NOT ILIKE " + bind(LikePattern(atom.value));
      break;
    case Compare::Between: {
      if (atom.openLower && atom.openUpper) {
        into.sql += "TRUE";
        break;
      }
      if (atom.openLower) {
        into.sql += column + " <= " + bind(atom.upper);
        break;
      }
      if (atom.openUpper) {
        into.sql += column + " >= " + bind(atom.value);
        break;
      }
      into.sql += column + " BETWEEN " + bind(atom.value);
      into.sql += " AND " + bind(atom.upper);
      break;
    }
  }
}

}

Clause Where(const FieldDef &def, const Expression &expr, std::size_t first) {
  return Where(def, expr, first, Quoted(def.name));
}

Clause
Where(const FieldDef &def, const Expression &expr, std::size_t first, std::string_view column) {
  const std::string named(column);
  Clause clause;
  if (expr.empty()) { return clause; }
  std::size_t next = first;
  clause.sql = "(";
  bool firstAlternative = true;
  for (const All &conjunction : expr) {
    if (!firstAlternative) { clause.sql += " OR "; }
    firstAlternative = false;
    clause.sql += "(";
    bool firstAtom = true;
    for (const Atom &atom : conjunction) {
      if (!firstAtom) { clause.sql += " AND "; }
      firstAtom = false;
      One(atom, def, clause, next, named);
    }
    clause.sql += ")";
  }
  clause.sql += ")";
  return clause;
}

}
