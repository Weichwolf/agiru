#include "Check.h"
#include "ResourceCost.h"
#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Decimal.h"
#include "type/Enum.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Variant.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

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
  CHECK_TRUE("and how many fields it carries -- the six AL declares and the five the platform "
             "adds",
             ref.FieldCount() == 6 + agiru::kSystemFieldCount);
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

// A TABLE WITH BOTH KINDS OF ENUMERATION, because `Resource Cost` declares two Options and no Enum
// -- AL table 202 has `field(1; Type; Option)` and `field(4; "Cost Type"; Option)` -- and the
// question below is what an ENUM field answers.
enum class Kind : std::int32_t { Yes = 0, No = 10 };
enum class Shade : std::int32_t { Pale = 0, Deep = 1 };

struct Painted;

} // namespace

template <> struct agiru::EnumTraits<Kind> {
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      agiru::EnumValueDef{.ordinal = 0, .name = "Yes", .caption = "Yes"},
      agiru::EnumValueDef{.ordinal = 10, .name = "No", .caption = "No"},
  }};
};

template <> struct agiru::OptionTraits<Shade> {
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      agiru::EnumValueDef{.ordinal = 0, .name = "Pale", .caption = "Pale"},
      agiru::EnumValueDef{.ordinal = 1, .name = "Deep", .caption = "Deep"},
  }};
};

namespace {

struct Painted : agiru::Table<Painted> {
  static constexpr agiru::TableId kId{50000};
  static constexpr std::string_view kName{"Painted"};

  agiru::Enum<Kind> Kind;
  agiru::Option<Shade> Shade;

  struct FieldNumber {
    static constexpr agiru::FieldNo Kind{1};
    static constexpr agiru::FieldNo Shade{2};
  };

  static constexpr std::array<agiru::FieldNo, 1> kKey1{{FieldNumber::Kind}};
};

inline constexpr std::array<agiru::FieldDef, 2> kPaintedFields{{
    agiru::Declare<&Painted::Kind>(
        Painted::FieldNumber::Kind, "Kind", "Kind", offsetof(Painted, Kind)),
    agiru::Declare<&Painted::Shade>(
        Painted::FieldNumber::Shade, "Shade", "Shade", offsetof(Painted, Shade)),
}};

inline constexpr std::array<agiru::KeyDef, 1> kPaintedKeys{{
    agiru::KeyDef{.name = "Key1", .fields = Painted::kKey1, .clustered = true},
}};

inline constexpr agiru::TableDef kPaintedTable{.id = Painted::kId,
                                               .name = Painted::kName,
                                               .caption = Painted::kName,
                                               .fields = kPaintedFields,
                                               .keys = kPaintedKeys};

} // namespace

template <> struct agiru::TableTraits<Painted> {
  static constexpr const agiru::TableDef &kTable = kPaintedTable;
};

