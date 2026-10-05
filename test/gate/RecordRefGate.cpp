#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Enum.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/KeyRef.h"
#include "type/Option.h"
#include "type/StringValue.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "Check.h"
#include "ResourceCost.h"
#include "Selection.h"
#include "options/Types.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

using agiru::Error;
using agiru::FieldRef;
using agiru::RecordRef;
using ResourceCost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
using ResourceCostType = agiru::options::OptionResourceGroupResourceAll;

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
  // THE SYSTEM FIELDS ARE NOT IN THE INDEX: `ApplicationAreaMgmt` reads every field from the first
  // application area to `FieldCount()` into a Boolean, and on BC that loop never meets SystemId.
  CHECK_TRUE("and how many fields it carries -- the six AL declares, and not the five the platform "
             "adds",
             ref.FieldCount() == 6);
  CHECK_TRUE("while the system fields stay reachable by number",
             ref.FieldExist(agiru::kSystemFields.front().no.Value()));
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
  const FieldRef empty;
  said.clear();
  try {
    (void)empty.Length();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("an unassigned FieldRef refuses Length rather than reporting zero", !said.empty());
}

/// A RECORDREF IS A REFERENCE TYPE, so a copy is a second handle on the same object: the BaseApp
/// passes one BY VALUE, opens it inside the callee, and reads it in the caller
/// (`FindRecordManagement.GetRecRefAndFieldsNoByType`, 11 UT cases on 2026-09-09).
void ACopyIsASecondHandleOnTheSameObject() {
  ResourceCost rec;
  const RecordRef ref;
  RecordRef copy = ref;
  CHECK_TRUE("both start closed", !ref.IsOpen() && !copy.IsOpen());
  copy.GetTable(rec);
  CHECK_TRUE("opening the copy opens the original", ref.IsOpen());
  CHECK_TRUE("and it is the same table",
             ref.Number() == agiru::TableTraits<ResourceCost>::kTable.id.Value());
  RecordRef assigned;
  assigned = ref;
  assigned.Close();
  CHECK_TRUE("closing through an assigned handle closes them all", !ref.IsOpen() && !copy.IsOpen());

  // THE NEGATIVE CONTROL: two handles made apart are two objects.
  RecordRef other;
  other.GetTable(rec);
  CHECK_TRUE("a RecordRef made on its own is not touched", !ref.IsOpen());
}

constexpr agiru::Integer kNoSuchField = 999;

void AVariantRetainsTheRecordRefBeyondItsSourceVariable() {
  agiru::Variant held;
  {
    RecordRef local;
    local.Open(agiru::TableTraits<ResourceCost>::kTable.id.Value());
    held = local;
    CHECK_TRUE("a Variant retains a handle instead of borrowing the local variable",
               &static_cast<const RecordRef &>(held) != &local);
    const agiru::Variant second = local;
    CHECK_TRUE("two boxed handles retain the same RecordRef identity", held == second);
    RecordRef unrelated;
    unrelated.Open(agiru::TableTraits<ResourceCost>::kTable.id.Value());
    CHECK_TRUE("the same table does not imply the same reference identity",
               held != agiru::Variant(unrelated));
  }
  const auto &retained = static_cast<const RecordRef &>(held);
  CHECK_TRUE("the boxed reference survives its source variable", retained.IsOpen());
  CHECK_TRUE("the owned table also survives",
             retained.Number() == agiru::TableTraits<ResourceCost>::kTable.id.Value());
  auto copy = held;
  CHECK_TRUE("a copied Variant retains reference identity", copy == held);
  auto replacement = held;
  const RecordRef closed;
  static_cast<RecordRef &>(replacement) = closed;
  CHECK_TRUE("assigning another boxed handle does not overwrite the original box",
             !static_cast<const RecordRef &>(replacement).IsOpen() && retained.IsOpen());
  static_cast<RecordRef &>(copy).Close();
  CHECK_TRUE("boxing preserves the shared RecordRef state", !retained.IsOpen());
  held = agiru::Variant{};
  CHECK_TRUE("clearing one Variant does not clear another box", copy.IsRecordRef());
}

