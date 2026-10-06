#include "meta/EnumDef.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/StringValue.h"
#include "type/Time.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace {

/// Exactly what the generator will emit for table 202 "Resource Cost", field 1:
///     OptionCaption = 'Resource,Group(Resource),All';
///     OptionMembers = Resource,"Group(Resource)",All;
/// The AL member `"Group(Resource)"` cannot be a C++ identifier, so the enumerator is renamed and
/// the name table keeps the AL spelling.
enum class ResourceCostType : std::int32_t {
  Resource = 0,
  GroupResource = 1,
  All = 2,
};

} // namespace

template <> struct agiru::OptionTraits<ResourceCostType> {
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      agiru::EnumValueDef{.ordinal = 0, .name = "Resource", .caption = "Resource"},
      agiru::EnumValueDef{.ordinal = 1, .name = "Group(Resource)", .caption = "Group(Resource)"},
      agiru::EnumValueDef{.ordinal = 2, .name = "All", .caption = "All"},
  }};
};

namespace {

using Type = agiru::Option<ResourceCostType>;

// WHAT THE COMPILER CAN DECIDE IS A static_assert AND NEVER A CASE. `Option` itself asserts that
// the members are zero-based and sequential, so instantiating the type below is already that check.
static_assert(agiru::ValuesAreDense(agiru::OptionTraits<ResourceCostType>::kValues));
static_assert(static_cast<std::int32_t>(ResourceCostType::Resource) == 0);
static_assert(Type{}.AsInteger() == 0);
static_assert(Type{ResourceCostType::All}.AsInteger() == 2);
static_assert(Type{ResourceCostType::All}.Name() == "All");

void TheTypeIsZeroBasedAndSequential() {
  // option-data-type.md: "a zero-based enumerator type ... assigned to sequential numbers,
  // starting with 0."
  CHECK_TRUE("the default is the first member", Type{}.Value() == ResourceCostType::Resource);
  CHECK_TRUE("the members are 0, 1, 2",
             Type{ResourceCostType::Resource}.AsInteger() == 0 &&
                 Type{ResourceCostType::GroupResource}.AsInteger() == 1 &&
                 Type{ResourceCostType::All}.AsInteger() == 2);
}

void ItConvertsToAnIntegerBothWays() {
  // "You can convert option data types to integers."
  CHECK_TRUE("an integer becomes an option", Type{1}.Value() == ResourceCostType::GroupResource);
  CHECK_TRUE("and back", Type{ResourceCostType::GroupResource}.AsInteger() == 1);
}

void TheNameKeepsTheAlSpelling() {
  CHECK_TEXT("a member that is no identifier keeps its AL name",
             std::string(Type{ResourceCostType::GroupResource}.Name()),
             "Group(Resource)");
  CHECK_TEXT("and its caption",
             std::string(Type{ResourceCostType::GroupResource}.Caption()),
             "Group(Resource)");
}

void AnUndeclaredOrdinalIsHeldRatherThanRefused() {
  // AL does not refuse an out-of-range ordinal on an option; an integer may be assigned to one.
  // So this type holds it and says it is not declared, rather than throwing where AL would not.
  const Type stray{7};
  CHECK_TRUE("an undeclared ordinal is kept", stray.AsInteger() == 7);
  CHECK_TRUE("and reported as undeclared", !stray.IsDeclared());
  CHECK_TRUE("with an empty name rather than a crash", stray.Name().empty());
}

void OptionsOrderByOrdinal() {
  CHECK_TRUE("ordering follows the ordinal",
             Type{ResourceCostType::Resource} < Type{ResourceCostType::All});
  CHECK_TRUE("equality follows the ordinal", Type{2} == Type{ResourceCostType::All});
}

/// EVALUATE READS A MEMBER BY NAME, BY CAPTION OR BY ORDINAL, and refuses what names none. A
/// TestPage sets a page variable of an option or enum type by rendering the value it was given
/// and evaluating the text back, so a reader that took only digits left the variable at its
/// default (Price List Line UT, 5 lookup cases, 2026-09-12).
void EvaluateReadsAMemberByNameCaptionOrOrdinal() {
  Type held;
  CHECK_TRUE("the AL name is read", static_cast<bool>(agiru::Evaluate(held, "Group(Resource)")));
  CHECK_TRUE("into its ordinal", held.AsInteger() == 1);
  CHECK_TRUE("and without regard to case", static_cast<bool>(agiru::Evaluate(held, "all")));
  CHECK_TRUE("into that member", held == ResourceCostType::All);
  CHECK_TRUE("digits are the ordinal", static_cast<bool>(agiru::Evaluate(held, "0")));
  CHECK_TRUE("and land as it", held == ResourceCostType::Resource);
  // THE NEGATIVE CONTROL: a text that names no member is refused and the value stays.
  CHECK_TRUE("a stranger is refused", !static_cast<bool>(agiru::Evaluate(held, "Elsewhere")));
  CHECK_TRUE("and the value stays where it was", held == ResourceCostType::Resource);
}

void ScalarEvaluationKeepsItsExistingReaders() {
  agiru::Boolean truth{};
  CHECK_TRUE("Boolean evaluation selects the shared spelling reader",
             agiru::Evaluate(truth, "YES") && truth);
  CHECK_TRUE("invalid Boolean input keeps its prior value",
             !agiru::Evaluate(truth, "unknown") && truth);
  agiru::Integer number{};
  CHECK_TRUE("integral evaluation reads a signed value", agiru::Evaluate(number, "-1234"));
  CHECK_TRUE("integral evaluation does not substitute a default", number == -1234);
  CHECK_TRUE("invalid integral input keeps the value",
             !agiru::Evaluate(number, "3junk") && number == -1234);
  agiru::BigInteger large{};
  CHECK_TRUE("BigInteger evaluation keeps the full signed width",
             agiru::Evaluate(large, "9223372036854775807") &&
                 large == std::numeric_limits<agiru::BigInteger>::max());
  agiru::Duration duration;
  CHECK_TRUE("Duration evaluation uses signed milliseconds",
             agiru::Evaluate(duration, "-1234") && duration.Milliseconds() == -1234);
  CHECK_TRUE("empty Duration input retains the zero-value contract",
             agiru::Evaluate(duration, "") && duration.Milliseconds() == 0);
  agiru::Option<void> raw;
  CHECK_TRUE("untyped Option evaluation reads ordinals",
             agiru::Evaluate(raw, "7") && raw.AsInteger() == 7);
  CHECK_TRUE("untyped Option refuses names rather than inventing metadata",
             !agiru::Evaluate(raw, "Resource") && raw.AsInteger() == 7);
  CHECK_TRUE("empty untyped Option input means ordinal zero",
             agiru::Evaluate(raw, "") && raw.AsInteger() == 0);
  agiru::Decimal exact;
  constexpr std::string_view digits = "0.1234567890123456789012345678";
  CHECK_TRUE("Decimal evaluation reads all 28 decimal places", agiru::Evaluate(exact, digits));
  CHECK_TEXT("Decimal evaluation is exact rather than binary floating point",
             exact.ToInvariantString(),
             digits);
  CHECK_TRUE("a refused decimal does not clear the destination",
             !agiru::Evaluate(exact, "invalid"));
  CHECK_TEXT(
      "Decimal refusal leaves the previous scale and value", exact.ToInvariantString(), digits);
  agiru::Text<0> text;
  CHECK_TRUE("Text evaluation preserves Unicode and literal operators",
             agiru::Evaluate(text, "雪🙂 |*"));
  CHECK_TEXT("Text is not interpreted as a filter", std::string_view(text), "雪🙂 |*");
  constexpr std::size_t kCodeLength = 20;
  agiru::Code<kCodeLength> code;
  CHECK_TRUE("Code evaluation retains its assignment normalization",
             agiru::Evaluate(code, " code "));
  CHECK_TEXT("Code still normalizes case and surrounding spaces", std::string_view(code), "CODE");

  struct Unreadable {
    int value = 1;
  } unsupported;

  CHECK_TRUE("unknown types keep explicit evaluation failure",
             !agiru::Evaluate(unsupported, "1") && unsupported.value == 1);
}

void FactoryAndTemporalEvaluationKeepsItsExistingReaders() {
  agiru::Guid identity;
  constexpr std::string_view guid = "{01234567-89AB-CDEF-0123-456789ABCDEF}";
  CHECK_TRUE("Guid evaluation selects the declared factory", agiru::Evaluate(identity, guid));
  CHECK_TEXT("the parsed identity is not a default or truncated value", identity.ToText(), guid);
  CHECK_TRUE("invalid Guid input returns the factory refusal", !agiru::Evaluate(identity, "bad"));
  CHECK_TEXT("factory refusal leaves the destination unchanged", identity.ToText(), guid);
  agiru::DateFormula formula;
  CHECK_TRUE("DateFormula evaluation selects its own factory", agiru::Evaluate(formula, "<1M>"));
  CHECK_TEXT("DateFormula retains its canonical typed expression", formula.ToText(), "1M");
  agiru::Date date;
  CHECK_TRUE("Date dispatch retains the existing invariant reader",
             agiru::Evaluate(date, "2028-01-25"));
  CHECK_TEXT("Date dispatch retains the full date", date.ToInvariantString(), "2028-01-25");
  agiru::Time time;
  CHECK_TRUE("Time dispatch retains milliseconds", agiru::Evaluate(time, "12:34:56.789"));
  CHECK_TEXT("Time is not evaluated as an integer", time.ToInvariantString(), "12:34:56.789");
  agiru::DateTime instant;
  CHECK_TRUE("DateTime dispatch retains its own reader",
             agiru::Evaluate(instant, "2028-01-25T12:34:56.789Z"));
  CHECK_TRUE("DateTime retains both date and time", instant == agiru::DateTime::Create(date, time));
}

} // namespace

int main() {
  return gate::Run("Option", [] {
    TheTypeIsZeroBasedAndSequential();
    ItConvertsToAnIntegerBothWays();
    TheNameKeepsTheAlSpelling();
    AnUndeclaredOrdinalIsHeldRatherThanRefused();
    OptionsOrderByOrdinal();
    EvaluateReadsAMemberByNameCaptionOrOrdinal();
    ScalarEvaluationKeepsItsExistingReaders();
    FactoryAndTemporalEvaluationKeepsItsExistingReaders();
  });
}
