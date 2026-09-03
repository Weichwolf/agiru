#include "type/Date.h"

#include "Check.h"

using agiru::Date;

namespace {

// WHAT THE COMPILER CAN DECIDE IS A static_assert AND NEVER A CASE. An AL date literal is a
// compile-time constant, so everything a constant can answer is answered at compile time.
// `date-data-type.md` writes this very day as `20180325D` in its own example.
constexpr int kYear = 2018;
constexpr unsigned kMonth = 3;
constexpr unsigned kDay = 25;
constexpr Date kMarch{Date::FromYmd(kYear, kMonth, kDay)};
static_assert(kMarch.Year() == kYear && kMarch.Month() == static_cast<int>(kMonth) &&
              kMarch.Day() == static_cast<int>(kDay));
static_assert(Date{}.IsUndefined(), "a date starts undefined -- 0D");
static_assert(Date{} < kMarch, "the undefined date is considered to be before all other dates");
static_assert(kMarch < kMarch.Closing(), "a closing date follows its normal date");
static_assert(kMarch.Closing() < Date::FromYmd(kYear, kMonth, kDay + 1),
              "and precedes the next normal date");

void TheUndefinedDateIsBeforeAllOtherDates() {
  // date-data-type.md: "An undefined or blank date is specified by 0D. The undefined date is
  // considered to be before all other dates."
  const Date undefined;
  CHECK_TRUE("a default date is undefined", undefined.IsUndefined());
  CHECK_TRUE("and sorts before the earliest date AL accepts",
             undefined < Date::FromYmd(Date::kFirstYear, 1, 1));
  CHECK_TRUE("it has no year, month or day",
             undefined.Year() == 0 && undefined.Month() == 0 && undefined.Day() == 0);
  CHECK_TRUE("and renders as nothing rather than as a day", undefined.ToInvariantString().empty());
}

void AClosingDateSitsBetweenTwoNormalDates() {
  // date-data-type.md: "The closing date for a given date is defined as a period of time that
  // follows a given normal date and precedes the next normal date."
  const Date last = Date::FromYmd(2025, 12, 31);
  const Date closing = last.Closing();
  const Date next = Date::FromYmd(2026, 1, 1);

  CHECK_TRUE("the closing date follows its normal date", last < closing);
  CHECK_TRUE("and precedes the next normal date", closing < next);
  CHECK_TRUE("it says which it is", closing.IsClosing() && !last.IsClosing());
  CHECK_TRUE("it is the same day",
             closing.Year() == 2025 && closing.Month() == 12 && closing.Day() == 31);
  CHECK_TRUE("and it goes back", closing.Normal() == last);
  CHECK_TRUE("closing an already closing date changes nothing", closing.Closing() == closing);
  CHECK_TRUE("and neither the undefined one", Date{}.Closing().IsUndefined());
}

void TheXmlFormatIsTheInvariantOne() {
  // devenv-format-property.md, standard date formats: format 9 is `2021-04-05` in every region,
  // while format 0 is `05-04-21` in Europe and `04/05/21` in the US.
  CHECK_TEXT("format 9 is yyyy-mm-dd", Date::FromYmd(2021, 4, 5).ToInvariantString(), "2021-04-05");
  CHECK_TEXT(
      "with month and day padded", Date::FromYmd(2026, 1, 2).ToInvariantString(), "2026-01-02");
  // The `<Closing>` element appears in formats 0 to 7 and NOT in the XML format, so the two render
  // the same and only IsClosing() separates them.
  CHECK_TEXT("a closing date renders like its normal date",
             Date::FromYmd(2021, 4, 5).Closing().ToInvariantString(),
             "2021-04-05");
}

void TheWeekdayIsIsoAndNotChronos() {
  // system-date2dwy-method.md: "The value 1 corresponds to day of the week (1-7, Monday = 1)."
  // std::chrono numbers Sunday 0, so a pass-through would be wrong on exactly one day in seven.
  CHECK_TRUE("Monday is 1", Date::FromYmd(2026, 8, 31).DayOfWeek() == 1);
  CHECK_TRUE("Sunday is 7 and not 0", Date::FromYmd(2026, 9, 6).DayOfWeek() == 7);
  CHECK_TRUE("and the day between them counts up", Date::FromYmd(2026, 9, 2).DayOfWeek() == 3);
}

void TheWeekNumberIsIsoAndFoundThroughItsThursday() {
  // system-date2dwy-method.md states the RULE: "week 01 of a year is the week that includes the
  // first Thursday of the Gregorian year. Or in other words, the week that includes 4 January."
  //
  // ITS WORKED EXAMPLE CONTRADICTS ITS OWN RULE and the example is the part that is wrong. It says
  // 1 January 2014 lies in a week "that starts on Monday, December 29, 2013, and ends Sunday,
  // January 4, 2014" -- but 29 December 2013 was a SUNDAY and 4 January 2014 a Saturday. The week
  // is Monday 30 December 2013 to Sunday 5 January 2014, with two days in 2013 and five in 2014,
  // not three and four. The conclusion the example draws is still right, and the rule is
  // unambiguous, so the rule is what is implemented and the illustration is what is discarded.
  const Date newYear = Date::FromYmd(2014, 1, 1);
  CHECK_TRUE("1 January 2014 is in week 1", newYear.WeekNo() == 1);
  CHECK_TRUE("and its week-numbering year is 2014", newYear.WeekYear() == 2014);

  const Date monday = Date::FromYmd(2013, 12, 30);
  CHECK_TRUE("30 December 2013 is the Monday that opens that week", monday.DayOfWeek() == 1);
  CHECK_TRUE("so it is in week 1 of a year that is not its own",
             monday.WeekNo() == 1 && monday.WeekYear() == 2014 && monday.Year() == 2013);

  const Date sunday = Date::FromYmd(2013, 12, 29);
  CHECK_TRUE("while 29 December 2013 is the Sunday that CLOSES the week before",
             sunday.DayOfWeek() == 7);
  CHECK_TRUE("and belongs to week 52 of 2013", sunday.WeekNo() == 52 && sunday.WeekYear() == 2013);

  // The other end: a year that has 53 weeks. 2020 began on a Wednesday and was a leap year.
  CHECK_TRUE("31 December 2020 is in week 53", Date::FromYmd(2020, 12, 31).WeekNo() == 53);
  CHECK_TRUE("and 4 January is in week 1 of its own year, by definition",
             Date::FromYmd(2026, 1, 4).WeekNo() == 1);
}

void TheRangeIsTheOneAlDeclares() {
  // date-data-type.md: "Denotes a date ranging from January 1, 1753 to December 31, 9999."
  CHECK_TRUE("the first day is a date", !Date::FromYmd(1753, 1, 1).IsUndefined());
  CHECK_TRUE("the last day is a date", !Date::FromYmd(9999, 12, 31).IsUndefined());
  CHECK_TRUE("the day before the first is not", Date::FromYmd(1752, 12, 31).IsUndefined());
  CHECK_TRUE("nor the day after the last", Date::FromYmd(10000, 1, 1).IsUndefined());
  CHECK_TRUE("and neither is a day that does not exist", Date::FromYmd(2025, 2, 29).IsUndefined());
  CHECK_TRUE("while a leap day that does exist is one",
             !Date::FromYmd(2024, 2, 29).IsUndefined() && Date::FromYmd(2024, 2, 29).Day() == 29);
}

void EveryDayOfAYearSurvivesTheEncoding() {
  // The whole point of packing three facts into one integer is that they come back out. A year of
  // days, each with its closing twin, checked against what went in.
  constexpr unsigned kLongestMonth = 31;
  constexpr int kDaysInALeapYear = 366;
  int checked = 0;
  bool intact = true;
  Date previous;
  for (const unsigned month : {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U}) {
    for (unsigned day = 1; day <= kLongestMonth; ++day) {
      const Date normal = Date::FromYmd(2024, month, day);
      if (normal.IsUndefined()) { continue; }
      ++checked;
      const Date closing = normal.Closing();
      intact = intact && normal.Month() == static_cast<int>(month) &&
               normal.Day() == static_cast<int>(day) && !normal.IsClosing() &&
               closing.IsClosing() && closing.Normal() == normal && previous < normal &&
               normal < closing;
      previous = closing;
    }
  }
  CHECK_TRUE("a leap year has 366 days", checked == kDaysInALeapYear);
  CHECK_TRUE("and every one of them round-trips, orders and closes", intact);
}

} // namespace

int main() {
  return gate::Run("Date", [] {
    TheUndefinedDateIsBeforeAllOtherDates();
    AClosingDateSitsBetweenTwoNormalDates();
    TheXmlFormatIsTheInvariantOne();
    TheWeekdayIsIsoAndNotChronos();
    TheWeekNumberIsIsoAndFoundThroughItsThursday();
    TheRangeIsTheOneAlDeclares();
    EveryDayOfAYearSurvivesTheEncoding();
  });
}
