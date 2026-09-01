#include "Names.h"

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

namespace {

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

} // namespace agiru::gen
