#include "Names.h"

#include "Scope.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

char Lower(char c) {
  return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

std::vector<std::string> Words(std::string_view name) {
  std::vector<std::string> words;
  std::string current;
  for (const char c : name) {
    if (std::isalnum(static_cast<unsigned char>(c)) != 0) {
      current += c;
      continue;
    }
    if (c == '%') {
      if (!current.empty()) {
        words.push_back(current);
        current.clear();
      }
      words.emplace_back("Percent");
      continue;
    }
    if (!current.empty()) {
      words.push_back(current);
      current.clear();
    }
  }
  if (!current.empty()) { words.push_back(current); }
  return words;
}

std::string Join(const std::vector<std::string> &words) {
  std::string out;
  for (const std::string &word : words) {
    std::string part = word;
    part[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(part[0])));
    out += part;
  }
  if (!out.empty() && (std::isdigit(static_cast<unsigned char>(out[0])) != 0)) {
    out.insert(0, "_");
  }
  return out;
}

} // namespace

std::string_view KindSuffix(ObjectKind kind) {
  switch (kind) {
    case ObjectKind::Table: return "Table";
    case ObjectKind::Codeunit: return "Codeunit";
    case ObjectKind::Page: return "Page";
    case ObjectKind::Report: return "Report";
    case ObjectKind::Query: return "Query";
    case ObjectKind::XmlPort: return "XmlPort";
    case ObjectKind::Enum: return "Enum";
    case ObjectKind::Interface: return "Interface";
    case ObjectKind::PermissionSet: return "PermissionSet";
  }
  return "Object";
}

std::string ClassName(std::string_view identifier, ObjectKind kind) {
  return std::string(identifier) + "_" + std::string(KindSuffix(kind));
}

std::string ClassAlias(std::string_view identifier, ObjectKind kind) {
  return "using " + std::string(identifier) + " = " + ClassName(identifier, kind) + ";\n";
}

ObjectKind KindOfNamespace(std::string_view space) {
  if (space == "tables") { return ObjectKind::Table; }
  if (space == "interfaces") { return ObjectKind::Interface; }
  if (space == "enums") { return ObjectKind::Enum; }
  if (space == "pages") { return ObjectKind::Page; }
  return ObjectKind::Codeunit;
}

std::string Identifier(std::string_view alName) {
  return Join(Words(alName));
}

std::string EnumeratorName(std::string_view optionMember) {
  const std::string name = Join(Words(optionMember));
  return name.empty() ? "Blank" : name;
}

// TWO MEMBERS CAN SCRUB TO ONE IDENTIFIER, AND 24 TABLES DO IT. `OptionMembers = ,"Sales Order",,,`
// is ordinary in the BaseApp -- a blank member marks "none" and later blanks are reserved ordinals
// that were never filled in. Every one of them becomes `Blank`, and the second is a redefinition
// the compiler refuses (measured 2026-09-01). The ORDINAL disambiguates, because it is the one
// thing that is already unique and already meaningful: the first keeps the bare name and each
// later collision carries the number AL gave it.
std::vector<std::string> EnumeratorNames(const std::vector<std::string> &members) {
  std::vector<std::string> names;
  names.reserve(members.size());
  for (std::size_t i = 0; i < members.size(); ++i) {
    std::string name = EnumeratorName(members[i]);
    if (std::ranges::find(names, name) != names.end()) { name += std::to_string(i); }
    names.push_back(std::move(name));
  }
  return names;
}

std::string OptionEnumName(std::string_view tableName, std::string_view fieldName) {
  return Identifier(tableName) + Identifier(fieldName);
}

// AL HAS NO ESCAPE CHARACTER AND C++ DOES. A Label may read
// `The Total Cubage %1 ... exceeds %3 %4.\\Do you still want to enter this %3?` -- in AL that
// backslash is a line break in the message and nothing more, and copied into a C++ literal it
// becomes `\\D`, which is an unknown escape sequence and therefore an error under -Werror. 196
// emitted strings in 69 of the BaseApp's tables carry one, and 16 carry a quote that would close
// the literal early (measured 2026-09-01).
std::string Literal(std::string_view text) {
  std::string out;
  out.reserve(text.size() + 2);
  out += '"';
  for (const char c : text) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += c;
    }
  }
  out += '"';
  return out;
}

