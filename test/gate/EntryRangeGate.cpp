#include "runtime/Error.h"
#include "runtime/Table.h"

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
  CHECK_TEXT("the bounds themselves pass", Refusal("0", "0", "100") + Refusal("100", "0", "100"), "");
  CHECK_TEXT("and so does anything between", Refusal("42.25", "0", "100"), "");
  CHECK_TEXT("a lone minimum bounds one side only", Refusal("1000000", "0", ""), "");
  CHECK_TEXT("a lone maximum the other", Refusal("-5", "", "100"), "");
  CHECK_TEXT("no declaration checks nothing", Refusal("-5", "", ""), "");
  CHECK_TEXT("an entry that is not a number is the field's parse to refuse, not this",
             Refusal("abc", "0", "100"), "");
  CHECK_TEXT("the code is the page's validation code",
             [] {
               try {
                 agiru::detail::CheckEntryRange("-1", "0", "");
               } catch (const agiru::Error &e) { return std::string(e.Code()); }
               return std::string{};
             }(),
             "TestValidation");
}

} // namespace

int main() {
  return gate::Run("EntryRange", [] { AnEntryOutsideTheDeclaredRangeIsRefusedWithThePlatformsWording(); });
}
