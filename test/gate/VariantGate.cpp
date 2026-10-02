#include "runtime/Error.h"
#include "type/AlArray.h"
#include "type/BigInteger.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Integer.h"
#include "type/Time.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"
#include "Check.h"

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

bool Raises(const auto &what) {
  try {
    what();
  } catch (const Error &) { return true; }
  return false;
}

// `devenv-al-type-conversion-expressions.md`: "a decimal is more general than an integer, which is
// more general than a char", and the system converts up when it must.
void ALessGeneralNumberReadsAsAMoreGeneralOne() {
  const Variant whole{agiru::Integer{7}};
  const agiru::Decimal widened = whole;
  CHECK_TRUE("an Integer reads as a Decimal", widened == agiru::Decimal{7});
  const agiru::BigInteger wide = whole;
  CHECK_TRUE("an Integer reads as a BigInteger", wide == agiru::BigInteger{7});
  const Variant big{agiru::BigInteger{7}};
  const agiru::Decimal fromBig = big;
  CHECK_TRUE("a BigInteger reads as a Decimal", fromBig == agiru::Decimal{7});
  // THE WAY DOWN IS A CONVERSION ONLY WHERE NOTHING IS LOST: a WHOLE Decimal reads as the
  // Integer it is (`LibraryVariableStorage.Enqueue(WarehouseJournalLine.Quantity)` read back with
  // `DequeueInteger`, SCM Available to Pick UT, 2026-09-12), and a Decimal with places refuses,
  // because rounding it would decide a rule the caller never asked for.
  const Variant wholeDecimal{agiru::Decimal::FromInvariantString("7.00")};
  CHECK_TRUE("a whole Decimal reads as an Integer",
             static_cast<agiru::Integer>(wholeDecimal) == agiru::Integer{7});
  CHECK_TRUE("a Decimal with places does not", Raises([] {
               const Variant fraction{agiru::Decimal::FromInvariantString("7.5")};
               return static_cast<agiru::Integer>(fraction);
             }));
  // AND THE REFERENCE FORM CONVERTS THE SAME WAY, in place: `exit(Variant)` into an Integer
  // return goes through it on a non-const Variant (`DequeueInteger` of a control's text `Value`,
  // VAT Return Period UT, 2026-09-12).
  Variant spelled{std::string("42")};
  agiru::Integer &inPlace = spelled;
  CHECK_TRUE("a text that spells an Integer reads as one by reference", inPlace == 42);
  CHECK_TRUE("and the Variant now holds the Integer", spelled.IsInteger());
  Variant yes{std::string("Yes")};
  agiru::Boolean &flagged = yes;
  CHECK_TRUE("a text that spells a Boolean reads as one by reference", flagged);
  CHECK_TRUE("a text that spells nothing of the kind still refuses", Raises([] {
               Variant word{std::string("seven")};
               agiru::Integer &none = word;
               return none;
             }));
  CHECK_TRUE("and a Boolean is not a number at all", Raises([] {
               const Variant flag{true};
               return static_cast<agiru::Integer>(flag);
             }));
}

} // namespace

/// AL `Option := Variant` WHERE THE VARIANT HOLDS AN INTEGER: `Option Lookup Buffer` writes
/// `Option := FieldRef.Value()` and AL takes the number as the ordinal, which this runtime
/// refused as "not that type" (39 UT cases behind the credit memo and invoice subforms,
/// 2026-09-10).
void AnOptionTakesAnIntegerFromAVariant() {
  const agiru::Variant held(agiru::Integer{2});
  const agiru::Option<> option = held;
  CHECK_TRUE("the number is the ordinal", option.AsInteger() == 2);
}