// AL TYPE NAMES ARE CASE-INSENSITIVE AND THE BASEAPP USES THAT. The same field type is written
// `Guid` 822 times and `GUID` 209 times, `BLOB` 163 times and `Blob` 54, `Enum` 1 351 times and
// `enum` 41 (measured 2026-09-01). Passing the spelling through would ask for eight door headers
// per type and would let `enum "Item Type"` slip past the enum path entirely. The canonical
// spelling is the one the platform documentation gives its data-type page -- `RecordId`, not
// `RecordID`.
namespace {

// AL'S OWN TYPE NAMES, AND THE LIST IS THE DOOR'S. A name that is not here and not an object this
// run read is a type nobody defines, so it goes to `absent::` -- which means every type the door
// gains has to arrive here too. It fell behind once already: `ModuleInfo`, `Version`,
// `Notification` and `Variant` were built, were not listed, and were sent to the absent as a
// result.
constexpr std::array kCanonicalTypes{
    std::string_view{"Action"},
    std::string_view{"AuditCategory"},
    std::string_view{"BigInteger"},
    std::string_view{"Blob"},
    std::string_view{"Boolean"},
    std::string_view{"Byte"},
    std::string_view{"Char"},
    std::string_view{"ClientType"},
    std::string_view{"Code"},
    std::string_view{"Codeunit"},
    std::string_view{"CommitBehavior"},
    std::string_view{"DataScope"},
    std::string_view{"Date"},
    std::string_view{"DateFormula"},
    std::string_view{"DateTime"},
    std::string_view{"Decimal"},
    std::string_view{"DefaultLayout"},
    std::string_view{"Dictionary"},
    std::string_view{"DotNet"},
    std::string_view{"Duration"},
    std::string_view{"Enum"},
    std::string_view{"ErrorBehavior"},
    std::string_view{"ErrorType"},
    std::string_view{"ExecutionContext"},
    std::string_view{"ExecutionMode"},
    std::string_view{"FieldClass"},
    std::string_view{"FieldRef"},
    std::string_view{"Guid"},
    std::string_view{"HttpRequestType"},
    std::string_view{"InStream"},
    std::string_view{"InherentPermissionsScope"},
    std::string_view{"Integer"},
    std::string_view{"Interface"},
    std::string_view{"IsolationLevel"},
    std::string_view{"KeyRef"},
    std::string_view{"List"},
    std::string_view{"Media"},
    std::string_view{"MediaSet"},
    std::string_view{"ModuleDependencyInfo"},
    std::string_view{"ModuleInfo"},
    std::string_view{"Notification"},
    std::string_view{"NotificationScope"},
    std::string_view{"ObjectType"},
    std::string_view{"Option"},
    std::string_view{"OutStream"},
    std::string_view{"Page"},
    std::string_view{"PageBackgroundTaskErrorLevel"},
    std::string_view{"PageStyle"},
    std::string_view{"PermissionObjectType"},
    std::string_view{"PromptMode"},
    std::string_view{"Query"},
    std::string_view{"Record"},
    std::string_view{"RecordId"},
    std::string_view{"RecordRef"},
    std::string_view{"Report"},
    std::string_view{"ReportFormat"},
    std::string_view{"ReportLayoutType"},
    std::string_view{"SecretText"},
    std::string_view{"SecurityFilter"},
    std::string_view{"SecurityOperationResult"},
    std::string_view{"TableConnectionType"},
    std::string_view{"TableFilter"},
    std::string_view{"TelemetryScope"},
    std::string_view{"TestAction"},
    std::string_view{"TestField"},
    std::string_view{"TestPage"},
    std::string_view{"TestPermissions"},
    std::string_view{"TestRequestPage"},
    std::string_view{"Text"},
    std::string_view{"TextBuilder"},
    std::string_view{"TextEncoding"},
    std::string_view{"Time"},
    std::string_view{"TransactionModel"},
    std::string_view{"TransactionType"},
    std::string_view{"Variant"},
    std::string_view{"Verbosity"},
    std::string_view{"Version"},
    std::string_view{"WebServiceActionResultCode"},
    std::string_view{"XmlPort"},
};

const std::string_view *CanonicalType(std::string_view alType) {
  const auto *const found = std::ranges::find_if(kCanonicalTypes, [alType](std::string_view known) {
    return known.size() == alType.size() &&
           std::ranges::equal(known, alType, [](char a, char b) { return Lower(a) == Lower(b); });
  });
  return found != kCanonicalTypes.end() ? found : nullptr;
}

} // namespace

std::string TypeName(std::string_view alType) {
  const std::string_view *const found = CanonicalType(alType);
  // An unknown type keeps its AL spelling, so that it fails at the `#include` under the name AL
  // gave it rather than under one this table invented.
  return found != nullptr ? std::string(*found) : std::string(alType);
}

bool IsAlTypeName(std::string_view alType) {
  return CanonicalType(alType) != nullptr;
}

namespace {

std::string_view KeywordOf(ObjectKind kind) {
  switch (kind) {
    case ObjectKind::Table: return "table";
    case ObjectKind::Codeunit: return "codeunit";
    case ObjectKind::Page: return "page";
    case ObjectKind::Report: return "report";
    case ObjectKind::Query: return "query";
    case ObjectKind::XmlPort: return "xmlport";
    case ObjectKind::Enum: return "enum";
    case ObjectKind::Interface: return "interface";
    case ObjectKind::PermissionSet: return "permissionset";
  }
  return {};
}

} // namespace

ObjectDeclaration DeclarationOf(std::string_view source, ObjectKind kind) {
  const std::string wanted = std::string(KeywordOf(kind)) + " ";
  const bool atStart = source.starts_with(wanted);
  const std::size_t found = atStart ? 0 : source.find("\n" + wanted);
  if (found == std::string_view::npos) { return {}; }

  const std::size_t at = atStart ? 0 : found + 1;
  const std::size_t eol = source.find('\n', at);
  const std::string_view line = source.substr(at, eol == std::string_view::npos ? eol : eol - at);

  ObjectDeclaration declared;
  declared.found = true;
  const std::size_t quote = line.find('"');
  if (quote != std::string_view::npos) {
    const std::size_t close = line.find('"', quote + 1);
    declared.name = std::string(line.substr(quote + 1, close - quote - 1));
  } else {
    std::istringstream words{std::string(line)};
    std::string word;
    words >> word >> word >> declared.name;
  }
  if (declared.name.empty()) { return {}; }

  // THE NAMESPACE MUST STAND ABOVE THE OBJECT. One found after it belongs to nothing this file
  // declares, and a declaration on line 1 can have none at all.
  const std::size_t ns = source.find("namespace ");
  if (ns != std::string_view::npos && ns < at) {
    const std::size_t end = source.find(';', ns);
    const std::size_t from = ns + std::string("namespace ").size();
    declared.nameSpace = std::string(source.substr(from, end - from));
  }
  return declared;
}

} // namespace agiru::gen
