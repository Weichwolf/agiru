#include "Check.h"
#include "ResourceCost.h"
#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "type/Option.h"

#include <string>

using agiru::Error;
using agiru::StrSubstNo;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostCostType;
using agiru::app::tables::ResourceCostType;

namespace {

/// Runs something that must raise, and hands back the message. An empty string means it did not
/// raise -- which every case below then reports as a failure rather than passing quietly.
std::string Raised(auto &&f) {
  try {
    f();
  } catch (const Error &e) { return e.what(); }
  return {};
}

void FieldErrorFollowsTheDocumentedThreeForms() {
  // record-fielderror-joker-string-method.md, example 3: with a Text parameter the message is
  // "<FieldCaption> <Text> in <TableCaption> <primary key>." and the period is the platform's.
  ResourceCost rec;
  rec.Type = ResourceCostType::All;
  rec.Code = "R100";
  CHECK_TEXT("FieldError with a text",
             Raised([&] { rec.OnValidateCode(); }),
             "Code cannot be specified when Type is All in Resource Cost "
             "Type='All',Code='R100',Work Type Code=''.");

  // Example 1: no text, blank field -> "You must specify ...".
  ResourceCost blank;
  CHECK_TEXT("FieldError on a blank field",
             Raised([&] { blank.FieldError(ResourceCost::FieldNumber::Code); }),
             "You must specify Code in Resource Cost Type='Resource',Code='',Work Type Code=''.");

  // Example 2: no text, field has a value -> "<Caption> must not be <value> ...".
  ResourceCost valued;
  valued.Code = "R100";
  CHECK_TEXT(
      "FieldError on a field with a value",
      Raised([&] { valued.FieldError(ResourceCost::FieldNumber::Code); }),
      "Code must not be R100 in Resource Cost Type='Resource',Code='R100',Work Type Code=''.");
}

void TheTriggerStaysSilentWhenAlWouldStaySilent() {
  // if (Code <> '') and (Type = Type::All) -- neither half alone fires it.
  ResourceCost onlyCode;
  onlyCode.Code = "R100";
  CHECK_SILENT("a code without Type::All passes", Raised([&] { onlyCode.OnValidateCode(); }));

  ResourceCost onlyAll;
  onlyAll.Type = ResourceCostType::All;
  CHECK_SILENT("Type::All without a code passes", Raised([&] { onlyAll.OnValidateCode(); }));
}

void TestFieldMismatchCarriesBothValues() {
  // The trigger on field 4: if "Work Type Code" = '' then TestField("Cost Type", ::Fixed).
  ResourceCost rec;
  rec.CostType = ResourceCostCostType::PercentExtra;
  CHECK_TEXT("TestField with a value that does not match",
             Raised([&] { rec.OnValidateCostType(); }),
             "Cost Type must be equal to 'Fixed'  in Resource Cost: "
             "Type='Resource', Code='', Work Type Code=''. Current value is '% Extra'.");

  // A matching value raises nothing, and neither does a filled Work Type Code.
  ResourceCost fixedCost;
  CHECK_SILENT("a matching value passes", Raised([&] { fixedCost.OnValidateCostType(); }));

  ResourceCost withWorkType;
  withWorkType.WorkTypeCode = "WT";
  withWorkType.CostType = ResourceCostCostType::LCYExtra;
  CHECK_SILENT("a filled Work Type Code skips the test",
               Raised([&] { withWorkType.OnValidateCostType(); }));
}

void TestFieldOnABlankFieldSaysSo() {
  ResourceCost rec;
  CHECK_TEXT("TestField on a blank field",
             Raised([&] { rec.TestField(ResourceCost::FieldNumber::Code); }),
             "Code must have a value in Resource Cost: "
             "Type='Resource', Code='', Work Type Code=''. It cannot be zero or empty.");

  rec.Code = "R100";
  CHECK_SILENT("and stays silent once it has one",
               Raised([&] { rec.TestField(ResourceCost::FieldNumber::Code); }));
}

void ThePrimaryKeySeparatorsDiffferBetweenTheTwo() {
  // NOT A SLIP. FieldError joins the key with a bare comma after a space; TestField prefixes a
  // colon and joins with a comma AND a space. The first is the documentation's own examples, the
  // second comes from the predecessor where it was verified against the BC test suite. A case
  // states it so that nobody "tidies" one into the other.
  ResourceCost rec;
  const std::string fieldError = Raised([&] { rec.FieldError(ResourceCost::FieldNumber::Code); });
  const std::string testField = Raised([&] { rec.TestField(ResourceCost::FieldNumber::Code); });
  CHECK_TRUE("FieldError writes ' Type=' with no colon",
             fieldError.find("Resource Cost Type='Resource',Code=") != std::string::npos);
  CHECK_TRUE("TestField writes ': Type=' with a colon and spaced commas",
             testField.find("Resource Cost: Type='Resource', Code=") != std::string::npos);
}

void StrSubstNoReplacesWhatItIsGiven() {
  CHECK_TEXT("two placeholders",
             StrSubstNo("cannot be specified when %1 is %2", "Type", "All"),
             "cannot be specified when Type is All");
  CHECK_TEXT("hash markers count too", StrSubstNo("#1 and #2", "a", "b"), "a and b");
  CHECK_TEXT("out of order", StrSubstNo("%2 before %1", "one", "two"), "two before one");
  // AL leaves a placeholder with no argument standing, so a message missing a parameter is
  // VISIBLE rather than merely wrong.
  CHECK_TEXT(
      "a placeholder without an argument stays", StrSubstNo("%1 and %2", "only"), "only and %2");
  CHECK_TEXT(
      "a percent that is not a placeholder stays", StrSubstNo("100% sure", "x"), "100% sure");
}

void FieldCaptionIsTheAlCaption() {
  const ResourceCost named;
  CHECK_TEXT("a caption with spaces",
             std::string(named.FieldCaption(ResourceCost::FieldNumber::WorkTypeCode)),
             "Work Type Code");
}

} // namespace

int main() {
  return gate::Run("RecordError", [] {
    FieldErrorFollowsTheDocumentedThreeForms();
    TheTriggerStaysSilentWhenAlWouldStaySilent();
    TestFieldMismatchCarriesBothValues();
    TestFieldOnABlankFieldSaysSo();
    ThePrimaryKeySeparatorsDiffferBetweenTheTwo();
    StrSubstNoReplacesWhatItIsGiven();
    FieldCaptionIsTheAlCaption();
  });
}
