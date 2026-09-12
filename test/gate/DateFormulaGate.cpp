#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/Language.h"

#include "Check.h"

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
  return DateFormula::FromText(formula)->CalcDate(Base()).ToInvariantString();
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
             DateFormula::FromText("<1M>")->CalcDate(december).ToInvariantString(),
             "2026-01-31");
  const Date january = Date::FromYmd(2026, 1, 31);
  CHECK_TEXT("31 January plus one month clamps to the end of February",
             DateFormula::FromText("<1M>")->CalcDate(january).ToInvariantString(),
             "2026-02-28");
  const Date leapDay = Date::FromYmd(2024, 2, 29);
  CHECK_TEXT("and a leap day plus one year clamps too",
             DateFormula::FromText("<1Y>")->CalcDate(leapDay).ToInvariantString(),
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
             DateFormula::FromText("<W1>")->CalcDate(newYear).ToInvariantString(),
             "1996-01-01");
  CHECK_TEXT("<W10> is nine weeks later",
             DateFormula::FromText("<W10>")->CalcDate(newYear).ToInvariantString(),
             "1996-03-04");
  // THE NEGATIVE CONTROL: the same text without the selector reading is a QUANTITY and moves the
  // base date instead, which is a different answer.
  CHECK_TRUE("<W1> and <1W> are not the same formula",
             DateFormula::FromText("<W1>")->CalcDate(newYear) !=
                 DateFormula::FromText("<1W>")->CalcDate(newYear));
}

void AnUnsetFormulaMovesNothingAndSaysSo() {
  const DateFormula empty;
  CHECK_TRUE("an unset formula is empty", empty.IsEmpty());
  CHECK_TRUE("and leaves the date where it was", empty.CalcDate(Base()) == Base());
  CHECK_TRUE("an undefined date stays undefined",
             DateFormula::FromText("<1M>")->CalcDate(Date{}).IsUndefined());
  CHECK_TEXT("the text comes back without its brackets, as Format gives it",
             DateFormula::FromText("<CM+10D>")->ToText(),
             "CM+10D");
}

/// `D15` IS THE 15TH OF THE MONTH THE DATE IS IN, which `dateformula-data-type.md` lists beside
/// `30D` and `CM+10D` ("On the 15th of each month"), and a day past the month's end clamps.
void TheDaySelectorPicksADayOfTheMonth() {
  CHECK_TEXT("<D15> from 21 May is 15 May", Reached("<D15>"), "1996-05-15");
  CHECK_TEXT(
      "<D31> in February clamps",
      DateFormula::FromText("<D31>")->CalcDate(Date::FromYmd(1996, 2, 10)).ToInvariantString(),
      "1996-02-29");
  CHECK_TRUE("<D15> and <15D> are not the same formula", Reached("<D15>") != Reached("<15D>"));
}

/// THE COLUMN HOLDS BC'S PACKED FORM: the digits and signs as ASCII and each unit as one control
/// byte, `C` 1, `D` 2, `WD` 3, `W` 4, `M` 5, `Q` 6, `Y` 7. Every DateFormula column of the CRONUS
/// load is in it -- 63 columns of 42 tables carry a control byte (`agiru_seeded`, 2026-09-12) --
/// and read as text they were refused and silently empty: every payment term's due date
/// calculation, every lead and shipping time. The predecessor decoded the same bytes from the
/// same hex dump (openerp WI-1077).
void TheStoredFormIsBCsPackedOne() {
  CHECK_TEXT("10\x02 is 10D", DateFormula::FromText("10\x02")->InvariantText(), "10D");
  CHECK_TEXT("\x01\x05 is CM", DateFormula::FromText("\x01\x05")->InvariantText(), "CM");
  CHECK_TEXT("\x01\x07-1\x07+1\x02 is CY-1Y+1D",
             DateFormula::FromText("\x01\x07-1\x07+1\x02")->InvariantText(),
             "CY-1Y+1D");
  CHECK_TEXT("and a packed 10D moves ten days",
             DateFormula::FromText("10\x02")->CalcDate(Base()).ToInvariantString(),
             "1996-05-31");
  CHECK_TEXT("the formula is written back packed",
             DateFormula::FromText("<CM-1M>")->ToStorageText(),
             "\x01\x05-1\x05");
  CHECK_TEXT("a weekday is one byte and its number",
             DateFormula::FromText("<-WD2>")->ToStorageText(),
             "-\x03"
             "2");
  CHECK_TEXT("an empty formula is an empty column", DateFormula{}.ToStorageText(), "");
  CHECK_TRUE("and the packed form reads back as the same formula",
             *DateFormula::FromText(DateFormula::FromText("<CQ+1M-10D>")->ToStorageText()) ==
                 *DateFormula::FromText("<CQ+1M-10D>"));
}

