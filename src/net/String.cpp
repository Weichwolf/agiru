#include "dotnet/String.h"

#include "runtime/Error.h"
#include "type/Variant.h"

#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::dotnet {

namespace {

std::vector<std::string> TextsOf(const Array &chars) {
  std::vector<std::string> out;
  for (const ::agiru::Variant &held : chars) { out.emplace_back(std::string_view(held)); }
  return out;
}

bool StartsWithAnyOf(std::string_view text, const std::vector<std::string> &pieces, std::size_t &length) {
  for (const std::string &piece : pieces) {
    if (!piece.empty() && text.starts_with(piece)) {
      length = piece.size();
      return true;
    }
  }
  return false;
}

Array SplitOn(std::string_view text, const std::vector<std::string> &pieces) {
  Array out;
  std::string current;
  std::size_t at = 0;
  while (at < text.size()) {
    std::size_t length = 0;
    if (StartsWithAnyOf(text.substr(at), pieces, length)) {
      out.Add(::agiru::Variant(current));
      current.clear();
      at += length;
      continue;
    }
    current += text[at];
    ++at;
  }
  out.Add(::agiru::Variant(current));
  return out;
}

std::vector<std::string> EachCharacter(std::string_view text) {
  std::vector<std::string> out;
  std::size_t at = 0;
  while (at < text.size()) {
    const auto lead = static_cast<unsigned char>(text[at]);
    std::size_t length = 1;
    if (lead >= 0xF0U) {
      length = 4;
    } else if (lead >= 0xE0U) {
      length = 3;
    } else if (lead >= 0xC0U) {
      length = 2;
    }
    if (at + length > text.size()) { length = text.size() - at; }
    out.emplace_back(text.substr(at, length));
    at += length;
  }
  return out;
}

bool IsSpace(char c) {
  return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::string_view TrimmedStart(std::string_view text, const std::vector<std::string> *chars) {
  while (!text.empty()) {
    std::size_t length = 0;
    if (chars == nullptr ? IsSpace(text.front()) : StartsWithAnyOf(text, *chars, length)) {
      text.remove_prefix(chars == nullptr ? 1 : length);
      continue;
    }
    break;
  }
  return text;
}

std::string_view TrimmedEnd(std::string_view text, const std::vector<std::string> *chars) {
  while (!text.empty()) {
    if (chars == nullptr) {
      if (!IsSpace(text.back())) { break; }
      text.remove_suffix(1);
      continue;
    }
    bool cut = false;
    for (const std::string &piece : *chars) {
      if (!piece.empty() && text.ends_with(piece)) {
        text.remove_suffix(piece.size());
        cut = true;
        break;
      }
    }
    if (!cut) { break; }
  }
  return text;
}

String Of(std::string_view text) {
  String made;
  made = text;
  return made;
}

std::string_view Sv(const ::agiru::Text<0> &text) {
  return std::string_view(text);
}

::agiru::Integer FirstOfAny(std::string_view text, const std::vector<std::string> &pieces) {
  ::agiru::Integer best = -1;
  for (const std::string &piece : pieces) {
    if (piece.empty()) { continue; }
    const std::size_t at = text.find(piece);
    if (at != std::string_view::npos && (best < 0 || static_cast<::agiru::Integer>(at) < best)) {
      best = static_cast<::agiru::Integer>(at);
    }
  }
  return best;
}

void CheckRange(std::string_view text, ::agiru::Integer startIndex, ::agiru::Integer length) {
  if (startIndex < 0 || length < 0 ||
      static_cast<std::size_t>(startIndex) + static_cast<std::size_t>(length) > text.size()) {
    throw Error("String: the index and length are outside the string (ArgumentOutOfRangeException)");
  }
}

}

String String::Binder::operator()(const Array &chars) const {
  std::string joined;
  for (const ::agiru::Variant &held : chars) { joined += std::string_view(held); }
  return Of(joined);
}

String String::Binder::operator()(std::string_view text) const {
  return Of(text);
}

String String::Binder::operator()(const Refused &refused) const {
  static_cast<void>(refused());
  return {};
}

::agiru::Char String::Chars(::agiru::Integer index) const {
  CheckRange(Sv(value_), index, 1);
  return ::agiru::Char{Sv(value_).substr(static_cast<std::size_t>(index), 1)};
}

Array String::Split(const Array &separators) const {
  return SplitOn(Sv(value_), TextsOf(separators));
}

Array String::Split(std::string_view separators) const {
  return SplitOn(Sv(value_), EachCharacter(separators));
}

Array String::ToCharArray() const {
  Array out;
  for (const std::string &one : EachCharacter(Sv(value_))) { out.Add(::agiru::Variant(one)); }
  return out;
}

Array String::ToCharArray(::agiru::Integer startIndex, ::agiru::Integer length) const {
  CheckRange(Sv(value_), startIndex, length);
  Array out;
  for (const std::string &one : EachCharacter(
           Sv(value_).substr(static_cast<std::size_t>(startIndex), static_cast<std::size_t>(length)))) {
    out.Add(::agiru::Variant(one));
  }
  return out;
}

String String::Replace(std::string_view oldValue, std::string_view newValue) const {
  if (oldValue.empty()) { throw Error("String.Replace: the value to replace is empty (ArgumentException)"); }
  std::string out;
  const std::string_view text = Sv(value_);
  std::size_t at = 0;
  while (at < text.size()) {
    if (text.substr(at).starts_with(oldValue)) {
      out += newValue;
      at += oldValue.size();
      continue;
    }
    out += text[at];
    ++at;
  }
  return Of(out);
}

String String::Trim() const {
  return Of(TrimmedEnd(TrimmedStart(Sv(value_), nullptr), nullptr));
}

String String::Trim(const Array &chars) const {
  const std::vector<std::string> pieces = TextsOf(chars);
  return Of(TrimmedEnd(TrimmedStart(Sv(value_), &pieces), &pieces));
}

String String::TrimStart() const {
  return Of(TrimmedStart(Sv(value_), nullptr));
}

String String::TrimStart(const Array &chars) const {
  const std::vector<std::string> pieces = TextsOf(chars);
  return Of(TrimmedStart(Sv(value_), &pieces));
}

String String::TrimEnd() const {
  return Of(TrimmedEnd(Sv(value_), nullptr));
}

String String::TrimEnd(const Array &chars) const {
  const std::vector<std::string> pieces = TextsOf(chars);
  return Of(TrimmedEnd(Sv(value_), &pieces));
}

String String::PadLeft(::agiru::Integer totalWidth, ::agiru::Char paddingChar) const {
  const std::size_t want = totalWidth < 0 ? 0 : static_cast<std::size_t>(totalWidth);
  std::string out;
  const std::string filler = ::agiru::Encoded(paddingChar);
  for (std::size_t have = Sv(value_).size(); have < want; ++have) { out += filler; }
  out += Sv(value_);
  return Of(out);
}

String String::PadRight(::agiru::Integer totalWidth, ::agiru::Char paddingChar) const {
  const std::size_t want = totalWidth < 0 ? 0 : static_cast<std::size_t>(totalWidth);
  std::string out(Sv(value_));
  const std::string filler = ::agiru::Encoded(paddingChar);
  for (std::size_t have = Sv(value_).size(); have < want; ++have) { out += filler; }
  return Of(out);
}

::agiru::Integer String::IndexOf(std::string_view value, ::agiru::Integer startIndex) const {
  if (startIndex < 0 || static_cast<std::size_t>(startIndex) > Sv(value_).size()) {
    throw Error("String.IndexOf: the start index is outside the string (ArgumentOutOfRangeException)");
  }
  const std::size_t at = Sv(value_).find(value, static_cast<std::size_t>(startIndex));
  return at == std::string_view::npos ? ::agiru::Integer{-1} : static_cast<::agiru::Integer>(at);
}

::agiru::Integer String::IndexOfAny(const Array &chars) const {
  return FirstOfAny(Sv(value_), TextsOf(chars));
}

::agiru::Integer String::IndexOfAny(std::string_view chars) const {
  return FirstOfAny(Sv(value_), EachCharacter(chars));
}

::agiru::Integer String::LastIndexOf(std::string_view value) const {
  const std::size_t at = Sv(value_).rfind(value);
  return at == std::string_view::npos ? ::agiru::Integer{-1} : static_cast<::agiru::Integer>(at);
}

String String::Substring(::agiru::Integer startIndex) const {
  CheckRange(Sv(value_), startIndex, 0);
  return Of(Sv(value_).substr(static_cast<std::size_t>(startIndex)));
}

String String::Substring(::agiru::Integer startIndex, ::agiru::Integer length) const {
  CheckRange(Sv(value_), startIndex, length);
  return Of(Sv(value_).substr(static_cast<std::size_t>(startIndex),
                                            static_cast<std::size_t>(length)));
}

}