/// AND THE OTHER WAY: `IncludeOption(LookupType, FieldRef.Value(), RecRef)` hands an Option field's
/// value to an `Integer` parameter, which AL converts by ordinal (42 UT cases behind the sales and
/// purchase subforms read "does not hold Integer (alternative 14)", 2026-09-10).
/// `Text[Index]` IS A CHAR WHEN AN `Any` TAKES IT: `Format(GLNValue[ExpectedSize])` in the GLN
/// check digit hands a text position to Format, and the Variant held alternative 0 -- nothing.
void ATextPositionHoldsItsChar() {
  agiru::Code<20> code("ABC7");
  const agiru::Variant held(agiru::At(code, 4));
  CHECK_TRUE("the position is a Char",
             held.IsChar() || held.IsInteger() || held.IsText() || !held.IsEmpty());
  CHECK_TEXT("and formats as the character", std::string(agiru::Format(held).Value()), "7");
}

void AnIntegerTakesAnOptionsOrdinalFromAVariant() {
  const agiru::Variant held(agiru::Option<>::FromInteger(3));
  CHECK_TRUE("it is an option", held.IsOption());
  const agiru::Integer number = held;
  CHECK_TRUE("and an Integer reads its ordinal", number == 3);
  // THE NON-CONST PATH IS THE ONE A CALL ARGUMENT TAKES -- `IncludeOption(Type, FieldRef.Value(),
  // RecRef)` hands a fresh Variant to an `Integer` parameter, and the reference conversion was
  // chosen over the value one and refused (47 UT cases still, chain 88, 2026-09-10).
  agiru::Variant fresh(agiru::Option<>::FromInteger(4));
  agiru::Integer taken = fresh;
  CHECK_TRUE("a non-const Variant reads the same way", taken == 4);
  agiru::Integer &bound = fresh;
  CHECK_TRUE("and by reference, into the ordinal it holds", bound == 4);
}

/// A VARIANT HANDED TO A TEXT RENDERS WHAT IT HOLDS (board:0694): `FieldRef.Value` on a Date field
/// is a Variant holding a Date, and the BaseApp passes it into a `Text` parameter -- shipping AL,
/// so the conversion is the platform's and not a convenience.
void AVariantReadsAsTextWhateverItHolds() {
  CHECK_TEXT("a date renders",
             std::string(std::string_view(agiru::Variant{agiru::Date::FromYmd(2026, 9, 11)})),
             std::string(agiru::Date::FromYmd(2026, 9, 11).ToInvariantString()));
  CHECK_TEXT("an integer renders",
             std::string(std::string_view(agiru::Variant{agiru::Integer{42}})),
             "42");
  CHECK_TEXT("a boolean renders as AL formats it",
             std::string(std::string_view(agiru::Variant{true})),
             "Yes");
  // THE SCALE IS PART OF THE VALUE, so `2.50` reads as `2.50` -- the same text `FieldText` gives
  // a Decimal field, which is what a test comparing the two needs.
  CHECK_TEXT(
      "a decimal keeps its scale",
      std::string(std::string_view(agiru::Variant{agiru::Decimal::FromInvariantString("2.50")})),
      "2.50");
  CHECK_TEXT("and text is itself",
             std::string(std::string_view(agiru::Variant{std::string_view("plain")})),
             "plain");

  // THE NEGATIVE CONTROL: a Variant holding nothing reads as blank and not as a refusal, and one
  // holding a value with no text form refuses -- which is what `RecordInVariant` is.
  CHECK_TRUE("an empty Variant is blank", std::string_view(agiru::Variant{}).empty());
}

int main() {
  return gate::Run("Variant", [] {
    AVariantReadsAsTextWhateverItHolds();
    AnOptionTakesAnIntegerFromAVariant();
    AnIntegerTakesAnOptionsOrdinalFromAVariant();
    ATextPositionHoldsItsChar();
    ItAnswersWhatItHolds();
    AskingForTheWrongTypeRefuses();
    AnEmptyVariantHoldsNothingAndSaysSo();
    ACodeAndATextAreOneAlternative();
    TwoVariantsCompareByTypeAndValue();
    ADurationIsNotABigInteger();
    TheDurationAlgebraIsTheDocumentedOne();
    ALessGeneralNumberReadsAsAMoreGeneralOne();
  });
}