void AFieldIsReachedByNumberAndByPosition() {
  ResourceCost rec;
  RecordRef ref;
  ref.GetTable(rec);

  const FieldRef code = ref.Field(2);
  CHECK_TRUE("a field found by number carries it", code.Number() == 2);
  CHECK_TEXT("and its AL name", std::string(code.Name()), "Code");
  CHECK_TRUE("and its declared length", code.Length() == 20);
  CHECK_TRUE("a Decimal has the platform size rather than its host wrapper size",
             ref.Field(ResourceCost::Field_No::UnitCost.Value()).Length() == 12);
  CHECK_TRUE("a system GUID has its fixed size",
             ref.Field(agiru::SystemFieldNumbers::SystemId.Value()).Length() == 16);
  CHECK_TRUE("an audit timestamp has its fixed size",
             ref.Field(agiru::SystemFieldNumbers::SystemCreatedAt.Value()).Length() == 8);

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

void FieldAndKeyReferencesKeepTheirRecordAlive() {
  FieldRef field;
  agiru::KeyRef key;
  {
    ResourceCost rec;
    rec.Code = "KEPT";
    RecordRef owner;
    owner.GetTable(rec);
    field = owner.Field(2);
    key = owner.KeyIndex(1);
  }
  CHECK_TEXT("a FieldRef outlives the RecordRef variable that made it", field.ToText(), "KEPT");
  const RecordRef fromField = field.Record();
  CHECK_TEXT("FieldRef.Record keeps the same row alive", fromField.Field(2).ToText(), "KEPT");
  const RecordRef fromKey = key.Record();
  CHECK_TEXT("KeyRef.Record keeps the same row alive", fromKey.Field(2).ToText(), "KEPT");
  const FieldRef copied = field;
  const agiru::KeyRef moved = std::move(key);
  CHECK_TEXT("a copied FieldRef shares the live row", copied.ToText(), "KEPT");
  CHECK_TEXT("a moved KeyRef keeps the live row", moved.Record().Field(2).ToText(), "KEPT");
  fromField.Field(2).Value("CHANGED");
  CHECK_TEXT("FieldRef.Record changes reach the originating field", field.ToText(), "CHANGED");
  CHECK_TEXT(
      "KeyRef.Record observes the same changed row", moved.Record().Field(2).ToText(), "CHANGED");
  copied.Record().Close();
  CHECK_TRUE("the retained declaration survives closing the record", field.Length() == 20);
  std::string said;
  try {
    (void)field.ToText();
  } catch (const Error &error) { said = error.what(); }
  CHECK_TRUE("closing one shared handle invalidates every field handle safely", !said.empty());
}

/// `SetPosition` IS THE WAY BACK FROM `GetPosition` (`recordref-setposition-method.md`): the key
/// fields take the values the text carries and nothing else moves. `Bin Content` hands a position
/// through a `RecordRef` to reopen a row (SCM - Warehouse UT, 3 cases refused, 2026-09-12).
void APositionRoundTripsThroughARecordRef() {
  ResourceCost rec;
  rec.Type = ResourceCostType::Resource;
  rec.Code = "R-1";
  rec.WorkTypeCode = "WT";
  RecordRef ref;
  ref.GetTable(rec);
  const std::string position = ref.GetPosition();
  ResourceCost other;
  other.WorkTypeCode = "ELSE";
  RecordRef back;
  back.GetTable(other);
  back.SetPosition(position);
  back.SetTable(other);
  CHECK_TEXT("the code comes back", std::string(std::string_view(other.Code)), "R-1");
  CHECK_TEXT("and so does the work type", std::string(std::string_view(other.WorkTypeCode)), "WT");
  CHECK_TRUE("and the type", other.Type == ResourceCostType::Resource);
  // THE NEGATIVE CONTROL: a part naming a field the table lacks refuses rather than being dropped.
  std::string said;
  try {
    back.SetPosition("9999='x'");
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a position over an unknown field refuses", !said.empty());
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
  CHECK_TEXT("with its value", std::string_view(code.Get<agiru::Text<0>>()), "WELDER");

  const agiru::Variant cost = ref.Field(6).Value();
  CHECK_TRUE("a Decimal comes out as a Decimal", cost.IsDecimal());
  CHECK_TEXT(
      "with its value and its scale", cost.Get<agiru::Decimal>().ToInvariantString(), "12.50");

  // AN OPTION COMES OUT AS AN OPTION (`fieldref-value-method.md`: the Variant answers `IsOption`),
  // carrying its ordinal and the member names out of the field's declaration -- so
  // `MyOption := FieldRef.Value` assigns and `Format` names the member (measured 2026-09-10, 30 UT
  // cases that read "the Variant does not hold that type (alternative 2)").
  const agiru::Variant type = ref.Field(1).Value();
  CHECK_TRUE("an option comes out as an option", type.IsOption() && !type.IsInteger());
  CHECK_TRUE("carrying the member's number", type.Get<agiru::OrdinalInVariant>().ordinal == 2);
  CHECK_TRUE("and the member names", type.Get<agiru::OrdinalInVariant>().values.size() == 3);
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
  CHECK_TRUE("the members come out in declaration order, comma-joined as the page says",
             type.OptionMembers().starts_with("Resource,"));

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

// Runtime-18 system FlowField declarations: devenv-table-system-fields.md.
constexpr std::size_t kAuditUserNameLength = 50;
constexpr std::size_t kAuditFullNameLength = 80;
constexpr std::size_t kPaintedDeclaredCount = 4;
constexpr std::size_t kRuntime18ImplicitCount = 10;
constexpr std::array<agiru::FieldNo, 4> kAuditNameNumbers{{
    agiru::FieldNo{2000000005},
    agiru::FieldNo{2000000006},
    agiru::FieldNo{2000000007},
    agiru::FieldNo{2000000008},
}};

struct Painted : agiru::Table<Painted> {
  agiru::detail::StateHandle State_Block;
  static constexpr agiru::TableId kId{50000};
  static constexpr std::string_view kName{"Painted"};

  agiru::Enum<::Kind> Kind;
  agiru::Option<::Shade> Shade;
  agiru::Integer Painted_Count{};
  agiru::Date Painted_On{};

  agiru::BigInteger SystemRowVersion{};
  agiru::Guid SystemId;
  agiru::DateTime SystemCreatedAt;
  agiru::Guid SystemCreatedBy;
  agiru::DateTime SystemModifiedAt;
  agiru::Guid SystemModifiedBy;
  agiru::Text<kAuditUserNameLength> SystemCreatedByUserName;
  agiru::Text<kAuditFullNameLength> SystemCreatedByFullName;
  agiru::Text<kAuditUserNameLength> SystemModifiedByUserName;
  agiru::Text<kAuditFullNameLength> SystemModifiedByFullName;

  struct Field_No {
    static constexpr agiru::FieldNo Kind{1};
    static constexpr agiru::FieldNo Shade{2};
    static constexpr agiru::FieldNo Painted_Count{3};
    static constexpr agiru::FieldNo Painted_On{4};
  };

  static constexpr std::array<agiru::FieldNo, 2> kKey1{{Field_No::Shade, Field_No::Kind}};
};

inline constexpr std::array<agiru::FieldDef, 4> kPaintedFields{{
    agiru::Declare<&Painted::Kind>(
        Painted::Field_No::Kind, "Kind", "Kind", offsetof(Painted, Kind)),
    agiru::Declare<&Painted::Shade>(
        Painted::Field_No::Shade, "Shade", "Shade", offsetof(Painted, Shade)),
    agiru::Declare<&Painted::Painted_Count>(
        Painted::Field_No::Painted_Count,
        "Painted Count",
        "Painted Count",
        offsetof(Painted, Painted_Count),
        agiru::Declared{.fieldClass = agiru::FieldClass::FlowField,
                        .calcFormula = "Count(\"Painted\")"}),
    agiru::Declare<&Painted::Painted_On>(
        Painted::Field_No::Painted_On,
        "Painted On",
        "Painted On",
        offsetof(Painted, Painted_On),
        agiru::Declared{.fieldClass = agiru::FieldClass::FlowFilter}),
}};

inline constexpr std::array<agiru::KeyDef, 1> kPaintedKeys{{
    agiru::KeyDef{.name = "Key1", .fields = Painted::kKey1, .clustered = true},
}};

constexpr auto PaintedImplicitProfile() {
  const auto base = agiru::WithSystemFields<Painted>(kPaintedFields);
  std::array<agiru::FieldDef, kPaintedDeclaredCount + kRuntime18ImplicitCount> fields{};
  fields[0] = agiru::Declare<&Painted::SystemRowVersion>(
      agiru::FieldNo{0}, "timestamp", "timestamp", offsetof(Painted, SystemRowVersion));
  for (std::size_t i = 0; i < base.size(); ++i) { fields[i + 1] = base[i]; }
  std::size_t output = base.size() + 1;
  fields[output++] = agiru::Declare<&Painted::SystemCreatedByUserName>(
      kAuditNameNumbers[0],
      "SystemCreatedByUserName",
      "SystemCreatedByUserName",
      offsetof(Painted, SystemCreatedByUserName),
      agiru::Declared{.fieldClass = agiru::FieldClass::FlowField, .editable = false});
  fields[output++] = agiru::Declare<&Painted::SystemCreatedByFullName>(
      kAuditNameNumbers[1],
      "SystemCreatedByFullName",
      "SystemCreatedByFullName",
      offsetof(Painted, SystemCreatedByFullName),
      agiru::Declared{.fieldClass = agiru::FieldClass::FlowField, .editable = false});
  fields[output++] = agiru::Declare<&Painted::SystemModifiedByUserName>(
      kAuditNameNumbers[2],
      "SystemModifiedByUserName",
      "SystemModifiedByUserName",
      offsetof(Painted, SystemModifiedByUserName),
      agiru::Declared{.fieldClass = agiru::FieldClass::FlowField, .editable = false});
  fields[output] = agiru::Declare<&Painted::SystemModifiedByFullName>(
      kAuditNameNumbers[3],
      "SystemModifiedByFullName",
      "SystemModifiedByFullName",
      offsetof(Painted, SystemModifiedByFullName),
      agiru::Declared{.fieldClass = agiru::FieldClass::FlowField, .editable = false});
  return fields;
}

inline constexpr auto kPaintedImplicitFields = PaintedImplicitProfile();

inline constexpr agiru::TableDef kPaintedTable{.id = Painted::kId,
                                               .name = Painted::kName,
                                               .caption = Painted::kName,
                                               .fields = kPaintedImplicitFields,
                                               .keys = kPaintedKeys};

} // namespace

template <> struct agiru::TableTraits<Painted> {
  static constexpr const agiru::TableDef &kTable = kPaintedTable;
};

namespace {
// THE GATE'S OWN TABLE IS IN THE CATALOGUE, the way every generated definition unit registers its
// table: `RecordRef.Open(50000)` looks it up there, and without this line the gate refused itself
// with "this installation carries no table 50000".
const agiru::RegisterTable<Painted> kPaintedInCatalogue;
}

namespace {

void ImplicitFieldsDoNotEnterTheDeclaredIndex() {
  constexpr agiru::BigInteger kTimestampValue = 37;
  Painted record;
  record.SystemRowVersion = kTimestampValue;
  record.SystemCreatedByUserName = "CREATOR";
  RecordRef ref;
  ref.GetTable(record);
  CHECK_TRUE("all ten implicit fields are present in the authored Runtime-18 fixture",
             kPaintedTable.fields.size() == kPaintedDeclaredCount + kRuntime18ImplicitCount);
  CHECK_TRUE("FieldCount excludes timestamp and all nine reserved fields", ref.FieldCount() == 4);
  constexpr std::array declared{2, 1, 3, 4};
  for (std::size_t i = 0; i < declared.size(); ++i) {
    CHECK_TRUE("the AL field index preserves declared fields and primary-first order",
               ref.FieldIndex(static_cast<int>(i + 1)).Number() == declared[i]);
  }
  constexpr std::array implicit{0,
                                2000000000,
                                2000000001,
                                2000000002,
                                2000000003,
                                2000000004,
                                2000000005,
                                2000000006,
                                2000000007,
                                2000000008};
  for (const int no : implicit) {
    CHECK_TRUE("every implicit field remains reachable by number", ref.FieldExist(no));
    CHECK_TRUE("the shared implicit predicate recognizes every reference identity",
               agiru::IsImplicitSystemField(agiru::FieldNo{no}));
  }
  CHECK_TRUE("timestamp retains its typed value", ref.Field(0).Value().IsBigInteger());
  CHECK_TEXT("timestamp reflection reads actual member storage", ref.Field(0).ToText(), "37");
  CHECK_TEXT("audit-name reflection reads actual member storage",
             ref.Field(kAuditNameNumbers[0].Value()).ToText(),
             "CREATOR");
  std::string refused;
  try {
    (void)ref.FieldIndex(static_cast<int>(kPaintedDeclaredCount + 1));
  } catch (const Error &error) { refused = error.what(); }
  CHECK_TRUE("the first implicit field cannot be reached through a declared index",
             !refused.empty());
  CHECK_TRUE("the reserved range does not include timestamp",
             !agiru::IsReservedSystemField(agiru::FieldNo{0}));
  CHECK_TRUE("the reserved-range boundary excludes its immediate predecessor",
             !agiru::IsImplicitSystemField(agiru::FieldNo{1999999999}));
  CHECK_TRUE("the reserved range includes its largest signed identifier",
             agiru::IsImplicitSystemField(agiru::FieldNo{2147483647}));
  CHECK_TRUE("ordinary and invalid negative numbers are not implicit fields",
             !agiru::IsImplicitSystemField(agiru::FieldNo{1}) &&
                 !agiru::IsImplicitSystemField(agiru::FieldNo{-1}));
}

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
  CHECK_TRUE("an enum has the original Option byte size", declared.Length() == 4);
  CHECK_TRUE("and an ordinary option has the same byte size", option.Length() == 4);

  // THE NEGATIVE CONTROL: they are still two different things, and IsEnum is the one place BC puts
  // that difference. A Type() that answered Option for both while IsEnum() also answered the same
  // for both would pass the two checks above and prove nothing.
  CHECK_TRUE("only the enum answers IsEnum", declared.IsEnum());
  CHECK_TRUE("the option does not", !option.IsEnum());

  // AND BOTH HAND OUT THEIR MEMBERS, which is the half `BankPmtApplRuleUT` reaches after the type
  // check: `OptionMembers` on an enum field must not be empty.
  CHECK_TRUE("the enum names its values", declared.OptionMembers().find(',') != std::string::npos);
  CHECK_TRUE("and so does the option", option.OptionMembers().find(',') != std::string::npos);
  CHECK_TRUE("the enum keeps its declared ordinal", declared.GetEnumValueOrdinal(2) == 10);
}

void AFieldAnswersTheClassItsTableDeclared() {
  Painted rec;
  RecordRef ref;
  ref.GetTable(rec);

  CHECK_TRUE("a FlowField says so", ref.Field(3).Class() == agiru::FieldClass::FlowField);
  CHECK_TRUE("a FlowFilter says so", ref.Field(4).Class() == agiru::FieldClass::FlowFilter);
  CHECK_TRUE("a calculated Integer has the fixed size", ref.Field(3).Length() == 4);
  CHECK_TRUE("a date filter has the fixed size", ref.Field(4).Length() == 4);

  // THE NEGATIVE CONTROL IS THE ORDINARY FIELD. `devenv-fieldclass-property.md` makes Normal the
  // default, so a Class() that returned a hardcoded Normal passes on field 1 and fails on the two
  // above -- which is why the ordinary field is checked LAST and never alone.
  CHECK_TRUE("and a field that declares none is Normal",
             ref.Field(1).Class() == agiru::FieldClass::Normal);
}

/// A CALCFORMULA TERM OVER THE TARGET'S OWN FLOWFIELD IS A CORRELATED SUBQUERY, the way a filter on
/// a FlowField is: `Service Header."No. of Unallocated Items"` counts the item lines whose
/// `"No. of Active/Finished Allocs" = const(0)`, itself a count, and reading that term as a column
/// failed the statement ("column does not exist", ERM VAT Tool - UT and Payment Registration UT,
/// 3 cases, 2026-09-12). `Painted` is the target here: its `Painted Count` is a FlowField.
void AFormulaTermOverAFlowFieldIsASubquery() {
  static constexpr std::array<agiru::FieldDef, 2> kFields{{
      agiru::FieldDef{.offset = 0,
                      .name = "Code",
                      .caption = "Code",
                      .no = agiru::FieldNo{1},
                      .type = agiru::FieldType::Code},
      agiru::FieldDef{.offset = 32,
                      .name = "Unpainted",
                      .caption = "Unpainted",
                      .calcFormula = R"(count("Painted" where("Painted Count" = const(0))))",
                      .no = agiru::FieldNo{2},
                      .fieldClass = agiru::FieldClass::FlowField,
                      .type = agiru::FieldType::Integer},
  }};
  static constexpr std::array<agiru::FieldNo, 1> kKey{{agiru::FieldNo{1}}};
  static constexpr std::array<agiru::KeyDef, 1> kKeys{{
      agiru::KeyDef{.name = "Key1", .fields = kKey, .clustered = true},
  }};
  static constexpr agiru::TableDef kOver{.id = agiru::TableId{50004},
                                         .name = "Flow Over Painted",
                                         .caption = "Flow Over Painted",
                                         .fields = kFields,
                                         .keys = kKeys};
  agiru::detail::RecordState state;
  state.filters.push_back(
      agiru::detail::FieldFilter{.field = agiru::FieldNo{2}, .group = 0, .text = "1"});
  const agiru::detail::Selection made = agiru::detail::Select(&state, kOver);
  CHECK_TRUE("the term is the FlowField's own subquery",
             made.where.find("(SELECT count(*) FROM \"Painted\")") != std::string::npos ||
                 made.where.find("(SELECT COUNT(*) FROM \"Painted\")") != std::string::npos);
  CHECK_TRUE("and never the FlowField's name as a column",
             made.where.find("\"Painted Count\" =") == std::string::npos);
  CHECK_TRUE("the constant is bound after the subquery",
             made.binds.size() == 2 && made.binds[0] == "0" && made.binds[1] == "1");
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
    ACopyIsASecondHandleOnTheSameObject();
    AVariantRetainsTheRecordRefBeyondItsSourceVariable();
    ItReachesTheTableWithoutNamingIt();
    OneThatIsNotOpenRefusesRatherThanAnsweringZero();
    AFieldIsReachedByNumberAndByPosition();
    FieldAndKeyReferencesKeepTheirRecordAlive();
    APositionRoundTripsThroughARecordRef();
    AValueCarriesItsType();
    AnEnumFieldReportsOption();
    TheFieldTypeCarriesThePlatformsOwnNumbers();
    TheEnumAccessorsAnswerByPositionAndByOrdinal();
    ImplicitFieldsDoNotEnterTheDeclaredIndex();
    AFieldAnswersTheClassItsTableDeclared();
    AFormulaTermOverAFlowFieldIsASubquery();
  });
}
