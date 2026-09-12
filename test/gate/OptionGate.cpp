#include "meta/EnumDef.h"
#include "type/Option.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <array>
#include <cstdint>
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

} // namespace

int main() {
  return gate::Run("Option", [] {
    TheTypeIsZeroBasedAndSequential();
    ItConvertsToAnIntegerBothWays();
    TheNameKeepsTheAlSpelling();
    AnUndeclaredOrdinalIsHeldRatherThanRefused();
    OptionsOrderByOrdinal();
    EvaluateReadsAMemberByNameCaptionOrOrdinal();
  });
}
