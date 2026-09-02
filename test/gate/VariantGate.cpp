#include "Check.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Date.h"
#include "type/Integer.h"
#include "type/Variant.h"

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

} // namespace

int main() {
  return gate::Run("Variant", [] {
    ItAnswersWhatItHolds();
    AskingForTheWrongTypeRefuses();
    AnEmptyVariantHoldsNothingAndSaysSo();
    ACodeAndATextAreOneAlternative();
    TwoVariantsCompareByTypeAndValue();
  });
}
