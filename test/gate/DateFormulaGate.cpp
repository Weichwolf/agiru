#include "Check.h"
#include "type/Date.h"
#include "type/DateFormula.h"

#include <string>

using agiru::Date;
using agiru::DateFormula;

namespace {

/// The reference date every worked example in `system-calcdate-string-date-method.md` starts from:
/// Tuesday, 21 May 1996.
constexpr int kYear = 1996;
constexpr unsigned kMay = 5;
constexpr unsigned kTwentyFirst = 21;

Date Base() {
  return Date::FromYmd(kYear, kMay, kTwentyFirst);
}

std::string Reached(const char *formula) {
  return DateFormula::FromText(formula).CalcDate(Base()).ToInvariantString();
}

/// THE PLATFORM'S OWN THREE EXAMPLES, with the results it prints. They are what pin CM and CQ to
/// the LAST day of their period rather than the first, and a signed WD to the occurrence off the
/// reference date rather than on it.
void TheDocumentedExamplesReachTheDocumentedDates() {
  CHECK_TRUE("21 May 1996 is a Tuesday", Base().DayOfWeek() == 2);
  CHECK_TEXT("<CQ+1M-10D> is 20 July 1996", Reached("<CQ+1M-10D>"), "1996-07-20");
  CHECK_TEXT("<CM+30D> is 30 June 1996", Reached("<CM+30D>"), "1996-06-30");
  CHECK_TEXT("<-WD2> is the PREVIOUS Tuesday, not this one", Reached("<-WD2>"), "1996-05-14");
}

/// THE NEGATIVE CONTROL for the two above: if CM were the FIRST of the month, <CM+30D> would be
/// 31 May rather than 30 June, and <CQ+1M-10D> would be 21 April rather than 20 July. Both wrong
/// answers are plausible dates, which is why the examples matter.
void TheOtherEndOfThePeriodGivesADifferentDate() {
  CHECK_TRUE("the last day of the month is not the first", Reached("<CM>") != Reached("<-CM>"));
  CHECK_TEXT("and the plain form is the LAST", Reached("<CM>"), "1996-05-31");
  CHECK_TEXT("while the negative one is the first", Reached("<-CM>"), "1996-05-01");
  CHECK_TEXT("a quarter behaves the same way", Reached("<CQ>"), "1996-06-30");
  CHECK_TEXT("and a year", Reached("<CY>"), "1996-12-31");
  CHECK_TEXT(
      "a week ends on Sunday, because BC weeks begin on Monday", Reached("<CW>"), "1996-05-26");
  CHECK_TEXT("and begins on Monday", Reached("<-CW>"), "1996-05-20");
}

/// `<nM>` KEEPS THE DAY AND CLAMPS IT to the target month's length. Adding a fixed 30 or 31 days
/// gives neither answer.
void MonthsKeepTheDayAndClampIt() {
  const Date december = Date::FromYmd(2025, 12, 31);
  CHECK_TEXT("31 December plus one month is 31 January",
             DateFormula::FromText("<1M>").CalcDate(december).ToInvariantString(),
             "2026-01-31");
  const Date january = Date::FromYmd(2026, 1, 31);
  CHECK_TEXT("31 January plus one month clamps to the end of February",
             DateFormula::FromText("<1M>").CalcDate(january).ToInvariantString(),
             "2026-02-28");
  const Date leapDay = Date::FromYmd(2024, 2, 29);
  CHECK_TEXT("and a leap day plus one year clamps too",
             DateFormula::FromText("<1Y>").CalcDate(leapDay).ToInvariantString(),
             "2025-02-28");
  CHECK_TEXT("a quarter is three months", Reached("<1Q>"), "1996-08-21");
}

void PlainQuantitiesMoveByTheirUnit() {
  CHECK_TEXT("30 days", Reached("<30D>"), "1996-06-20");
  CHECK_TEXT("two weeks", Reached("<2W>"), "1996-06-04");
  CHECK_TEXT("backwards too", Reached("<-30D>"), "1996-04-21");
  CHECK_TEXT("a unit with no number means one", Reached("<D>"), "1996-05-22");
}

/// `<Wn>` IS THE MONDAY OF ISO WEEK n, and it is neither `<nW>` nor `<WDn>`. Reading `W1` as a week
/// quantity with the digit dropped left the date unmoved -- silently, which is how the predecessor
/// records losing it.
void TheWeekSelectorIsNotAWeekQuantity() {
  const Date newYear = Date::FromYmd(kYear, 1, 1);
  CHECK_TEXT("<W1> is the Monday of ISO week 1",
             DateFormula::FromText("<W1>").CalcDate(newYear).ToInvariantString(),
             "1996-01-01");
  CHECK_TEXT("<W10> is nine weeks later",
             DateFormula::FromText("<W10>").CalcDate(newYear).ToInvariantString(),
             "1996-03-04");
  // THE NEGATIVE CONTROL: the same text without the selector reading is a QUANTITY and moves the
  // base date instead, which is a different answer.
  CHECK_TRUE("<W1> and <1W> are not the same formula",
             DateFormula::FromText("<W1>").CalcDate(newYear) !=
                 DateFormula::FromText("<1W>").CalcDate(newYear));
}

void AnUnsetFormulaMovesNothingAndSaysSo() {
  const DateFormula empty;
  CHECK_TRUE("an unset formula is empty", empty.IsEmpty());
  CHECK_TRUE("and leaves the date where it was", empty.CalcDate(Base()) == Base());
  CHECK_TRUE("an undefined date stays undefined",
             DateFormula::FromText("<1M>").CalcDate(Date{}).IsUndefined());
  CHECK_TEXT("the text comes back without its brackets, as Format gives it",
             DateFormula::FromText("<CM+10D>").ToText(),
             "CM+10D");
}

} // namespace

int main() {
  return gate::Run("DateFormula", [] {
    TheDocumentedExamplesReachTheDocumentedDates();
    TheOtherEndOfThePeriodGivesADifferentDate();
    MonthsKeepTheDayAndClampIt();
    PlainQuantitiesMoveByTheirUnit();
    TheWeekSelectorIsNotAWeekQuantity();
    AnUnsetFormulaMovesNothingAndSaysSo();
  });
}
