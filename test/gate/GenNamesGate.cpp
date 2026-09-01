#include "Check.h"
#include "Names.h"

#include <string>
#include <vector>

using agiru::gen::EnumeratorName;
using agiru::gen::EnumeratorNames;
using agiru::gen::Identifier;
using agiru::gen::Literal;
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

/// AL text goes into a C++ literal, and the two do not agree on what a backslash is.
void AlTextSurvivesBecomingACppLiteral() {
  // AL's backslash is a line break in a message and NOT an escape. Bin Creation Worksheet Line
  // writes `...exceeds the entered %3 %4.\\Do you still want to enter this %3?`, which lands in the
  // C++ literal as `\\D` -- an unknown escape sequence, and an error under -Werror.
  CHECK_TEXT("a backslash is escaped rather than handed to the C++ compiler",
             Literal("%4.\\Do you still"),
             "\"%4.\\\\Do you still\"");
  // The one that is not a warning but a broken file: a quote closes the literal early and
  // everything after it is read as C++.
  CHECK_TEXT("and so is a quote", Literal("say \"no\""), "\"say \\\"no\\\"\"");
  CHECK_TEXT("a plain caption is left exactly as AL wrote it",
             Literal("Group(Resource)"),
             "\"Group(Resource)\"");
  CHECK_TEXT("and a percent sign is not a C++ escape at all", Literal("% Extra"), "\"% Extra\"");
}

/// TWO AL MEMBERS CAN SCRUB TO ONE IDENTIFIER. `Whse. Cross-Dock Opportunity` declares
/// `OptionMembers = ,"Sales Order",,,,` -- a blank marking "none" and four reserved ordinals nobody
/// filled in. Every one becomes `Blank`, and the second is a redefinition the compiler refuses.
void CollidingEnumeratorsAreSeparatedByTheirOrdinal() {
  const std::vector<std::string> members{"", "Sales Order", "", "", ""};
  const std::vector<std::string> names = EnumeratorNames(members);

  CHECK_TRUE("one name per member", names.size() == members.size());
  CHECK_TEXT("the first blank keeps the bare name", names[0], "Blank");
  CHECK_TEXT("a member that is a name keeps it", names[1], "SalesOrder");
  CHECK_TEXT("and every later blank carries the ordinal AL gave it", names[2], "Blank2");
  CHECK_TEXT("so no two are the same", names[3], "Blank3");
  CHECK_TRUE("which is what the compiler needs",
             names[0] != names[2] && names[2] != names[3] && names[3] != names[4]);

  // THE NEGATIVE CONTROL. A rule that renamed unconditionally would also pass the checks above and
  // would change 1047 option enumerations that have nothing wrong with them.
  const std::vector<std::string> distinct = EnumeratorNames({"Resource", "Group(Resource)", "All"});
  CHECK_TEXT("a list with no collision is left alone", distinct[0], "Resource");
  CHECK_TEXT("and so is the rest of it", distinct[2], "All");
}

} // namespace

int main() {
  return gate::Run("GenNames", [] {
    AnAlNameBecomesAnIdentifier();
    AnOptionMemberBecomesAnEnumerator();
    AnOptionFieldGetsItsOwnEnumeration();
    AlTextSurvivesBecomingACppLiteral();
    CollidingEnumeratorsAreSeparatedByTheirOrdinal();
  });
}
