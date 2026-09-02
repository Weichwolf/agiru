#include "type/DateFormula.h"

#include "type/Date.h"

#include <cctype>
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

namespace agiru {

namespace {

constexpr int kMonthsPerQuarter = 3;
constexpr int kMonthsPerYear = 12;
constexpr int kDaysPerWeek = 7;
constexpr int kIsoMonday = 1;
constexpr int kIsoSunday = 7;
constexpr unsigned kLastOfDecember = 31;

/// Reads the digits at `at`, leaving `at` past them.
int TakeNumber(std::string_view text, std::size_t &at) {
  int n = 0;
  while (at < text.size() && (std::isdigit(static_cast<unsigned char>(text[at])) != 0)) {
    n = (n * 10) + (text[at] - '0');
    ++at;
  }
  return n;
}

char Upper(char c) {
  return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

bool IsUnit(char c) {
  return c == 'D' || c == 'W' || c == 'M' || c == 'Q' || c == 'Y';
}

/// The first or the last day of the period `unit` names around `d`.
///
/// THE SIGN DECIDES WHICH END, and that is the one rule the platform page does not state. Its
/// worked examples pin the positive side: `<CM+30D>` from 1996-05-21 is 1996-06-30, which requires
/// CM to be 05-31 rather than 05-01, and `<CQ+1M-10D>` is 1996-07-20, which requires CQ to be the
/// quarter's last day. The negative side is the predecessor's, measured rather than documented:
/// `Date2DMY('<-CM>')` is 1 in an ERMKPIWebService test, so `-C...` is the FIRST day. Recorded on
/// board:0022 as a measured hint rather than a citation.
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

/// AL `<nM>` KEEPS THE DAY AND CLAMPS IT to the target month's length: 31 December plus one month
/// is 31 January, and 31 January plus one month is 28 or 29 February. Adding a fixed number of days
/// would give neither.
Date AddMonths(const Date &d, int months) {
  const int total = ((d.Year() * kMonthsPerYear) + d.Month() - 1) + months;
  const int year = total / kMonthsPerYear;
  const int month = (total % kMonthsPerYear) + 1;
  const std::chrono::year_month_day_last last{std::chrono::year{year} /
                                              std::chrono::month{static_cast<unsigned>(month)} /
                                              std::chrono::last};
  const auto length = static_cast<int>(static_cast<unsigned>(last.day()));
  return Date::FromYmd(year,
                       static_cast<unsigned>(month),
                       static_cast<unsigned>(d.Day() < length ? d.Day() : length));
}

/// The next or previous occurrence of an ISO weekday, EXCLUSIVE of the reference date.
///
/// `system-calcdate-string-date-method.md` gives `<-WD2>` from Tuesday 1996-05-21 as 1996-05-14 --
/// the Tuesday a week earlier, not the day itself.
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

} // namespace

DateFormula DateFormula::FromText(std::string_view text) {
  DateFormula formula;
  if (!text.empty() && text.front() == '<' && text.back() == '>') {
    text.remove_prefix(1);
    text.remove_suffix(1);
  }
  formula.text_ = std::string(text);

  std::size_t at = 0;
  bool negative = false;
  while (at < text.size()) {
    const char c = Upper(text[at]);
    if (c == '+' || c == '-') {
      negative = c == '-';
      ++at;
      continue;
    }
    if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      ++at;
      continue;
    }
    // `C<unit>` -- one end of the current period.
    if (c == 'C' && at + 1 < text.size() && IsUnit(Upper(text[at + 1]))) {
      formula.terms_.push_back(Term{
          .kind = Kind::Period, .unit = Upper(text[at + 1]), .count = 0, .negative = negative});
      at += 2;
      negative = false;
      continue;
    }
    // `WD<n>` -- a weekday selector. Read BEFORE `W`, or it splits into W(eek) and D(ay).
    if (c == 'W' && at + 1 < text.size() && Upper(text[at + 1]) == 'D') {
      at += 2;
      formula.terms_.push_back(Term{
          .kind = Kind::Weekday, .unit = 'D', .count = TakeNumber(text, at), .negative = negative});
      negative = false;
      continue;
    }
    // `W<n>` -- the Monday of ISO week n. First-class grammar, and distinct from `<nW>`.
    if (c == 'W' && at + 1 < text.size() &&
        (std::isdigit(static_cast<unsigned char>(text[at + 1])) != 0)) {
      ++at;
      formula.terms_.push_back(Term{
          .kind = Kind::Week, .unit = 'W', .count = TakeNumber(text, at), .negative = negative});
      negative = false;
      continue;
    }
    // `<n><unit>` -- a quantity. The number may be absent, which means one.
    const std::size_t before = at;
    const int n = TakeNumber(text, at);
    if (at < text.size() && IsUnit(Upper(text[at]))) {
      formula.terms_.push_back(Term{.kind = Kind::Amount,
                                    .unit = Upper(text[at]),
                                    .count = at == before ? 1 : n,
                                    .negative = negative});
      ++at;
      negative = false;
      continue;
    }
    ++at;
  }
  return formula;
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
        const std::chrono::sys_days monday{std::chrono::year{d.Year()} / std::chrono::January /
                                           std::chrono::day{4}};
        const Date january4 = Date::FromYmd(d.Year(), 1, 4);
        static_cast<void>(monday);
        const Date firstMonday =
            Date::FromDaysSinceFirst(january4.DaysSinceFirst() - (january4.DayOfWeek() - 1));
        d = Date::FromDaysSinceFirst(firstMonday.DaysSinceFirst() +
                                     ((term.count - 1) * kDaysPerWeek));
        break;
      }
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

} // namespace agiru
