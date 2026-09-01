#include "Check.h"
#include "Names.h"

using agiru::gen::EnumeratorName;
using agiru::gen::Identifier;
using agiru::gen::OptionEnumName;

namespace {

void AnAlNameBecomesAnIdentifier() {
  CHECK_TEXT("spaces close up", Identifier("Work Type Code"), "WorkTypeCode");
  CHECK_TEXT("a plain name is itself", Identifier("Code"), "Code");
  CHECK_TEXT("existing capitals are kept", Identifier("VAT Amount"), "VATAmount");
  // AL names carry punctuation that no identifier may: "G/L Account", "No.", "Gen. Jnl.-Post Line".
  CHECK_TEXT("a slash is a word boundary", Identifier("G/L Account"), "GLAccount");
  CHECK_TEXT("a full stop is a word boundary", Identifier("No."), "No");
  CHECK_TEXT("dots and hyphens together", Identifier("Gen. Jnl.-Post Line"), "GenJnlPostLine");
  // An identifier may not start with a digit.
  CHECK_TEXT("a leading digit is escaped", Identifier("3 Way Match"), "_3WayMatch");
}

void AnOptionMemberBecomesAnEnumerator() {
  // The two members of table 202 that no identifier can spell.
  CHECK_TEXT("parentheses close up", EnumeratorName("Group(Resource)"), "GroupResource");
  // A per cent sign is a WORD, not a boundary: '% Extra' and 'Extra' would otherwise collide.
  CHECK_TEXT("a per cent sign becomes a word", EnumeratorName("% Extra"), "PercentExtra");
  CHECK_TEXT("an acronym keeps its case", EnumeratorName("LCY Extra"), "LCYExtra");
  // AL allows a blank member, which is ordinal zero of many option fields.
  CHECK_TEXT("a blank member is named", EnumeratorName(""), "Blank");
  CHECK_TEXT("and so is one made only of punctuation", EnumeratorName(" - "), "Blank");
}

void AnOptionFieldGetsItsOwnEnumeration() {
  // A local option belongs to one field of one table, so its name carries both: two tables with a
  // `Type` field must not share an enumeration, which is the collision the predecessor names as a
  // failure class -- a bare field name resolved to the wrong ordinals.
  CHECK_TEXT("table and field", OptionEnumName("Resource Cost", "Type"), "ResourceCostType");
  CHECK_TEXT("and again", OptionEnumName("Resource Cost", "Cost Type"), "ResourceCostCostType");
}

} // namespace

int main() {
  return gate::Run("GenNames", [] {
    AnAlNameBecomesAnIdentifier();
    AnOptionMemberBecomesAnEnumerator();
    AnOptionFieldGetsItsOwnEnumeration();
  });
}
