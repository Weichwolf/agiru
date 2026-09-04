#include "BuiltinsWritten.h"

#include "meta/EnumDef.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/StringValue.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

constexpr ::agiru::Integer kDefaultChecksumModulus = 10;

namespace {

constexpr ::agiru::Integer kDisplayFormat = 0;
constexpr ::agiru::Integer kEditFormat = 1;
constexpr ::agiru::Integer kCodeFormat = 2;
constexpr ::agiru::Integer kXmlFormat = 9;

constexpr int kTwoDigitYear = 100;

bool AsText(::agiru::Integer format) {
  return format == kDisplayFormat || format == kEditFormat;
}

std::string OrdinalText(const ::agiru::OrdinalInVariant &held, ::agiru::Integer format) {
  if (!AsText(format)) { return std::to_string(held.ordinal); }
  const EnumValueDef *member = ValueOf(held.values, held.ordinal);
  if (member == nullptr) { return std::to_string(held.ordinal); }
  return std::string(member->caption.empty() ? member->name : member->caption);
}

std::string DateText(const ::agiru::Date &held, ::agiru::Integer format) {
  if (format != kCodeFormat) { return held.ToInvariantString(); }
  if (held.IsUndefined()) { return {}; }
  return std::format("{:02}{:02}{:02}{}",
                     held.Month(),
                     held.Day(),
                     held.Year() % kTwoDigitYear,
                     held.IsClosing() ? "CD" : "D");
}

std::string TimeText(const ::agiru::Time &held, ::agiru::Integer format) {
  if (format != kCodeFormat) { return held.ToInvariantString(); }
  return std::format(
      "{:02}{:02}{:02}.{:03}T", held.Hour(), held.Minute(), held.Second(), held.Millisecond());
}

std::string Rendered(const ::agiru::Variant &Value, ::agiru::Integer format) {
  if (Value.IsEmpty()) { throw Error("Format: the Variant holds no value"); }
  if (Value.Is<Boolean>()) {
    if (AsText(format)) { return ToText(Value.Get<Boolean>()); }
    if (format == kXmlFormat) { return Value.Get<Boolean>() ? "true" : "false"; }
    return Value.Get<Boolean>() ? "1" : "0";
  }
  if (Value.Is<::agiru::Integer>()) { return ToText(Value.Get<::agiru::Integer>()); }
  if (Value.Is<BigInteger>()) { return ToText(Value.Get<BigInteger>()); }
  if (Value.Is<Decimal>()) { return Value.Get<Decimal>().ToInvariantString(); }
  if (Value.Is<std::string>()) { return Value.Get<std::string>(); }
  if (Value.Is<Date>()) { return DateText(Value.Get<Date>(), format); }
  if (Value.Is<Time>()) { return TimeText(Value.Get<Time>(), format); }
  if (Value.Is<DateTime>()) { return Value.Get<DateTime>().ToInvariantString(); }
  if (Value.Is<Duration>()) { return Value.Get<Duration>().ToInvariantString(); }
  if (Value.Is<Guid>()) { return Value.Get<Guid>().ToText(); }
  if (Value.Is<RecordId>()) { return Value.Get<RecordId>().ToText(); }
  if (Value.Is<DateFormula>()) { return Value.Get<DateFormula>().ToText(); }
  if (Value.Is<OrdinalInVariant>()) { return OrdinalText(Value.Get<OrdinalInVariant>(), format); }
  throw Error("Format: a Variant holding a record renders its primary key, which needs a key");
}

std::string Fitted(std::string rendered, ::agiru::Integer Length, bool numeric, char filler) {
  if (Length <= 0) { return rendered; }
  const auto wanted = static_cast<std::size_t>(Length);
  if (rendered.size() > wanted) {
    rendered.resize(wanted);
    return rendered;
  }
  const std::string pad(wanted - rendered.size(), filler);
  return numeric ? pad + rendered : rendered + pad;
}

bool Numeric(const ::agiru::Variant &Value) {
  return Value.Is<::agiru::Integer>() || Value.Is<BigInteger>() || Value.Is<Decimal>();
}

struct Token {
  std::string name;
  std::string argument;
  bool literal;
};

std::vector<Token> Parsed(std::string_view spec) {
  std::vector<Token> tokens;
  std::string literal;
  for (std::size_t at = 0; at < spec.size(); ++at) {
    const std::size_t close = spec[at] == '<' ? spec.find('>', at) : std::string_view::npos;
    if (close == std::string_view::npos) {
      literal.push_back(spec[at]);
      continue;
    }
    if (!literal.empty()) {
      tokens.push_back({.name = literal, .argument = {}, .literal = true});
      literal.clear();
    }
    const std::string_view body = spec.substr(at + 1, close - at - 1);
    const std::size_t comma = body.find(',');
    std::string name(comma == std::string_view::npos ? body : body.substr(0, comma));
    for (char &c : name) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
    while (!name.empty() && name.back() == ' ') { name.pop_back(); }
    tokens.push_back({.name = name,
                      .argument = comma == std::string_view::npos
                                      ? std::string{}
                                      : std::string(body.substr(comma + 1)),
                      .literal = false});
    at = close;
  }
  if (!literal.empty()) { tokens.push_back({.name = literal, .argument = {}, .literal = true}); }
  return tokens;
}

std::int32_t Number(std::string_view text) {
  std::int32_t value = 0;
  for (const char c : text) {
    if (std::isdigit(static_cast<unsigned char>(c)) == 0) { return 0; }
    value = value * 10 + (c - '0');
  }
  return value;
}

bool IsFiller(const Token &token) {
  return token.name == "filler character" || token.name == "filler char";
}

std::int64_t WholeOf(const ::agiru::Variant &Value) {
  if (Value.Is<::agiru::Integer>()) { return Value.Get<::agiru::Integer>(); }
  if (Value.Is<BigInteger>()) { return Value.Get<BigInteger>(); }
  throw Error("Format: <Integer> and <Sign> want a whole number");
}

bool IsDateElement(std::string_view name) {
  return name == "day" || name == "month" || name == "year" || name == "year4";
}

bool IsTimeElement(std::string_view name) {
  return name == "hours24" || name == "minutes" || name == "seconds";
}

std::string DateElement(const ::agiru::Variant &Value, const Token &token, char filler) {
  if (!Value.Is<Date>()) { throw Error("Format: a date element wants a date"); }
  const Date held = Value.Get<Date>();
  const std::int32_t part = token.name == "day"     ? held.Day()
                            : token.name == "month" ? held.Month()
                            : token.name == "year4" ? held.Year()
                                                    : held.Year() % kTwoDigitYear;
  return Fitted(
      std::to_string(part), token.name == "year4" ? 0 : Number(token.argument), true, filler);
}

std::string TimeElement(const ::agiru::Variant &Value, const Token &token, char filler) {
  if (!Value.Is<Time>()) { throw Error("Format: a time element wants a time"); }
  const Time held = Value.Get<Time>();
  const std::int32_t part = token.name == "hours24"   ? held.Hour()
                            : token.name == "minutes" ? held.Minute()
                                                      : held.Second();
  return Fitted(std::to_string(part), Number(token.argument), true, filler);
}

std::string SpecToken(const ::agiru::Variant &Value, const Token &token, char filler) {
  const std::int32_t width = Number(token.argument);
  if (token.name == "integer") {
    const std::int64_t whole = WholeOf(Value);
    return Fitted(std::to_string(whole < 0 ? -whole : whole), width, true, filler);
  }
  if (token.name == "sign") { return WholeOf(Value) < 0 ? "-" : ""; }
  if (token.name == "text") {
    return Fitted(Rendered(Value, kDisplayFormat), width, false, filler);
  }
  if (token.name == "standard format") { return Rendered(Value, Number(token.argument)); }
  if (IsDateElement(token.name)) { return DateElement(Value, token, filler); }
  if (IsTimeElement(token.name)) { return TimeElement(Value, token, filler); }
  throw Error("Format: the format element <" + token.name +
              "> is declared by devenv-format-property.md and not implemented yet (board:0007)");
}

}