/// A FORMULA SHOWS AND READS IN THE SESSION'S LANGUAGE (`dateformula-data-type.md`: stored
/// language-independently, "converted to a valid date conversion string for the currently selected
/// language" when shown). German is anchored by ERM General Journal UT, which reads a yearly
/// Recurring Frequency back as `1J` under `GlobalLanguage(1031)`; French and Spanish by the
/// platform page's own `1W+1D` -- `1S+1J` and `1S+1D`.
void AFormulaShowsAndReadsInTheSessionsLanguage() {
  constexpr agiru::Integer kGerman = 1031;
  constexpr agiru::Integer kDanish = 1030;
  constexpr agiru::Integer kFrench = 1036;
  CHECK_TRUE("a thread formats in en-US until a session says otherwise",
             agiru::Language::Current() == agiru::Language::kEnglishUnitedStates);
  CHECK_TEXT("under English 1Y is 1Y", DateFormula::FromText("<1Y>")->ToText(), "1Y");
  CHECK_TRUE("and 1J is not a formula", !DateFormula::FromText("1J").has_value());
  agiru::Language::MakeCurrent(kGerman);
  CHECK_TEXT(
      "under German the same formula shows 1J", DateFormula::FromText("<1Y>")->ToText(), "1J");
  CHECK_TEXT(
      "and the invariant text is still 1Y", DateFormula::FromText("<1Y>")->InvariantText(), "1Y");
  CHECK_TEXT("CM+10D shows LM+10T", DateFormula::FromText("<CM+10D>")->ToText(), "LM+10T");
  CHECK_TEXT("and a weekday is WT", DateFormula::FromText("<-WD2>")->ToText(), "-WT2");
  CHECK_TEXT("a German 1J reads as a year", DateFormula::FromText("1J")->InvariantText(), "1Y");
  CHECK_TEXT("LM+10T reads as CM+10D", DateFormula::FromText("lm+10t")->InvariantText(), "CM+10D");
  CHECK_TEXT("the invariant letters still read, because none of them clashes",
             DateFormula::FromText("1Y")->InvariantText(),
             "1Y");
  CHECK_TRUE("but inside the brackets only the invariant letters are a formula",
             !DateFormula::FromText("<1J>").has_value());
  agiru::Language::MakeCurrent(kFrench);
  CHECK_TEXT("under French 1W+1D is 1S+1J, as the platform page says",
             DateFormula::FromText("<1W+1D>")->ToText(),
             "1S+1J");
  agiru::Language::MakeCurrent(kDanish);
  CHECK_TEXT("under Danish a year is Å", DateFormula::FromText("<1Y>")->ToText(), "1Å");
  CHECK_TEXT("and a lowercase å reads as one", DateFormula::FromText("1å")->InvariantText(), "1Y");
  agiru::Language::MakeCurrent(agiru::Language::kEnglishUnitedStates);
  CHECK_TEXT(
      "and back under English it is 1Y again", DateFormula::FromText("<1Y>")->ToText(), "1Y");
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
    TheDaySelectorPicksADayOfTheMonth();
    TheStoredFormIsBCsPackedOne();
    AFormulaShowsAndReadsInTheSessionsLanguage();
  });
}
