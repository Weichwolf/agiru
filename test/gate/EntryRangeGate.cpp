#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Decimal.h"

#include "Check.h"

#include <string>
#include <string_view>

namespace {

/// A `MinValue` / `MaxValue` IS A UI ENTRY LIMIT AND NOT A VALIDATE LIMIT
/// (`devenv-minvalue-property.md`: application code is not checked), with the platform's own
/// wording: `Document Search.AmountTolerance` declares `MinValue = 0; MaxValue = 100;` and
/// `Payment Registration UT` asserts an error on `SetValue(150)` and on `SetValue(-1)`, and none
/// on 0, 100 and anything between (openerp WI-801 shipped the same gate: GAINED 36).
std::string Refusal(std::string_view text, std::string_view low, std::string_view high) {
  try {
    agiru::detail::CheckEntryRange(text, low, high);
  } catch (const agiru::Error &e) { return e.what(); }
  return {};
}

void AnEntryOutsideTheDeclaredRangeIsRefusedWithThePlatformsWording() {
  CHECK_TEXT("below the minimum",
             Refusal("-1", "0", "100"),
             "The value must be greater than or equal to 0. Value: -1.");
  CHECK_TEXT("above the maximum",
             Refusal("150.5", "0", "100"),
             "The value must be less than or equal to 100. Value: 150.5.");
  CHECK_TEXT(
      "the bounds themselves pass", Refusal("0", "0", "100") + Refusal("100", "0", "100"), "");
  CHECK_TEXT("and so does anything between", Refusal("42.25", "0", "100"), "");
  CHECK_TEXT("a lone minimum bounds one side only", Refusal("1000000", "0", ""), "");
  CHECK_TEXT("a lone maximum the other", Refusal("-5", "", "100"), "");
  CHECK_TEXT("no declaration checks nothing", Refusal("-5", "", ""), "");
  CHECK_TEXT("an entry that is not a number is the field's parse to refuse, not this",
             Refusal("abc", "0", "100"),
             "");
  CHECK_TEXT(
      "the code is the page's validation code",
      [] {
        try {
          agiru::detail::CheckEntryRange("-1", "0", "");
        } catch (const agiru::Error &e) { return std::string(e.Code()); }
        return std::string{};
      }(),
      "TestValidation");
}

/// `DecimalPlaces` "is evaluated on text boxes and fields during validation"
/// (`devenv-decimalplaces-property.md`): what a user TYPES into a `0 : 5` field is rounded to
/// five places, the way `MinValue` and `NotBlank` are checked at the client -- and a `Validate`
/// from code is NOT rounded. `SCM Whse. UOM Rnding. UT` assigns `1 / 7` to a base unit's
/// `Qty. Rounding Precision` and validates `44 * (1 / 7)` into a `0 : 5` field, then expects
/// `Qty. per Unit of Measure mod Precision = 0`, which only an unrounded validate satisfies
/// (nine cases; the rounding-on-validate reading of board:0677 left them red).
void ATypedDecimalKeepsTheDeclaredPlaces() {
  const agiru::Decimal seventh = agiru::Decimal{1} / agiru::Decimal{7};
  CHECK_TEXT("0 : 5 keeps five",
             agiru::detail::DeclaredPlaces(seventh, "0 : 5").ToInvariantString(),
             "0.14286");
  CHECK_TEXT("2:5 keeps five",
             agiru::detail::DeclaredPlaces(seventh, "2:5").ToInvariantString(),
             "0.14286");
  CHECK_TEXT("a lone number is the maximum",
             agiru::detail::DeclaredPlaces(seventh, "2").ToInvariantString(),
             "0.14");
  CHECK_TEXT(
      ":3 keeps three", agiru::detail::DeclaredPlaces(seventh, ":3").ToInvariantString(), "0.143");
  CHECK_TEXT("2: names no maximum",
             agiru::detail::DeclaredPlaces(seventh, "2:").ToInvariantString(),
             seventh.ToInvariantString());
  CHECK_TEXT("no declaration rounds nothing",
             agiru::detail::DeclaredPlaces(seventh, "").ToInvariantString(),
             seventh.ToInvariantString());
  CHECK_TEXT("0 rounds to the integer",
             agiru::detail::DeclaredPlaces(seventh, "0:0").ToInvariantString(),
             "0");
}

} // namespace

int main() {
  return gate::Run("EntryRange", [] {
    AnEntryOutsideTheDeclaredRangeIsRefusedWithThePlatformsWording();
    ATypedDecimalKeepsTheDeclaredPlaces();
  });
}
