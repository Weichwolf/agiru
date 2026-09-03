#include "meta/EnumDef.h"
#include "type/Enum.h"

#include "Check.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace {

/// THE PLATFORM'S OWN EXAMPLE, from `enum-frominteger-method.md`:
///     enum 50130 YesNo { value(0; Yes) { } value(10; No) { } }
///     Answer := Enum::YesNo.FromInteger(10); // Ordinal value for 'No'
/// It is the whole difference between Enum and Option in four lines: the ordinal of `No` is 10 and
/// its position is 1, and the documented way to reach it uses the 10.
enum class YesNo : std::int32_t {
  Yes = 0,
  No = 10,
};

/// A sparse enum that does not start at zero either -- `Sales Order Print Option` in the BaseApp
/// declares 1, 2, 3, 4 and no 0 at all.
enum class SalesOrderPrintOption : std::int32_t {
  PreAssignedNo = 1,
  No = 2,
  Both = 3,
  Neither = 4,
};

} // namespace

template <> struct agiru::EnumTraits<YesNo> {
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      agiru::EnumValueDef{.ordinal = 0, .name = "Yes", .caption = "Yes"},
      agiru::EnumValueDef{.ordinal = 10, .name = "No", .caption = "No"},
  }};
};

template <> struct agiru::EnumTraits<SalesOrderPrintOption> {
  static constexpr std::array<agiru::EnumValueDef, 4> kValues{{
      agiru::EnumValueDef{.ordinal = 1, .name = "Pre-Assigned No.", .caption = "Pre-Assigned No."},
      agiru::EnumValueDef{.ordinal = 2, .name = "No.", .caption = "No."},
      agiru::EnumValueDef{.ordinal = 3, .name = "Both", .caption = "Both"},
      agiru::EnumValueDef{.ordinal = 4, .name = "Neither", .caption = "Neither"},
  }};
};

namespace {

using Answer = agiru::Enum<YesNo>;
using PrintOption = agiru::Enum<SalesOrderPrintOption>;

static_assert(!agiru::ValuesAreDense(agiru::EnumTraits<YesNo>::kValues),
              "the case is worthless if its own example is dense");
static_assert(Answer{YesNo::No}.AsInteger() == 10);
static_assert(Answer::FromInteger(10).Name() == "No");
static_assert(Answer::FromInteger(1).Name().empty());

void TheOrdinalIsDeclaredAndNotCounted() {
  CHECK_TRUE("the second value's ordinal is 10, not 1", Answer{YesNo::No}.AsInteger() == 10);
  CHECK_TEXT("and looking up 10 finds it", std::string(Answer::FromInteger(10).Name()), "No");
  CHECK_TRUE("while 1 -- its POSITION -- names nothing",
             !Answer::FromInteger(1).IsDeclared() && Answer::FromInteger(1).Name().empty());
}

void FromIntegerHoldsWhatItIsGiven() {
  // The documented example assigns an ordinal and compares against a member afterwards.
  const Answer answer = Answer::FromInteger(10);
  CHECK_TRUE("FromInteger(10) equals No", answer == YesNo::No);
  CHECK_TRUE("and is a declared value", answer.IsDeclared());
}

void AnEnumThatDeclaresNoZeroStillDefaultsToZero() {
  // Nothing in the platform documentation gives an enum field another default, and an integer
  // column holds 0 for one that was never set. So the default is 0 and IsDeclared() is what says
  // that 0 is not one of this enumeration's values.
  const PrintOption unset;
  CHECK_TRUE("the default ordinal is 0", unset.AsInteger() == 0);
  CHECK_TRUE("which this enumeration does not declare", !unset.IsDeclared());
  CHECK_TRUE("so it renders as nothing rather than as the first value", unset.Name().empty());
}

void TheNameKeepsTheAlSpelling() {
  CHECK_TEXT("a value that is no identifier keeps its AL name",
             std::string(PrintOption{SalesOrderPrintOption::PreAssignedNo}.Name()),
             "Pre-Assigned No.");
}

void LookupFindsEveryDeclaredOrdinal() {
  // The dense fast path in ValueOf() must not answer differently from the search behind it. Every
  // declared ordinal, and one on each side of the range.
  for (const agiru::EnumValueDef &value : agiru::EnumTraits<SalesOrderPrintOption>::kValues) {
    CHECK_TEXT("every declared ordinal resolves to its own name",
               std::string(PrintOption::FromInteger(value.ordinal).Name()),
               std::string(value.name));
  }
  CHECK_TRUE("an ordinal below the range is undeclared", !PrintOption::FromInteger(0).IsDeclared());
  CHECK_TRUE("and one above it too", !PrintOption::FromInteger(5).IsDeclared());
  CHECK_TRUE("a negative ordinal does not reach past the table",
             !PrintOption::FromInteger(-1).IsDeclared());
}

void EnumsOrderByOrdinal() {
  CHECK_TRUE("ordering follows the ordinal", Answer{YesNo::Yes} < Answer{YesNo::No});
  CHECK_TRUE("equality follows the ordinal", Answer::FromInteger(10) == Answer{YesNo::No});
}

} // namespace

int main() {
  return gate::Run("Enum", [] {
    TheOrdinalIsDeclaredAndNotCounted();
    FromIntegerHoldsWhatItIsGiven();
    AnEnumThatDeclaresNoZeroStillDefaultsToZero();
    TheNameKeepsTheAlSpelling();
    LookupFindsEveryDeclaredOrdinal();
    EnumsOrderByOrdinal();
  });
}