namespace {

/// AN ENUM FIELD REPORTS `Option`, AND THAT IS THE PLATFORM'S ANSWER RATHER THAN A SIMPLIFICATION.
/// `fieldtype-option.md` tabulates every member of the FieldType that `FieldRef.Type()` returns and
/// there is no `Enum` among them. `BankPmtApplRuleUT` stands on it: it reads `Field.Type` from the
/// virtual Field table, leaves the procedure unless it is `Option`, and then asks the field for
/// `OptionMembers` -- on `Sales Header."Document Type"`, which has been an Enum for years.
void AnEnumFieldReportsOption() {
  Painted rec;
  RecordRef ref;
  ref.GetTable(rec);

  const FieldRef declared = ref.Field(1);
  const FieldRef option = ref.Field(2);

  CHECK_TRUE("an enum field reports Option", declared.Type() == agiru::FieldType::Option);
  CHECK_TRUE("and so does an option field", option.Type() == agiru::FieldType::Option);

  // THE NEGATIVE CONTROL: they are still two different things, and IsEnum is the one place BC puts
  // that difference. A Type() that answered Option for both while IsEnum() also answered the same
  // for both would pass the two checks above and prove nothing.
  CHECK_TRUE("only the enum answers IsEnum", declared.IsEnum());
  CHECK_TRUE("the option does not", !option.IsEnum());

  // AND BOTH HAND OUT THEIR MEMBERS, which is the half `BankPmtApplRuleUT` reaches after the type
  // check: `OptionMembers` on an enum field must not be empty.
  CHECK_TRUE("the enum names its values", declared.OptionMembers().Count() == 2);
  CHECK_TRUE("and so does the option", option.OptionMembers().Count() == 2);
  CHECK_TRUE("the enum keeps its declared ordinal", declared.GetEnumValueOrdinal(2) == 10);
}

/// THE FIELD TYPE'S NUMBERS ARE THE PLATFORM'S OWN AND NOT A COUNTER. AL compares the result of
/// `FieldRef.Type()` against `Field.Type::Code` directly, so a dense 0, 1, 2 ... of this tree's own
/// invention would make every such comparison quietly false -- the silent-wrong-data class, and one
/// that no other case here would catch, because every check inside agiru compares the enum against
/// itself and agrees whatever the numbers are.
void TheFieldTypeCarriesThePlatformsOwnNumbers() {
  CHECK_TRUE("Boolean is 3", static_cast<int>(agiru::FieldType::Boolean) == 3);
  CHECK_TRUE("Option is 5", static_cast<int>(agiru::FieldType::Option) == 5);
  CHECK_TRUE("Integer is 7", static_cast<int>(agiru::FieldType::Integer) == 7);
  CHECK_TRUE("Decimal is 9", static_cast<int>(agiru::FieldType::Decimal) == 9);
  CHECK_TRUE("Date is 11", static_cast<int>(agiru::FieldType::Date) == 11);
  CHECK_TRUE("Blob is 14", static_cast<int>(agiru::FieldType::Blob) == 14);
  CHECK_TRUE("BigInteger is 18", static_cast<int>(agiru::FieldType::BigInteger) == 18);
  CHECK_TRUE("Guid is 21", static_cast<int>(agiru::FieldType::Guid) == 21);
  CHECK_TRUE("DateTime is 22", static_cast<int>(agiru::FieldType::DateTime) == 22);
  CHECK_TRUE("Text is 31", static_cast<int>(agiru::FieldType::Text) == 31);
  CHECK_TRUE("Code is 33", static_cast<int>(agiru::FieldType::Code) == 33);
  CHECK_TRUE("Media is 40", static_cast<int>(agiru::FieldType::Media) == 40);

  // THE GAPS ARE THE POINT. A counter cannot produce them, so their presence is what says these
  // numbers were taken from somewhere rather than assigned here.
  CHECK_TRUE("nothing sits at 4",
             static_cast<int>(agiru::FieldType::Option) -
                     static_cast<int>(agiru::FieldType::Boolean) ==
                 2);
  CHECK_TRUE("and the range is wider than the member count",
             static_cast<int>(agiru::FieldType::Media) > 17);

  // AN ENUM FIELD'S TAG STAYS OUT OF THE PLATFORM'S RANGE, so it cannot escape looking like one.
  CHECK_TRUE("the internal Enum tag is above every platform number",
             static_cast<int>(agiru::FieldType::Enum) > static_cast<int>(agiru::FieldType::Media));
}

} // namespace

int main() {
  return gate::Run("RecordRef", [] {
    ItReachesTheTableWithoutNamingIt();
    OneThatIsNotOpenRefusesRatherThanAnsweringZero();
    AFieldIsReachedByNumberAndByPosition();
    AValueCarriesItsType();
    AnEnumFieldReportsOption();
    TheFieldTypeCarriesThePlatformsOwnNumbers();
    TheEnumAccessorsAnswerByPositionAndByOrdinal();
  });
}
