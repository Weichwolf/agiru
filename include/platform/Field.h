#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief The AL virtual table `Field` (2000000041) -- one row per field of every table.

namespace agiru::platform {

/// \brief AL `FieldClass` -- what a field IS, as against what it holds.
///
/// `FindRecordManagement` and its neighbours branch on it: `if FldRef.Class = FieldClass::FlowField
/// then ...`. Measured from `~/Git/openerp/openerp/runtime/al_system_enums.py`.
enum class FieldClass : std::int32_t {
  Normal = 0,     ///< Stored in the row.
  FlowField = 1,  ///< Calculated, and therefore not a column (board:0019).
  FlowFilter = 2, ///< A filter carried on the record rather than a value.
};

/// \brief AL `ObsoleteState` -- how far along a field is on its way out.
///
/// \note THE ORDER IS THE DOCUMENTED ONE AS FAR AS IT GOES. `devenv-obsoletestate-property.md`
///       lists `No` first as "the normal/default setting" and `Pending` second, and names the full
///       set as `Moved`, `No`, `Pending`, `PendingMove`, `Removed` -- alphabetically, which is not
///       an ordinal order. The last three are ordered here by the version that introduced them,
///       which is [SET] rather than measured, and it costs nothing until AL compares one: the
///       BaseApp writes `ObsoleteState = Pending` 3 445 times and `Removed` 1 321 times, both of
///       which are pinned.
enum class ObsoleteState : std::int32_t {
  No = 0,          ///< Not obsolete.
  Pending = 1,     ///< Will become obsolete.
  Removed = 2,     ///< Gone, and the declaration is a headstone.
  Moved = 3,       ///< Moved to another extension.
  PendingMove = 4, ///< On its way to another extension.
};

}

namespace agiru::detail {

/// \brief One value of an option that has gaps.
struct Named {
  std::int32_t ordinal;  ///< Its number.
  std::string_view name; ///< Its AL name.
};

/// \brief Fills the gaps of a sparse option with blank members.
///
/// \tparam N How many ordinals the option spans, counting from zero.
/// \param named The values that have a name.
/// \return Every ordinal from 0 to N-1, named or blank.
///
/// NAV WRITES A SPARSE OPTION AS A DENSE ONE WITH BLANKS. `option-data-type.md` promises an option
/// is "zero-based ... assigned to sequential numbers, starting with 0", and `FieldType` runs 3, 5,
/// 7, ... 40 -- both are true at once because the OptionString carries an empty member at every
/// gap. This builds that string's table, so `Option` can keep asserting density.
template <std::size_t N>
constexpr std::array<EnumValueDef, N> Sparse(std::span<const Named> named) {
  std::array<EnumValueDef, N> values{};
  for (std::size_t i = 0; i < N; ++i) {
    values[i] = EnumValueDef{.ordinal = static_cast<std::int32_t>(i), .name = "", .caption = ""};
  }
  for (const Named &one : named) {
    values[static_cast<std::size_t>(one.ordinal)] =
        EnumValueDef{.ordinal = one.ordinal, .name = one.name, .caption = one.name};
  }
  return values;
}

/// \brief The named values of AL's `FieldType`, which `Field.Type` and `FieldRef.Type()` return.
inline constexpr std::array<Named, 17> kFieldTypeNames{{
    {.ordinal = 3, .name = "Boolean"},
    {.ordinal = 5, .name = "Option"},
    {.ordinal = 7, .name = "Integer"},
    {.ordinal = 9, .name = "Decimal"},
    {.ordinal = 11, .name = "Date"},
    {.ordinal = 12, .name = "Time"},
    {.ordinal = 14, .name = "BLOB"},
    {.ordinal = 15, .name = "DateFormula"},
    {.ordinal = 18, .name = "BigInteger"},
    {.ordinal = 20, .name = "Duration"},
    {.ordinal = 21, .name = "GUID"},
    {.ordinal = 22, .name = "DateTime"},
    {.ordinal = 23, .name = "RecordID"},
    {.ordinal = 31, .name = "Text"},
    {.ordinal = 33, .name = "Code"},
    {.ordinal = 39, .name = "MediaSet"},
    {.ordinal = 40, .name = "Media"},
}};

}

/// \brief The vocabulary of AL `FieldType`, which the virtual table reports as an option.
template <> struct agiru::OptionTraits<agiru::FieldType> {
  /// \brief Every ordinal from 0 to 40, named where AL names one.
  static constexpr auto kValues = agiru::detail::Sparse<41>(agiru::detail::kFieldTypeNames);
};

/// \brief The vocabulary of AL `FieldClass`.
template <> struct agiru::OptionTraits<agiru::platform::FieldClass> {
  /// \brief The three classes, which are dense already.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "Normal", .caption = "Normal"},
      {.ordinal = 1, .name = "FlowField", .caption = "FlowField"},
      {.ordinal = 2, .name = "FlowFilter", .caption = "FlowFilter"},
  }};
};

