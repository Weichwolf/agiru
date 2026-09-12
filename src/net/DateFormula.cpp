#include "type/DateFormula.h"

#include "type/Date.h"
#include "type/Integer.h"
#include "type/Language.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

constexpr std::string_view kNotAFormula =
    "a date formula is a sign, a number and a unit, and this character is none of them";

constexpr int kMonthsPerQuarter = 3;
constexpr int kMonthsPerYear = 12;
constexpr int kDaysPerWeek = 7;
constexpr int kIsoMonday = 1;
constexpr int kIsoSunday = 7;
constexpr unsigned kLastOfDecember = 31;
constexpr int kDecimalBase = 10;

struct Letters {
  std::string_view current;
  std::string_view day;
  std::string_view week;
  std::string_view month;
  std::string_view quarter;
  std::string_view year;
  std::string_view weekday;
};

constexpr Letters kInvariant{"C", "D", "W", "M", "Q", "Y", "WD"};

constexpr Letters kPacked{"\x01", "\x02", "\x04", "\x05", "\x06", "\x07", "\x03"};

constexpr Integer kPrimaryLanguageMask = 0x3FF;
constexpr Integer kGerman = 0x07;
constexpr Integer kDanish = 0x06;
constexpr Integer kDutch = 0x13;
constexpr Integer kFrench = 0x0C;
constexpr Integer kSpanish = 0x0A;
constexpr Integer kSwedish = 0x1D;
constexpr Integer kNorwegian = 0x14;

struct LanguageLetters {
  Integer primary;
  Letters letters;
};

constexpr std::array<LanguageLetters, 7> kByLanguage{{
    {kGerman, {"L", "T", "W", "M", "Q", "J", "WT"}},
    {kDanish, {"L", "D", "U", "M", "K", "Å", "UD"}},
    {kDutch, {"H", "D", "W", "M", "K", "J", "WD"}},
    {kFrench, {"C", "J", "S", "M", "T", "A", "WD"}},
    {kSpanish, {"C", "D", "S", "M", "T", "A", "WD"}},
    {kSwedish, {"L", "D", "V", "M", "K", "Å", "VD"}},
    {kNorwegian, {"L", "D", "U", "M", "K", "Å", "UD"}},
}};

const Letters &LettersOf(Integer language) {
  const Integer primary = language & kPrimaryLanguageMask;
  for (const LanguageLetters &entry : kByLanguage) {
    if (entry.primary == primary) { return entry.letters; }
  }
  return kInvariant;
}

constexpr unsigned char kLatin1Lead = 0xC3;
constexpr unsigned char kLatin1LowerFirst = 0xA0;
constexpr unsigned char kLatin1LowerLast = 0xBE;
constexpr unsigned char kLatin1Division = 0xB7;
constexpr unsigned char kLatin1CaseDistance = 0x20;

unsigned char Folded(unsigned char c, bool afterLead) {
  if (afterLead) {
    if (c >= kLatin1LowerFirst && c <= kLatin1LowerLast && c != kLatin1Division) {
      return static_cast<unsigned char>(c - kLatin1CaseDistance);
    }
    return c;
  }
  return static_cast<unsigned char>(std::toupper(c));
}

bool TokenAt(std::string_view text, std::size_t at, std::string_view token) {
  if (token.empty() || at + token.size() > text.size()) { return false; }
  bool afterLead = false;
  for (std::size_t i = 0; i < token.size(); ++i) {
    const auto c = static_cast<unsigned char>(text[at + i]);
    if (Folded(c, afterLead) != static_cast<unsigned char>(token[i])) { return false; }
    afterLead = c == kLatin1Lead;
  }
  return true;
}

bool DigitAt(std::string_view text, std::size_t at) {
  return at < text.size() && (std::isdigit(static_cast<unsigned char>(text[at])) != 0);
}

struct UnitLetter {
  char unit;
  std::string_view Letters::*letter;
};

constexpr std::array<UnitLetter, 5> kUnits{{{'D', &Letters::day},
                                            {'W', &Letters::week},
                                            {'M', &Letters::month},
                                            {'Q', &Letters::quarter},
                                            {'Y', &Letters::year}}};

