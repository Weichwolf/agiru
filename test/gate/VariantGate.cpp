#include "Check.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Duration.h"
#include "type/Integer.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <cstdint>
#include <string>

using agiru::Date;
using agiru::Error;
using agiru::Variant;

namespace {

/// A VARIANT ANSWERS WHAT IT HOLDS AND NEVER CONVERTS. The page gives sixty `IsX()` predicates and
/// no conversions, because a Variant is how AL passes a value whose type the callee decides on.
void ItAnswersWhatItHolds() {
  constexpr agiru::Integer kNumber = 42;
  const Variant number{kNumber};
  CHECK_TRUE("an integer says it is an integer", number.IsInteger());
  CHECK_TRUE("and says it is nothing else", !number.IsDate() && !number.IsText());
  CHECK_TRUE("and hands the value back", number.Get<agiru::Integer>() == kNumber);

  const Variant when{Date::FromYmd(2026, 3, 12)};
  CHECK_TRUE("a date says it is a date", when.IsDate());
  CHECK_TRUE("and not an integer", !when.IsInteger());
  CHECK_TEXT("and hands it back whole", when.Get<Date>().ToInvariantString(), "2026-03-12");
}

/// THE NEGATIVE CONTROL, AND IT IS THE WHOLE POINT. A `Get<Date>()` that read an integer as a day
/// number would turn a type error into a plausible wrong date, silently.
void AskingForTheWrongTypeRefuses() {
  constexpr agiru::Integer kDayNumber = 20260312;
  const Variant number{kDayNumber};
  std::string said;
  try {
    (void)number.Get<Date>();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("asking a number for a date refuses", !said.empty());
  CHECK_TRUE("rather than reading the number as one",
             said.find("does not hold") != std::string::npos);
}

void AnEmptyVariantHoldsNothingAndSaysSo() {
  const Variant empty;
  CHECK_TRUE("it is empty", empty.IsEmpty());
  CHECK_TRUE("and holds none of the types", !empty.IsInteger() && !empty.IsText());
  // THE NEGATIVE CONTROL: one that was given a value is not empty, including a value that is zero.
  CHECK_TRUE("a variant holding zero is NOT empty", !Variant{agiru::Integer{0}}.IsEmpty());
  CHECK_TRUE("and neither is one holding the empty string", !Variant{std::string{}}.IsEmpty());
}

/// AL's Code IS a Text with a normalisation rule, and a Variant carries the VALUE rather than the
/// rule -- so both answer the same alternative, which is what BaseApp code assumes when it puts a
/// Code in and asks IsText.
void ACodeAndATextAreOneAlternative() {
  const Variant text{std::string{"ACME"}};
  CHECK_TRUE("it is a text", text.IsText());
  CHECK_TRUE("and it is a code", text.IsCode());
}

void TwoVariantsCompareByTypeAndValue() {
  CHECK_TRUE("the same value of the same type is equal",
             Variant{agiru::Integer{1}} == Variant{agiru::Integer{1}});
  CHECK_TRUE("a different value is not",
             !(Variant{agiru::Integer{1}} == Variant{agiru::Integer{2}}));
  CHECK_TRUE("and neither is the same number as a different type",
             !(Variant{agiru::Integer{1}} == Variant{agiru::BigInteger{1}}));
}

/// A DURATION AND A BIGINTEGER ARE TWO TYPES, AND A VARIANT MUST TELL THEM APART. They are both
/// 64-bit integers and were one C++ type until this asked for the difference:
/// `variant-data-type.md` lists `IsDuration()` and `IsBigInteger()` as separate questions, and a
/// FieldRef cannot render a field without knowing which of the two it is.
void ADurationIsNotABigInteger() {
  constexpr std::int64_t kMilliseconds = 90000;
  const Variant span{agiru::Duration{kMilliseconds}};
  const Variant count{agiru::BigInteger{kMilliseconds}};

  CHECK_TRUE("a duration says it is a duration", span.IsDuration());
  CHECK_TRUE("and NOT a big integer", !span.IsBigInteger());
  CHECK_TRUE("a big integer says it is one", count.IsBigInteger());
  CHECK_TRUE("and NOT a duration", !count.IsDuration());

  // THE NEGATIVE CONTROL, and it is the whole reason the alias had to go: the same 64 bits.
  CHECK_TRUE("they carry the same number",
             span.Get<agiru::Duration>().Milliseconds() == count.Get<agiru::BigInteger>());
  CHECK_TRUE("and are still not equal, because the type is part of the value", !(span == count));
}

/// The algebra `duration-data-type.md` states outright.
void TheDurationAlgebraIsTheDocumentedOne() {
  const agiru::DateTime start =
      agiru::DateTime::Create(Date::FromYmd(2009, 1, 1), agiru::Time::FromHms(8, 0, 0));
  const agiru::DateTime end =
      agiru::DateTime::Create(Date::FromYmd(2009, 1, 1), agiru::Time::FromHms(9, 30, 1));

  const agiru::Duration between = end - start;
  constexpr std::int64_t kHourAndAHalfAndOne = ((std::int64_t{90} * 60) + 1) * 1000;
  CHECK_TRUE("DateTime - DateTime is a Duration of milliseconds",
             between.Milliseconds() == kHourAndAHalfAndOne);
  CHECK_TRUE("DateTime + Duration lands back on the later instant", start + between == end);
  CHECK_TRUE("DateTime - Duration lands back on the earlier one", end - between == start);
  CHECK_TRUE("a duration the other way round is negative", (start - end).Milliseconds() < 0);
  CHECK_TRUE("and adding it moves backwards", end + (start - end) == start);
}

} // namespace

int main() {
  return gate::Run("Variant", [] {
    ItAnswersWhatItHolds();
    AskingForTheWrongTypeRefuses();
    AnEmptyVariantHoldsNothingAndSaysSo();
    ACodeAndATextAreOneAlternative();
    TwoVariantsCompareByTypeAndValue();
    ADurationIsNotABigInteger();
    TheDurationAlgebraIsTheDocumentedOne();
  });
}