std::string
Format(const ::agiru::Variant &Value, ::agiru::Integer Length, ::agiru::Integer FormatNumber) {
  if (FormatNumber != kDisplayFormat && FormatNumber != kEditFormat &&
      FormatNumber != kCodeFormat && FormatNumber != kXmlFormat) {
    throw Error("Format: standard format " + std::to_string(FormatNumber) +
                " is not one devenv-format-property.md tabulates");
  }
  return Fitted(Rendered(Value, FormatNumber), Length, Numeric(Value), ' ');
}

std::string
Format(const ::agiru::Variant &Value, ::agiru::Integer Length, std::string_view FormatString) {
  const std::vector<Token> tokens = Parsed(FormatString);
  char filler = ' ';
  for (const Token &token : tokens) {
    if (!token.literal && IsFiller(token) && !token.argument.empty()) {
      filler = token.argument.front();
    }
  }
  std::string rendered;
  bool numeric = false;
  for (const Token &token : tokens) {
    if (token.literal) {
      rendered += token.name;
      continue;
    }
    if (IsFiller(token)) { continue; }
    numeric = numeric || token.name == "integer" || token.name == "sign";
    rendered += SpecToken(Value, token, filler);
  }
  return Fitted(std::move(rendered), Length, numeric, filler);
}

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