/// \brief The vocabulary of AL `ObsoleteState`.
template <> struct agiru::OptionTraits<agiru::platform::ObsoleteState> {
  /// \brief The five states.
  static constexpr std::array<agiru::EnumValueDef, 5> kValues{{
      {.ordinal = 0, .name = "No", .caption = "No"},
      {.ordinal = 1, .name = "Pending", .caption = "Pending"},
      {.ordinal = 2, .name = "Removed", .caption = "Removed"},
      {.ordinal = 3, .name = "Moved", .caption = "Moved"},
      {.ordinal = 4, .name = "PendingMove", .caption = "PendingMove"},
  }};
};

namespace agiru::platform {

/// \brief The AL virtual table `Field` -- one row per field of every table in the catalogue.
///
/// \note IT IS NOT AN AL OBJECT. No `.al` file declares it: the platform computes its rows, which
///       is why it lives here and not in `apps/` (board:0032). To AL code it is an ordinary table,
///       and the BaseApp uses it as one -- `Field.SetRange(Type, Field.Type::Code)`,
///       `Field.SetFilter("Field Name", '*Global Dimension*')`.
///
/// \note MOST USES ARE TEMPORARY. Measured over BCApps on 2026-09-02: 1 069 `Record Field`
///       declarations, of which 282 carry `temporary`. A temporary one needs nothing but this
///       declaration, because `Temporary<T>` keeps its own store and touches no database.
///
/// \note THE FIELD NUMBERS 1 TO 13 ARE MEASURED, the three after them are [SET].
///       `~/Git/openerp/openerp/runtime/base/system_tables.py` carries the first thirteen with
///       their numbers; it carries `Field Caption`, `Enabled` and `Is Part of Primary Key` without
///       any, and says why -- the BaseApp reaches them BY NAME, which the generator turns into a
///       member access, so the number is not part of that contract. They are numbered here so the
///       field table can exist at all, and the origin is written down rather than implied.
class Field : public Table<Field> {
public:
  /// \brief The AL table number of the virtual `Field` table.
  static constexpr TableId kId{2000000041};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Field"};

  /// \brief The declared lengths, which are AL's and not this file's.
  static constexpr std::size_t kNameLength = 30;
  /// \brief The declared length of `FieldCaption`.
  static constexpr std::size_t kCaptionLength = 80;
  /// \brief The declared length of `ObsoleteReason`.
  static constexpr std::size_t kReasonLength = 248;
  /// \brief The declared length of `OptionString`.
  static constexpr std::size_t kOptionStringLength = 250;

  /// \brief AL `Field."TableNo"`.
  ::agiru::Integer TableNo{};
  /// \brief AL `Field."No."`.
  ::agiru::Integer No{};
  /// \brief AL `Field."TableName"`.
  Text<kNameLength> TableName;
  /// \brief AL `Field."FieldName"`.
  Text<kNameLength> FieldName;
  /// \brief AL `Field."Type"`.
  Option<agiru::FieldType> Type;
  /// \brief AL `Field."Len"`.
  ::agiru::Integer Len{};
  /// \brief AL `Field."Class"`.
  Option<FieldClass> Class;
  /// \brief AL `Field."RelationTableNo"`.
  ::agiru::Integer RelationTableNo{};
  /// \brief AL `Field."RelationFieldNo"`.
  ::agiru::Integer RelationFieldNo{};
  /// \brief AL `Field."OptionString"`.
  Text<kOptionStringLength> OptionString;
  /// \brief AL `Field."ObsoleteState"`.
  Option<ObsoleteState> ObsoleteState;
  /// \brief AL `Field."ObsoleteReason"`.
  Text<kReasonLength> ObsoleteReason;
  /// \brief AL `Field."Field Caption"`.
  Text<kCaptionLength> FieldCaption;
  /// \brief AL `Field."Enabled"`.
  Boolean Enabled{};
  /// \brief AL `Field."Is Part of Primary Key"`.
  Boolean IsPartOfPrimaryKey{};

