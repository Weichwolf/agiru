#include "Check.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Time.h"

using agiru::Date;
using agiru::DateTime;
using agiru::Time;

namespace {

// `time-data-type.md` writes this very time as `115934.444T` in its own example.
constexpr int kHour = 11;
constexpr int kMinute = 59;
constexpr int kSecond = 34;
constexpr int kFraction = 444;
constexpr Time kNoon{Time::FromHms(kHour, kMinute, kSecond, kFraction)};

static_assert(kNoon.Hour() == kHour && kNoon.Minute() == kMinute && kNoon.Second() == kSecond &&
              kNoon.Millisecond() == kFraction);
static_assert(Time{}.IsUndefined(), "0T");
static_assert(Time{} < kNoon);

void ATimeIsMillisecondsSinceMidnight() {
  // The page's own CompareTime example subtracts two times and compares the result against
  // 86 399 999, so the difference is in MILLISECONDS and a day is 86 400 000 of them.
  CHECK_TRUE("the largest AL time is one millisecond short of a day",
             Time::FromHms(23, 59, 59, 999).AsMilliseconds() == Time::kMillisecondsPerDay - 1);
  CHECK_TRUE("and the difference across the whole day is 86 399 999",
             Time::FromHms(23, 59, 59, 999).AsMilliseconds() - Time{}.AsMilliseconds() ==
                 Time::kMillisecondsPerDay - 1);
  CHECK_TEXT("a time renders invariantly", kNoon.ToInvariantString(), "11:59:34.444");
}

void MidnightAndTheUndefinedTimeAreTheSameValue() {
  // AL does not separate them: `0T` is the blank time and `000000T` is the same literal. A type
  // that separated them would be inventing a difference no AL program can see.
  CHECK_TRUE("midnight is the undefined time", Time::FromHms(0, 0, 0, 0).IsUndefined());
  CHECK_TRUE("and they are equal", Time::FromHms(0, 0, 0, 0) == Time{});
}

void AnImpossibleTimeIsTheUndefinedOne() {
  CHECK_TRUE("24 o'clock is not a time", Time::FromHms(24, 0, 0).IsUndefined());
  CHECK_TRUE("nor 60 minutes", Time::FromHms(1, 60, 0).IsUndefined());
  CHECK_TRUE("nor 1000 milliseconds", Time::FromHms(1, 0, 0, 1000).IsUndefined());
  CHECK_TRUE("nor a count outside the day",
             Time::FromMilliseconds(Time::kMillisecondsPerDay).IsUndefined());
}

void ADateTimeIsBuiltFromADateAndATime() {
  // `datetime-data-type.md`: "The only constant available when you use the DateTime data type is
  // the undefined DateTime, 0DT. To assign a constant value ... you must use the CreateDateTime
  // method."
  const Date day = Date::FromYmd(2026, 3, 12);
  const DateTime instant = DateTime::Create(day, kNoon);
  CHECK_TRUE("the date part comes back", instant.Date() == day);
  CHECK_TRUE("and so does the time part", instant.Time() == kNoon);
  CHECK_TEXT("and it renders as UTC in the XML format",
             instant.ToInvariantString(),
             "2026-03-12T11:59:34.444Z");
}

void TheUndefinedDateTimeIsTheEarliestInstant() {
  // date-data-type.md: the undefined value "is represented by the earliest valid date in SQL
  // Server ... 01-01-1753 00:00:00:000", so that instant is not separately representable and the
  // sentinel is not an invention of this type.
  CHECK_TRUE("a default DateTime is undefined", DateTime{}.IsUndefined());
  CHECK_TRUE("building one on the earliest instant lands on it too",
             DateTime::Create(Date::FromYmd(Date::kFirstYear, 1, 1), Time{}).IsUndefined());
  CHECK_TRUE("an undefined date gives an undefined DateTime",
             DateTime::Create(Date{}, kNoon).IsUndefined());
  CHECK_TRUE("whose date part is undefined in turn", DateTime{}.Date().IsUndefined());
  CHECK_TRUE("and which renders as nothing", DateTime{}.ToInvariantString().empty());
}

void ADateTimeHasNoClosingDate() {
  // datetime-data-type.md says so outright: "The DateTime data type does not support closing
  // dates." So a closing date handed to CreateDateTime keeps its DAY and loses the bit.
  const Date day = Date::FromYmd(2025, 12, 31);
  const DateTime fromClosing = DateTime::Create(day.Closing(), kNoon);
  CHECK_TRUE("a closing date and its normal date give the same instant",
             fromClosing == DateTime::Create(day, kNoon));
  CHECK_TRUE("and the date that comes back is the normal one", !fromClosing.Date().IsClosing());
}

void InstantsOrderAcrossTheDayBoundary() {
  const Date first = Date::FromYmd(2026, 3, 12);
  const Date second = Date::FromYmd(2026, 3, 13);
  CHECK_TRUE("later in the same day is later",
             DateTime::Create(first, Time::FromHms(1, 0, 0)) <
                 DateTime::Create(first, Time::FromHms(2, 0, 0)));
  CHECK_TRUE("and the last millisecond of a day precedes the next day's midnight",
             DateTime::Create(first, Time::FromHms(23, 59, 59, 999)) <
                 DateTime::Create(second, Time{}));
  CHECK_TRUE("a day apart is 86 400 000 milliseconds",
             DateTime::Create(second, Time{}).AsMilliseconds() -
                     DateTime::Create(first, Time{}).AsMilliseconds() ==
                 Time::kMillisecondsPerDay);
}

} // namespace

int main() {
  return gate::Run("Clock", [] {
    ATimeIsMillisecondsSinceMidnight();
    MidnightAndTheUndefinedTimeAreTheSameValue();
    AnImpossibleTimeIsTheUndefinedOne();
    ADateTimeIsBuiltFromADateAndATime();
    TheUndefinedDateTimeIsTheEarliestInstant();
    ADateTimeHasNoClosingDate();
    InstantsOrderAcrossTheDayBoundary();
  });
}