std::string_view Letter(const Letters &letters, char unit) {
  for (const UnitLetter &entry : kUnits) {
    if (entry.unit == unit) { return letters.*entry.letter; }
  }
  return {};
}

struct UnitAt {
  char unit;
  std::size_t length;
};

std::optional<UnitAt> UnitAtIn(std::string_view text, std::size_t at, const Letters &letters) {
  for (const UnitLetter &entry : kUnits) {
    const std::string_view token = letters.*entry.letter;
    if (TokenAt(text, at, token)) { return UnitAt{.unit = entry.unit, .length = token.size()}; }
  }
  return std::nullopt;
}

std::optional<UnitAt>
UnitAtEither(std::string_view text, std::size_t at, const Letters &localized) {
  if (const std::optional<UnitAt> found = UnitAtIn(text, at, localized); found.has_value()) {
    return found;
  }
  return UnitAtIn(text, at, kInvariant);
}

enum class Piece : std::uint8_t { Period, Weekday, Week, DayOfMonth, Amount };

struct Token {
  Piece piece = Piece::Amount;
  char sign = 0;
  std::string number;
  char unit = 'D';
};

std::string TakeDigits(std::string_view text, std::size_t &at) {
  std::string digits;
  while (DigitAt(text, at)) {
    digits += text[at];
    ++at;
  }
  return digits;
}

std::expected<std::vector<Token>, Refusal> Tokens(std::string_view text, const Letters &localized) {
  std::vector<Token> out;
  std::size_t at = 0;
  char sign = 0;
  while (at < text.size()) {
    const auto c = static_cast<unsigned char>(text[at]);
    if (c == '+' || c == '-') {
      sign = static_cast<char>(c);
      ++at;
      continue;
    }
    if (std::isspace(c) != 0) {
      ++at;
      continue;
    }
    std::size_t length = 0;
    if (TokenAt(text, at, localized.weekday)) {
      length = localized.weekday.size();
    } else if (TokenAt(text, at, kInvariant.weekday)) {
      length = kInvariant.weekday.size();
    }
    if (length != 0 && DigitAt(text, at + length)) {
      at += length;
      out.push_back(Token{.piece = Piece::Weekday, .sign = sign, .number = TakeDigits(text, at)});
      sign = 0;
      continue;
    }
    length = 0;
    if (TokenAt(text, at, localized.current)) {
      length = localized.current.size();
    } else if (TokenAt(text, at, kInvariant.current)) {
      length = kInvariant.current.size();
    }
    if (length != 0) {
      const std::optional<UnitAt> unit = UnitAtEither(text, at + length, localized);
      if (unit.has_value()) {
        at += length + unit->length;
        out.push_back(
            Token{.piece = Piece::Period, .sign = sign, .number = {}, .unit = unit->unit});
        sign = 0;
        continue;
      }
    }
    const std::optional<UnitAt> selector = UnitAtEither(text, at, localized);
    if (selector.has_value() && (selector->unit == 'W' || selector->unit == 'D') &&
        DigitAt(text, at + selector->length)) {
      at += selector->length;
      out.push_back(Token{.piece = selector->unit == 'W' ? Piece::Week : Piece::DayOfMonth,
                          .sign = sign,
                          .number = TakeDigits(text, at)});
      sign = 0;
      continue;
    }
    const std::string number = TakeDigits(text, at);
    const std::optional<UnitAt> unit = UnitAtEither(text, at, localized);
    if (unit.has_value()) {
      at += unit->length;
      out.push_back(
          Token{.piece = Piece::Amount, .sign = sign, .number = number, .unit = unit->unit});
      sign = 0;
      continue;
    }
    return std::unexpected(Refusal{.what = kNotAFormula, .at = at + 1});
  }
  return out;
}

std::string Rendered(const std::vector<Token> &tokens, const Letters &letters) {
  std::string out;
  for (const Token &token : tokens) {
    if (token.sign != 0) { out += token.sign; }
    switch (token.piece) {
      case Piece::Period:
        out += letters.current;
        out += Letter(letters, token.unit);
        break;
      case Piece::Weekday:
        out += letters.weekday;
        out += token.number;
        break;
      case Piece::Week:
        out += letters.week;
        out += token.number;
        break;
      case Piece::DayOfMonth:
        out += letters.day;
        out += token.number;
        break;
      case Piece::Amount:
        out += token.number;
        out += Letter(letters, token.unit);
        break;
    }
  }
  return out;
}