  /// \note NO SYSTEM FIELDS. A virtual table is not stored, so there is no row to carry a SystemId
  ///       or an audit stamp -- `devenv-virtual-tables.md`: "Virtual tables aren't stored in the
  ///       database, but are computed at runtime". This is why it does not derive from
  ///       `SystemFieldNumbers` the way a generated table does.
  struct Field_No {
    /// \brief The AL field number of `TableNo`.
    static constexpr ::agiru::FieldNo TableNo{1};
    /// \brief The AL field number of `No.`.
    static constexpr ::agiru::FieldNo No{2};
    /// \brief The AL field number of `TableName`.
    static constexpr ::agiru::FieldNo TableName{3};
    /// \brief The AL field number of `FieldName`.
    static constexpr ::agiru::FieldNo FieldName{4};
    /// \brief The AL field number of `Type`.
    static constexpr ::agiru::FieldNo Type{5};
    /// \brief The AL field number of `Len`.
    static constexpr ::agiru::FieldNo Len{6};
    /// \brief The AL field number of `Class`.
    static constexpr ::agiru::FieldNo Class{7};
    /// \brief The AL field number of `RelationTableNo`.
    static constexpr ::agiru::FieldNo RelationTableNo{8};
    /// \brief The AL field number of `RelationFieldNo`.
    static constexpr ::agiru::FieldNo RelationFieldNo{9};
    /// \brief The AL field number of `OptionString`.
    static constexpr ::agiru::FieldNo OptionString{10};
    /// \brief The AL field number of `ObsoleteState`.
    static constexpr ::agiru::FieldNo ObsoleteState{11};
    /// \brief The AL field number of `ObsoleteReason`.
    static constexpr ::agiru::FieldNo ObsoleteReason{12};
    /// \brief The AL field number of `Field Caption`.
    static constexpr ::agiru::FieldNo FieldCaption{20};
    /// \brief The AL field number of `Enabled`.
    static constexpr ::agiru::FieldNo Enabled{21};
    /// \brief The AL field number of `Is Part of Primary Key`.
    static constexpr ::agiru::FieldNo IsPartOfPrimaryKey{22};
  };

  /// \brief The primary key: the table and the field within it.
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{{Field_No::TableNo, Field_No::No}};
};

/// \brief The field table of the virtual `Field` table, as static const data.
inline constexpr std::array<FieldDef, 15> kFieldFields{{
    Declare<&Field::TableNo>(
        Field::Field_No::TableNo, "TableNo", "TableNo", offsetof(Field, TableNo)),
    Declare<&Field::No>(Field::Field_No::No, "No.", "No.", offsetof(Field, No)),
    Declare<&Field::TableName>(
        Field::Field_No::TableName, "TableName", "TableName", offsetof(Field, TableName)),
    Declare<&Field::FieldName>(
        Field::Field_No::FieldName, "FieldName", "FieldName", offsetof(Field, FieldName)),
    Declare<&Field::Type>(Field::Field_No::Type, "Type", "Type", offsetof(Field, Type)),
    Declare<&Field::Len>(Field::Field_No::Len, "Len", "Len", offsetof(Field, Len)),
    Declare<&Field::Class>(Field::Field_No::Class, "Class", "Class", offsetof(Field, Class)),
    Declare<&Field::RelationTableNo>(Field::Field_No::RelationTableNo,
                                     "RelationTableNo",
                                     "RelationTableNo",
                                     offsetof(Field, RelationTableNo)),
    Declare<&Field::RelationFieldNo>(Field::Field_No::RelationFieldNo,
                                     "RelationFieldNo",
                                     "RelationFieldNo",
                                     offsetof(Field, RelationFieldNo)),
    Declare<&Field::OptionString>(Field::Field_No::OptionString,
                                  "OptionString",
                                  "OptionString",
                                  offsetof(Field, OptionString)),
    Declare<&Field::ObsoleteState>(Field::Field_No::ObsoleteState,
                                   "ObsoleteState",
                                   "ObsoleteState",
                                   offsetof(Field, ObsoleteState)),
    Declare<&Field::ObsoleteReason>(Field::Field_No::ObsoleteReason,
                                    "ObsoleteReason",
                                    "ObsoleteReason",
                                    offsetof(Field, ObsoleteReason)),
    Declare<&Field::FieldCaption>(Field::Field_No::FieldCaption,
                                  "Field Caption",
                                  "Field Caption",
                                  offsetof(Field, FieldCaption)),
    Declare<&Field::Enabled>(
        Field::Field_No::Enabled, "Enabled", "Enabled", offsetof(Field, Enabled)),
    Declare<&Field::IsPartOfPrimaryKey>(Field::Field_No::IsPartOfPrimaryKey,
                                        "Is Part of Primary Key",
                                        "Is Part of Primary Key",
                                        offsetof(Field, IsPartOfPrimaryKey)),
}};

/// \brief The keys of the virtual `Field` table.
inline constexpr std::array<KeyDef, 1> kFieldKeys{{
    KeyDef{.name = "Key1", .fields = Field::kKey1, .clustered = true},
}};

/// \brief The declaration of the virtual `Field` table.
inline constexpr TableDef kFieldTable{.id = Field::kId,
                                      .name = Field::kName,
                                      .caption = Field::kName,
                                      .fields = kFieldFields,
                                      .keys = kFieldKeys};

static_assert(FieldsAreSorted(kFieldTable), "the field table is searched by number");
static_assert(std::is_standard_layout_v<Field>,
              "offsetof reaches a field only in a standard-layout "
              "record");

}

/// \brief What the runtime reaches the virtual `Field` table through.
template <> struct agiru::TableTraits<agiru::platform::Field> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kFieldTable;
};
