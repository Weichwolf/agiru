#include "Check.h"
#include "ResourceCost.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <string>

using agiru::Error;
using agiru::FieldRef;
using agiru::RecordRef;
using agiru::app::ResourceCost;
using agiru::app::ResourceCostType;

namespace {

/// A RecordRef REACHES A RECORD BY NUMBER RATHER THAN BY NAME, which is the same address the field
/// table already gives -- arrived at from the other side.
void ItReachesTheTableWithoutNamingIt() {
  ResourceCost rec;
  RecordRef ref;
  CHECK_TRUE("a fresh RecordRef is not open", !ref.IsOpen());
  ref.GetTable(rec);
  CHECK_TRUE("and is open once it points at a record", ref.IsOpen());

  constexpr agiru::Integer kResourceCost = 202;
  CHECK_TRUE("it answers the AL table number", ref.Number() == kResourceCost);
  CHECK_TEXT("and the AL name", std::string(ref.Name()), "Resource Cost");
  CHECK_TRUE("and how many fields the table declares", ref.FieldCount() == 6);
  CHECK_TRUE("and how many keys", ref.KeyCount() == 2);
}

/// THE NEGATIVE CONTROL for every question above: a RecordRef that points at nothing SAYS so rather
/// than answering zero, which would read as a table with no fields.
void OneThatIsNotOpenRefusesRatherThanAnsweringZero() {
  const RecordRef ref;
  std::string said;
  try {
    (void)ref.Number();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("Number refuses", !said.empty());
  said.clear();
  try {
    (void)ref.FieldCount();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("and so does FieldCount", !said.empty());
  CHECK_TRUE("while FieldExist simply answers false", !ref.FieldExist(1));
}

constexpr agiru::Integer kNoSuchField = 999;

void AFieldIsReachedByNumberAndByPosition() {
  ResourceCost rec;
  RecordRef ref;
  ref.GetTable(rec);

  const FieldRef code = ref.Field(2);
  CHECK_TRUE("a field found by number carries it", code.Number() == 2);
  CHECK_TEXT("and its AL name", std::string(code.Name()), "Code");
  CHECK_TRUE("and its declared length", code.Length() == 20);

  const FieldRef first = ref.FieldIndex(1);
  CHECK_TRUE("FieldIndex counts from ONE", first.Number() == 1);
  CHECK_TEXT("and reaches the first field", std::string(first.Name()), "Type");

  CHECK_TRUE("a field the table declares exists", ref.FieldExist(2));
  CHECK_TRUE("and one it does not, does not", !ref.FieldExist(kNoSuchField));

  std::string said;
  try {
    (void)ref.Field(kNoSuchField);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("reaching for a field that is not there refuses", !said.empty());
  said.clear();
  try {
    (void)ref.FieldIndex(0);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("and index 0 is outside the list, not the first field", !said.empty());
}

/// A FIELD'S VALUE CARRIES ITS TYPE OUT OF THE RECORD. This is why a Variant had to tell a Duration
/// from a BigInteger: both are 64 bits in the record and two different answers here.
void AValueCarriesItsType() {
  ResourceCost rec;
  rec.Code = "WELDER";
  rec.UnitCost = agiru::Decimal::FromInvariantString("12.50");
  rec.Type = ResourceCostType::All;

  RecordRef ref;
  ref.GetTable(rec);

  const agiru::Variant code = ref.Field(2).Value();
  CHECK_TRUE("a Code comes out as text", code.IsText());
  CHECK_TEXT("with its value", code.Get<std::string>(), "WELDER");

  const agiru::Variant cost = ref.Field(6).Value();
  CHECK_TRUE("a Decimal comes out as a Decimal", cost.IsDecimal());
  CHECK_TEXT(
      "with its value and its scale", cost.Get<agiru::Decimal>().ToInvariantString(), "12.50");

  // AN OPTION COMES OUT AS ITS ORDINAL, because that is what it is -- and the NAMES come from the
  // same FieldRef, out of the field's declaration.
  const agiru::Variant type = ref.Field(1).Value();
  CHECK_TRUE("an option comes out as an integer ordinal", type.IsInteger());
  CHECK_TRUE("carrying the member's number", type.Get<agiru::Integer>() == 2);
}

/// POSITION AND ORDINAL ARE DIFFERENT QUESTIONS, and the platform gives both accessors because of
/// it. Resource Cost's `Type` is dense, so the gate uses the one that shows the difference: the
/// name reached by ordinal and the name reached by position agree here and would not on a sparse
/// enumeration -- which is what the Enum gate covers.
void TheEnumAccessorsAnswerByPositionAndByOrdinal() {
  ResourceCost rec;
  RecordRef ref;
  ref.GetTable(rec);
  const FieldRef type = ref.Field(1);

  CHECK_TRUE("the option declares three members", type.EnumValueCount() == 3);
  CHECK_TEXT("the first by POSITION", std::string(type.GetEnumValueName(1)), "Resource");
  CHECK_TRUE("whose ordinal is 0", type.GetEnumValueOrdinal(1) == 0);
  CHECK_TEXT("and the same value by ORDINAL",
             std::string(type.GetEnumValueNameFromOrdinalValue(0)),
             "Resource");
  CHECK_TEXT("a member that is no identifier keeps its AL name",
             std::string(type.GetEnumValueName(2)),
             "Group(Resource)");
  CHECK_TRUE("an index outside the list answers nothing rather than the first",
             type.GetEnumValueName(0).empty() && type.GetEnumValueName(4).empty());
  CHECK_TRUE("and an ordinal the enumeration does not declare answers nothing",
             type.GetEnumValueNameFromOrdinalValue(9).empty());
  CHECK_TRUE("the members come out in declaration order", type.OptionMembers().Count() == 3);
  CHECK_TEXT("first", type.OptionMembers().Get(1), "Resource");

  // An OPTION is not an ENUM, and the platform asks that separately.
  CHECK_TRUE("an option field is not an enum field", !type.IsEnum());
}

} // namespace

int main() {
  return gate::Run("RecordRef", [] {
    ItReachesTheTableWithoutNamingIt();
    OneThatIsNotOpenRefusesRatherThanAnsweringZero();
    AFieldIsReachedByNumberAndByPosition();
    AValueCarriesItsType();
    TheEnumAccessorsAnswerByPositionAndByOrdinal();
  });
}