int Counted(const std::string &digits) {
  int n = 0;
  for (const char c : digits) { n = (n * kDecimalBase) + (c - '0'); }
  return n;
}

constexpr unsigned char kFirstPackedUnit = 1;
constexpr unsigned char kLastPackedUnit = 7;

bool IsPacked(std::string_view text) {
  for (const char c : text) {
    const auto byte = static_cast<unsigned char>(c);
    if (byte >= kFirstPackedUnit && byte <= kLastPackedUnit) { return true; }
  }
  return false;
}

std::string Unpacked(std::string_view text) {
  constexpr std::array<std::string_view, kLastPackedUnit + 1> kLetters{
      "", "C", "D", "WD", "W", "M", "Q", "Y"};
  std::string out;
  for (const char c : text) {
    const auto byte = static_cast<unsigned char>(c);
    if (byte >= kFirstPackedUnit && byte <= kLastPackedUnit) {
      out += kLetters[byte];
    } else {
      out += c;
    }
  }
  return out;
}

Date Boundary(const Date &d, char unit, bool first) {
  const int year = d.Year();
  const int month = d.Month();
  switch (unit) {
    case 'D': return d;
    case 'W': {
      const int weekday = d.DayOfWeek();
      const int shift = first ? kIsoMonday - weekday : kIsoSunday - weekday;
      return Date::FromDaysSinceFirst(d.DaysSinceFirst() + shift);
    }
    case 'M': {
      if (first) { return Date::FromYmd(year, static_cast<unsigned>(month), 1); }
      const int nextMonth = month == kMonthsPerYear ? 1 : month + 1;
      const int nextYear = month == kMonthsPerYear ? year + 1 : year;
      return Date::FromDaysSinceFirst(
          Date::FromYmd(nextYear, static_cast<unsigned>(nextMonth), 1).DaysSinceFirst() - 1);
    }
    case 'Q': {
      const int quarter = (month - 1) / kMonthsPerQuarter;
      if (first) {
        return Date::FromYmd(year, static_cast<unsigned>((quarter * kMonthsPerQuarter) + 1), 1);
      }
      const int endMonth = (quarter * kMonthsPerQuarter) + kMonthsPerQuarter;
      const int nextMonth = endMonth == kMonthsPerYear ? 1 : endMonth + 1;
      const int nextYear = endMonth == kMonthsPerYear ? year + 1 : year;
      return Date::FromDaysSinceFirst(
          Date::FromYmd(nextYear, static_cast<unsigned>(nextMonth), 1).DaysSinceFirst() - 1);
    }
    case 'Y':
      return first ? Date::FromYmd(year, 1, 1)
                   : Date::FromYmd(year, kMonthsPerYear, kLastOfDecember);
    default: return d;
  }
}

Date AddMonths(const Date &d, int months) {
  const int total = ((d.Year() * kMonthsPerYear) + d.Month() - 1) + months;
  const int year = total / kMonthsPerYear;
  const int month = (total % kMonthsPerYear) + 1;
  const auto length =
      static_cast<int>(calendar::LastDayOfMonth(year, static_cast<unsigned>(month)));
  return Date::FromYmd(year,
                       static_cast<unsigned>(month),
                       static_cast<unsigned>(d.Day() < length ? d.Day() : length));
}

Date DayOfMonth(const Date &d, int day) {
  if (day < 1) { return d; }
  const auto length =
      static_cast<int>(calendar::LastDayOfMonth(d.Year(), static_cast<unsigned>(d.Month())));
  return Date::FromYmd(d.Year(),
                       static_cast<unsigned>(d.Month()),
                       static_cast<unsigned>(day < length ? day : length));
}

Date Weekday(const Date &d, int target, bool backwards) {
  if (target < kIsoMonday || target > kIsoSunday) { return d; }
  const int today = d.DayOfWeek();
  int shift = 0;
  if (backwards) {
    const int back = (today - target) % kDaysPerWeek;
    shift = -(back == 0 ? kDaysPerWeek : back);
  } else {
    const int forward = (target - today) % kDaysPerWeek;
    shift = forward == 0 ? kDaysPerWeek : forward;
  }
  return Date::FromDaysSinceFirst(d.DaysSinceFirst() + shift);
}

}

