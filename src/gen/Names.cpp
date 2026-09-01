#include "Names.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>
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

std::string Identifier(std::string_view alName) {
  return Join(Words(alName));
}

std::string EnumeratorName(std::string_view optionMember) {
  const std::string name = Join(Words(optionMember));
  return name.empty() ? "Blank" : name;
}

std::string OptionEnumName(std::string_view tableName, std::string_view fieldName) {
  return Identifier(tableName) + Identifier(fieldName);
}

// AL TYPE NAMES ARE CASE-INSENSITIVE AND THE BASEAPP USES THAT. The same field type is written
// `Guid` 822 times and `GUID` 209 times, `BLOB` 163 times and `Blob` 54, `Enum` 1 351 times and
// `enum` 41 (measured 2026-09-01). Passing the spelling through would ask for eight door headers
// per type and would let `enum "Item Type"` slip past the enum path entirely. The canonical
// spelling is the one the platform documentation gives its data-type page -- `RecordId`, not
// `RecordID`.
std::string TypeName(std::string_view alType) {
  static constexpr std::array kCanonical{
      std::string_view{"BigInteger"},  std::string_view{"Blob"},
      std::string_view{"Boolean"},     std::string_view{"Byte"},
      std::string_view{"Code"},        std::string_view{"Date"},
      std::string_view{"DateFormula"}, std::string_view{"DateTime"},
      std::string_view{"Decimal"},     std::string_view{"Duration"},
      std::string_view{"Enum"},        std::string_view{"Guid"},
      std::string_view{"Integer"},     std::string_view{"Media"},
      std::string_view{"MediaSet"},    std::string_view{"Option"},
      std::string_view{"RecordId"},    std::string_view{"TableFilter"},
      std::string_view{"Text"},        std::string_view{"Time"},
  };
  const auto *const found = std::ranges::find_if(kCanonical, [alType](std::string_view known) {
    return known.size() == alType.size() &&
           std::ranges::equal(known, alType, [](char a, char b) { return Lower(a) == Lower(b); });
  });
  // An unknown type keeps its AL spelling, so that it fails at the `#include` under the name AL
  // gave it rather than under one this table invented.
  return found != kCanonical.end() ? std::string(*found) : std::string(alType);
}

} // namespace agiru::gen
