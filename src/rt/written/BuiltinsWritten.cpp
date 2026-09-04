#include "BuiltinsWritten.h"

#include "runtime/Error.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Integer.h"
#include "type/StringValue.h"

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {

constexpr ::agiru::Integer kDefaultChecksumModulus = 10;

std::string ConvertStr(std::string_view String,
                       std::string_view FromCharacters,
                       std::string_view ToCharacters) {
  if (FromCharacters.size() != ToCharacters.size()) {
    throw Error("ConvertStr: FromCharacters and ToCharacters must be the same length");
  }
  if (FromCharacters.empty()) { return std::string(String); }
  std::string out(String);
  for (char &c : out) {
    const std::size_t at = FromCharacters.find(c);
    if (at != std::string_view::npos) { c = ToCharacters[at]; }
  }
  return out;
}

std::string CopyStr(std::string_view String,
                    ::agiru::Integer Position,
                    std::optional<::agiru::Integer> Length) {
  if (Position < 1) { throw Error("CopyStr: the position must be 1 or more"); }
  if (Length.has_value() && *Length < 0) { throw Error("CopyStr: the length must be 0 or more"); }
  const auto at = static_cast<std::size_t>(Position);
  const std::size_t from = detail::ByteOfUnit(String, at);
  if (from >= String.size()) { return {}; }
  if (!Length.has_value()) { return std::string(String.substr(from)); }
  const std::size_t upto = detail::ByteOfUnit(String, at + static_cast<std::size_t>(*Length));
  return std::string(String.substr(from, upto - from));
}

std::string DelChr(std::string_view String,
                   std::optional<std::string_view> Where,
                   std::optional<std::string_view> Which) {
  const std::string_view where = Where.value_or("=");
  const std::string_view which = Which.value_or(" ");
  if (where.empty() || which.empty()) { return std::string(String); }
  if (where.find_first_not_of("=<>") != std::string_view::npos) {
    throw Error("DelChr: Where takes only '=', '<' and '>'");
  }
  if (where.find('=') != std::string_view::npos) {
    std::string out;
    for (const char c : String) {
      if (which.find(c) == std::string_view::npos) { out.push_back(c); }
    }
    return out;
  }
  std::size_t from = 0;
  std::size_t upto = String.size();
  if (where.find('<') != std::string_view::npos) {
    const std::size_t first = String.find_first_not_of(which);
    from = first == std::string_view::npos ? String.size() : first;
  }
  if (where.find('>') != std::string_view::npos) {
    const std::size_t last = String.find_last_not_of(which);
    upto = last == std::string_view::npos ? from : last + 1;
  }
  return std::string(String.substr(from, upto - from));
}

std::string
DelStr(std::string_view String, ::agiru::Integer Position, std::optional<::agiru::Integer> Length) {
  if (Position < 1) { throw Error("DelStr: the position must be 1 or more"); }
  if (Length.has_value() && *Length < 1) { throw Error("DelStr: the length must be 1 or more"); }
  const auto at = static_cast<std::size_t>(Position);
  const std::size_t from = detail::ByteOfUnit(String, at);
  if (from >= String.size()) { return std::string(String); }
  if (!Length.has_value()) { return std::string(String.substr(0, from)); }
  const std::size_t upto = detail::ByteOfUnit(String, at + static_cast<std::size_t>(*Length));
  return std::string(String.substr(0, from)) + std::string(String.substr(upto));
}

std::string IncStr(std::string_view String) {
  return IncStr(String, 1);
}

std::string IncStr(std::string_view String, ::agiru::BigInteger Increment) {
  const std::size_t last = String.find_last_of("0123456789");
  if (last == std::string_view::npos) { return {}; }
  std::size_t first = last;
  while (first > 0 && String[first - 1] >= '0' && String[first - 1] <= '9') { --first; }
  const std::string_view digits = String.substr(first, last - first + 1);
  const ::agiru::BigInteger value = std::stoll(std::string(digits)) + Increment;
  if (value < 0) { throw Error("IncStr: the result must be zero or positive"); }
  std::string grown = std::to_string(value);
  while (grown.size() < digits.size()) { grown.insert(grown.begin(), '0'); }
  return std::string(String.substr(0, first)) + grown + std::string(String.substr(last + 1));
}

std::string InsStr(std::string_view String, std::string_view SubString, ::agiru::Integer Position) {
  if (Position < 1) { throw Error("InsStr: the position must be 1 or more"); }
  const std::size_t at = detail::ByteOfUnit(String, static_cast<std::size_t>(Position));
  return std::string(String.substr(0, at)) + std::string(SubString) +
         std::string(String.substr(at));
}

std::string LowerCase(std::string_view String) {
  return detail::LowerText(String);
}

std::string PadStr(std::string_view String,
                   ::agiru::Integer Length,
                   std::optional<std::string_view> FillCharacter) {
  if (Length < 0) { throw Error("PadStr: the length must be 0 or more"); }
  const std::string_view filler = FillCharacter.value_or(" ");
  if (detail::Utf16Length(filler) != 1) {
    throw Error("PadStr: the fill character must be exactly one character");
  }
  const auto want = static_cast<std::size_t>(Length);
  const std::size_t have = detail::Utf16Length(String);
  if (have >= want) { return std::string(String.substr(0, detail::ByteOfUnit(String, want + 1))); }
  std::string out(String);
  for (std::size_t i = have; i < want; ++i) { out += filler; }
  return out;
}

std::string SelectStr(::agiru::Integer Number, std::string_view CommaString) {
  if (Number < 1) { throw Error("SelectStr: the number must be 1 or more"); }
  std::size_t at = 0;
  for (::agiru::Integer i = 1;; ++i) {
    const std::size_t comma = CommaString.find(',', at);
    if (i == Number) {
      return std::string(comma == std::string_view::npos ? CommaString.substr(at)
                                                         : CommaString.substr(at, comma - at));
    }
    if (comma == std::string_view::npos) {
      throw Error("SelectStr: the string holds fewer than " + std::to_string(Number) +
                  " substrings");
    }
    at = comma + 1;
  }
}

::agiru::Integer StrCheckSum(std::string_view String,
                             std::string_view WeightString,
                             std::optional<::agiru::Integer> Modulus) {
  if (String.empty()) { return 0; }
  const ::agiru::Integer modulus = Modulus.value_or(kDefaultChecksumModulus);
  if (modulus == 0) { throw Error("StrCheckSum: the modulus must not be zero"); }
  if (String.find_first_not_of("0123456789") != std::string_view::npos) {
    throw Error("StrCheckSum: the string must be digits only");
  }
  if (WeightString.find_first_not_of("0123456789") != std::string_view::npos) {
    throw Error("StrCheckSum: the weight must be digits only");
  }
  if (WeightString.size() > String.size()) {
    throw Error("StrCheckSum: the weight is longer than the string");
  }
  ::agiru::Integer total = 0;
  for (std::size_t i = 0; i < String.size(); ++i) {
    const ::agiru::Integer weight = i < WeightString.size() ? WeightString[i] - '0' : 1;
    total += (String[i] - '0') * weight;
  }
  return (modulus - (total % modulus)) % modulus;
}

::agiru::Integer StrPos(std::string_view String, std::string_view SubString) {
  if (SubString.empty()) { return 0; }
  return detail::IndexOfText(String, SubString, 1);
}

std::string UpperCase(std::string_view String) {
  return detail::UpperText(String);
}

::agiru::Date WorkDate(::agiru::Date NewDate) {
  Session &session = Session::Current();
  return NewDate.IsUndefined() ? session.WorkDate() : session.WorkDate(NewDate);
}

std::string CompanyName() {
  return std::string(Session::Current().CompanyName());
}

::agiru::Date Today() {
  return ::agiru::CurrentDateTime().Date();
}

}