std::expected<DateFormula, Refusal> DateFormula::FromText(std::string_view text) {
  DateFormula formula;
  bool invariantOnly = false;
  if (!text.empty() && text.front() == '<' && text.back() == '>') {
    text.remove_prefix(1);
    text.remove_suffix(1);
    invariantOnly = true;
  }
  std::string unpacked;
  if (IsPacked(text)) {
    unpacked = Unpacked(text);
    text = unpacked;
    invariantOnly = true;
  }
  const Letters &localized = invariantOnly ? kInvariant : LettersOf(Language::Current());
  const std::expected<std::vector<Token>, Refusal> tokens = Tokens(text, localized);
  if (!tokens.has_value()) { return std::unexpected(tokens.error()); }
  for (const Token &token : *tokens) {
    const bool negative = token.sign == '-';
    switch (token.piece) {
      case Piece::Period:
        formula.terms_.push_back(
            Term{.kind = Kind::Period, .unit = token.unit, .count = 0, .negative = negative});
        break;
      case Piece::Weekday:
        formula.terms_.push_back(Term{.kind = Kind::Weekday,
                                      .unit = 'D',
                                      .count = Counted(token.number),
                                      .negative = negative});
        break;
      case Piece::Week:
        formula.terms_.push_back(Term{
            .kind = Kind::Week, .unit = 'W', .count = Counted(token.number), .negative = negative});
        break;
      case Piece::DayOfMonth:
        formula.terms_.push_back(Term{.kind = Kind::DayOfMonth,
                                      .unit = 'D',
                                      .count = Counted(token.number),
                                      .negative = negative});
        break;
      case Piece::Amount:
        formula.terms_.push_back(Term{.kind = Kind::Amount,
                                      .unit = token.unit,
                                      .count = token.number.empty() ? 1 : Counted(token.number),
                                      .negative = negative});
        break;
    }
  }
  formula.text_ = Rendered(*tokens, kInvariant);
  return formula;
}

std::string DateFormula::ToText() const {
  const Letters &letters = LettersOf(Language::Current());
  if (&letters == &kInvariant) { return text_; }
  const std::expected<std::vector<Token>, Refusal> tokens = Tokens(text_, kInvariant);
  if (!tokens.has_value()) { return text_; }
  return Rendered(*tokens, letters);
}

std::string DateFormula::ToStorageText() const {
  const std::expected<std::vector<Token>, Refusal> tokens = Tokens(text_, kInvariant);
  if (!tokens.has_value()) { return text_; }
  return Rendered(*tokens, kPacked);
}

Date DateFormula::CalcDate(const Date &from) const {
  if (from.IsUndefined()) { return from; }
  Date d = from;
  for (const Term &term : terms_) {
    const int signed_ = term.negative ? -term.count : term.count;
    switch (term.kind) {
      case Kind::Period: d = Boundary(d, term.unit, term.negative); break;
      case Kind::Weekday: d = Weekday(d, term.count, term.negative); break;
      case Kind::Week: {
        const Date january4 = Date::FromYmd(d.Year(), 1, 4);
        const Date firstMonday =
            Date::FromDaysSinceFirst(january4.DaysSinceFirst() - (january4.DayOfWeek() - 1));
        d = Date::FromDaysSinceFirst(firstMonday.DaysSinceFirst() +
                                     ((term.count - 1) * kDaysPerWeek));
        break;
      }
      case Kind::DayOfMonth: d = DayOfMonth(d, term.count); break;
      case Kind::Amount:
        switch (term.unit) {
          case 'D': d = Date::FromDaysSinceFirst(d.DaysSinceFirst() + signed_); break;
          case 'W':
            d = Date::FromDaysSinceFirst(d.DaysSinceFirst() + (signed_ * kDaysPerWeek));
            break;
          case 'M': d = AddMonths(d, signed_); break;
          case 'Q': d = AddMonths(d, signed_ * kMonthsPerQuarter); break;
          case 'Y': d = AddMonths(d, signed_ * kMonthsPerYear); break;
          default: break;
        }
        break;
    }
  }
  return d;
}

}
