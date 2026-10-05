#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/ReflectionOptions.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/DataClassification.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

/// \file
/// \brief The AL virtual table `Field` (2000000041) -- one row per field of every table.

namespace agiru::platform {

/// \brief AL `FieldClass` -- what a field IS, as against what it holds.
///
/// `Virtual Tables/Field.Table.al` declares `Normal,FlowField,FlowFilter`.
enum class FieldClass : std::int32_t {
  Normal = 0,     ///< Stored in the row.
  FlowField = 1,  ///< Calculated, and therefore not a column (board:0019).
  FlowFilter = 2, ///< A filter carried on the record rather than a value.
};

/// \brief `Field.Type` codes from `Field.Table.al`, not internal metadata or FieldRef codes.
enum class FieldDataType : std::int32_t {
  TableFilter = 4912,  ///< AL TableFilter.
  RecordId = 4988,     ///< AL RecordID.
  OemText = 11519,     ///< Legacy OemText.
  Date = 11775,        ///< AL Date.
  Time = 11776,        ///< AL Time.
  DateFormula = 11797, ///< AL DateFormula.
  Decimal = 12799,     ///< AL Decimal.
  Media = 26207,       ///< AL Media.
  MediaSet = 26208,    ///< AL MediaSet.
  Text = 31488,        ///< AL Text.
  Code = 31489,        ///< AL Code; distinct from FieldRef.Type().
  Binary = 33791,      ///< Legacy Binary.
  Blob = 33793,        ///< AL BLOB.
  Boolean = 34047,     ///< AL Boolean.
  Integer = 34559,     ///< AL Integer.
  OemCode = 35071,     ///< Legacy OemCode.
  Option = 35583,      ///< AL Option and enum fields.
  BigInteger = 36095,  ///< AL BigInteger.
  Duration = 36863,    ///< AL Duration.
  Guid = 37119,        ///< AL GUID.
  DateTime = 37375,    ///< AL DateTime.
};

/// \brief The `SQLDataType` vocabulary declared by System `Field.Table.al`.
enum class FieldSQLDataType : std::int32_t {
  Varchar = 0,    ///< Default Code storage property.
  Integer = 1,    ///< Numeric Code storage property.
  Variant = 2,    ///< Mixed Code storage property.
  BigInteger = 3, ///< Large numeric Code storage property.
};

/// \brief The compile-time `Field.Access` vocabulary, not a runtime permission grant.
enum class FieldAccess : std::int32_t {
  Public = 0,    ///< Available to other apps.
  Internal = 1,  ///< Available within the declaring app.
  Protected = 2, ///< Available through derived declarations.
  Local = 3,     ///< Available within the declaring object.
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
/// This preserves the current metadata-tag compatibility vocabulary. It is not the native
/// Field.Type mapping, which has its own compact coded vocabulary below.
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

/// \brief Names for the current metadata-tag compatibility vocabulary, not native ordinals.
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

/// \brief Compatibility vocabulary for the current internal `FieldType` metadata tags.
template <> struct agiru::OptionTraits<agiru::FieldType> {
  /// \brief Every ordinal from 0 to 40, named where AL names one.
  static constexpr auto kValues = agiru::detail::Sparse<41>(agiru::detail::kFieldTypeNames);
};

/// \brief The telemetry classifier vocabulary; not the native Field option order.
template <> struct agiru::OptionTraits<agiru::DataClassification> {
  /// \brief The seven classifications, dense.
  static constexpr std::array<agiru::EnumValueDef, 7> kValues{{
      {.ordinal = 0, .name = "ToBeClassified", .caption = "ToBeClassified"},
      {.ordinal = 1, .name = "SystemMetadata", .caption = "SystemMetadata"},
      {.ordinal = 2,
       .name = "EndUserIdentifiableInformation",
       .caption = "EndUserIdentifiableInformation"},
      {.ordinal = 3, .name = "AccountData", .caption = "AccountData"},
      {.ordinal = 4,
       .name = "EndUserPseudonymousIdentifiers",
       .caption = "EndUserPseudonymousIdentifiers"},
      {.ordinal = 5, .name = "CustomerContent", .caption = "CustomerContent"},
      {.ordinal = 6,
       .name = "OrganizationIdentifiableInformation",
       .caption = "OrganizationIdentifiableInformation"},
  }};
};

template <> struct agiru::OptionTraits<agiru::platform::FieldClass> {
  /// \brief The three classes, which are dense already.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "Normal", .caption = "Normal"},
      {.ordinal = 1, .name = "FlowField", .caption = "FlowField"},
      {.ordinal = 2, .name = "FlowFilter", .caption = "FlowFilter"},
  }};
};

/// \brief The compact, sorted native `Field.Type` vocabulary from System symbols.
template <> struct agiru::OptionTraits<agiru::platform::FieldDataType> {
  /// \brief Native codes are explicit rather than ordinary zero-based option positions.
  static constexpr bool kCodedOrdinals = true;
  /// \brief All twenty-one source members; no padding for the ordinal gaps.
  static constexpr std::array<agiru::EnumValueDef, 21> kValues{{
      {.ordinal = 4912, .name = "TableFilter", .caption = "TableFilter"},
      {.ordinal = 4988, .name = "RecordID", .caption = "RecordID"},
      {.ordinal = 11519, .name = "OemText", .caption = "OemText"},
      {.ordinal = 11775, .name = "Date", .caption = "Date"},
      {.ordinal = 11776, .name = "Time", .caption = "Time"},
      {.ordinal = 11797, .name = "DateFormula", .caption = "DateFormula"},
      {.ordinal = 12799, .name = "Decimal", .caption = "Decimal"},
      {.ordinal = 26207, .name = "Media", .caption = "Media"},
      {.ordinal = 26208, .name = "MediaSet", .caption = "MediaSet"},
      {.ordinal = 31488, .name = "Text", .caption = "Text"},
      {.ordinal = 31489, .name = "Code", .caption = "Code"},
      {.ordinal = 33791, .name = "Binary", .caption = "Binary"},
      {.ordinal = 33793, .name = "BLOB", .caption = "BLOB"},
      {.ordinal = 34047, .name = "Boolean", .caption = "Boolean"},
      {.ordinal = 34559, .name = "Integer", .caption = "Integer"},
      {.ordinal = 35071, .name = "OemCode", .caption = "OemCode"},
      {.ordinal = 35583, .name = "Option", .caption = "Option"},
      {.ordinal = 36095, .name = "BigInteger", .caption = "BigInteger"},
      {.ordinal = 36863, .name = "Duration", .caption = "Duration"},
      {.ordinal = 37119, .name = "GUID", .caption = "GUID"},
      {.ordinal = 37375, .name = "DateTime", .caption = "DateTime"},
  }};
};

/// \brief Native `Field.SQLDataType` members in source order.
template <> struct agiru::OptionTraits<agiru::platform::FieldSQLDataType> {
  /// \brief The four declared storage properties.
  static constexpr std::array<agiru::EnumValueDef, 4> kValues{{
      {.ordinal = 0, .name = "Varchar", .caption = "Varchar"},
      {.ordinal = 1, .name = "Integer", .caption = "Integer"},
      {.ordinal = 2, .name = "Variant", .caption = "Variant"},
      {.ordinal = 3, .name = "BigInteger", .caption = "BigInteger"},
  }};
};

/// \brief Native `Field.Access` members in source order.
template <> struct agiru::OptionTraits<agiru::platform::FieldAccess> {
  /// \brief The four declared compile-time access levels.
  static constexpr std::array<agiru::EnumValueDef, 4> kValues{{
      {.ordinal = 0, .name = "Public", .caption = "Public"},
      {.ordinal = 1, .name = "Internal", .caption = "Internal"},
      {.ordinal = 2, .name = "Protected", .caption = "Protected"},
      {.ordinal = 3, .name = "Local", .caption = "Local"},
  }};
};

namespace agiru::platform {

/// \brief The AL virtual table `Field` -- one row per field of every table in the catalogue.
///
/// \note System symbols declare the table; the platform supplies its rows. Its partial runtime
///       binding lives here rather than in generated application sources (board:0034).
///
/// \note The declaration follows all twenty-four fields in System `Field.Table.al`.
///       Temporary storage is distinct from the incomplete production metadata provider.
/// \warning `Field.Type`, `FieldRef.Type()` and internal `FieldType` are separate boundaries.
///          Package provenance and several metadata properties remain open (board:0034).
class Field : public Table<Field> {
public:
  /// \brief The AL table number of the virtual `Field` table.
  static constexpr TableId kId{2000000041};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Field"};
  /// \brief Source-declared extension availability, not a cloud integration requirement.
  static constexpr std::string_view kScope{"Cloud"};

  detail::StateHandle State_Block;

  /// \brief The declared lengths, which are AL's and not this file's.
  static constexpr std::size_t kNameLength = 30;
  /// \brief The declared length of `FieldCaption`.
  static constexpr std::size_t kCaptionLength = 80;
  /// \brief The declared length of `ObsoleteReason`.
  static constexpr std::size_t kReasonLength = 248;
  /// \brief The declared length of `OptionString`.
  static constexpr std::size_t kOptionStringLength = 2047;
  /// \brief The System-symbol length of `ExternalName`.
  static constexpr std::size_t kExternalNameLength = 100;

  /// \brief AL `Field."TableNo"`.
  ::agiru::Integer TableNo{};
  /// \brief AL `Field."No."`.
  ::agiru::Integer No{};
  /// \brief AL `Field."TableName"`.
  Text<kNameLength> TableName;
  /// \brief AL `Field."FieldName"`.
  Text<kNameLength> FieldName;
  /// \brief AL `Field."Type"`.
  Option<FieldDataType> Type;
  /// \brief AL `Field."Len"`.
  ::agiru::Integer Len{};
  /// \brief AL `Field."Class"`.
  Option<FieldClass> Class;
  /// \brief AL `Field."Type Name"`, declared as Text[30] in System symbols.
  ///
  /// Native metadata renders the primitive type, appending Code/Text length without a separator
  /// (BC 28.4 FieldDataProvider.GetFieldTypeName). Typed Get and provisioning share the mapper;
  /// native virtual-table navigation remains incomplete (board:0044).
  Text<kNameLength> TypeName;
  /// \brief AL `Field."RelationTableNo"`.
  ::agiru::Integer RelationTableNo{};
  /// \brief AL `Field."RelationFieldNo"`.
  ::agiru::Integer RelationFieldNo{};
  /// \brief AL `Field.SQLDataType`; not the PostgreSQL column type.
  Option<FieldSQLDataType> SQLDataType;
  /// \brief AL `Field."OptionString"`.
  Text<kOptionStringLength> OptionString;
  /// \brief AL `Field."ObsoleteState"`.
  Option<::agiru::platform::ObsoleteState> ObsoleteState;
  /// \brief AL `Field."ObsoleteReason"`.
  Text<kReasonLength> ObsoleteReason;
  /// \brief AL `Field."Field Caption"`.
  Text<kCaptionLength> FieldCaption;
  /// \brief AL `Field."Enabled"`.
  Boolean Enabled{};
  /// \brief AL `Field.IsPartOfPrimaryKey`, which the declaration spells without spaces.
  Boolean IsPartOfPrimaryKey{};
  /// \brief AL `Field.DataClassification`, with its own source-declared option order.
  Option<FieldDataClassification> DataClassification;
  /// \brief AL `Field.ExternalName`, populated from the retained external-name property.
  Text<kExternalNameLength> ExternalName;
  /// \brief AL `Field."App Package ID"`; production package provenance is not yet supplied.
  Guid AppPackageID;
  /// \brief AL `Field."App Runtime Package ID"`; not an inferred C++ app identifier.
  Guid AppRuntimePackageID;
  /// \brief AL `Field.OptimizeForTextSearch`.
  Boolean OptimizeForTextSearch{};
  /// \brief AL `Field.Access`; not a runtime security boundary.
  Option<FieldAccess> Access;
  /// \brief AL `Field.IsAllowedInCustomizations`; separate from page editability.
  Boolean IsAllowedInCustomizations{};

  /// \brief AL `Field.SystemId` -- blank, because a virtual table has no row to carry one.
  Guid SystemId;
  /// \brief AL `Field.SystemCreatedAt` -- blank, for the same reason.
  DateTime SystemCreatedAt;
  /// \brief AL `Field.SystemCreatedBy` -- blank.
  Guid SystemCreatedBy;
  /// \brief AL `Field.SystemModifiedAt` -- blank.
  DateTime SystemModifiedAt;
  /// \brief AL `Field.SystemModifiedBy` -- blank.
  Guid SystemModifiedBy;

  /// \brief Implicit read-only SQL rowversion buffer; provider ownership is separate.
  BigInteger SystemRowVersion{};
  /// \brief Current creator User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemCreatedByUserName{};
  /// \brief Current creator full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemCreatedByFullName{};
  /// \brief Current modifier User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemModifiedByUserName{};
  /// \brief Current modifier full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemModifiedByFullName{};

  /// \note THE SYSTEM FIELDS ARE DECLARED AND NEVER FILLED. A virtual table is not stored
  ///       (`devenv-virtual-tables.md`: "computed at runtime"), so there is no row to carry a
  ///       SystemId or an audit stamp -- but AL reaches them anyway: `Config. Package Management`
  ///       writes `Field.FieldNo(SystemId)` to filter the system fields OUT of a table's field
  ///       list, and `FieldNo` needs the member to name. So they exist, at the platform's numbers,
  ///       and read as blank.
  struct Field_No : SystemFieldNumbers {
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
    /// \brief The System-symbol field number of `Type Name`.
    static constexpr ::agiru::FieldNo TypeName{9};
    /// \brief The AL field number of `RelationTableNo`.
    static constexpr ::agiru::FieldNo RelationTableNo{21};
    /// \brief The AL field number of `RelationFieldNo`.
    static constexpr ::agiru::FieldNo RelationFieldNo{22};
    /// \brief The AL field number of `OptionString`.
    static constexpr ::agiru::FieldNo OptionString{24};
    /// \brief The AL field number of `ObsoleteState`.
    static constexpr ::agiru::FieldNo ObsoleteState{25};
    /// \brief The AL field number of `ObsoleteReason`.
    static constexpr ::agiru::FieldNo ObsoleteReason{26};
    /// \brief The AL field number of `Field Caption`.
    static constexpr ::agiru::FieldNo FieldCaption{20};
    /// \brief The AL field number of `Enabled`.
    static constexpr ::agiru::FieldNo Enabled{8};
    /// \brief The AL field number of `IsPartOfPrimaryKey`.
    static constexpr ::agiru::FieldNo IsPartOfPrimaryKey{28};
    /// \brief The System-symbol field number of `DataClassification`.
    static constexpr ::agiru::FieldNo DataClassification{27};
    /// \brief The System-symbol field number of `ExternalName`.
    static constexpr ::agiru::FieldNo ExternalName{10};
    /// \brief The System-symbol field number of `SQLDataType`.
    static constexpr ::agiru::FieldNo SQLDataType{23};
    /// \brief The System-symbol field number of `App Package ID`.
    static constexpr ::agiru::FieldNo AppPackageID{60};
    /// \brief The System-symbol field number of `App Runtime Package ID`.
    static constexpr ::agiru::FieldNo AppRuntimePackageID{61};
    /// \brief The System-symbol field number of `OptimizeForTextSearch`.
    static constexpr ::agiru::FieldNo OptimizeForTextSearch{62};
    /// \brief The System-symbol field number of `Access`.
    static constexpr ::agiru::FieldNo Access{63};
    /// \brief The System-symbol field number of `IsAllowedInCustomizations`.
    static constexpr ::agiru::FieldNo IsAllowedInCustomizations{64};
  };

  /// \brief The primary key: the table and the field within it.
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{{Field_No::TableNo, Field_No::No}};

  /// \brief AL `Field.Get(TableNo, No)` -- the field of a table, by number.
  ///
  /// \param TableNo The table, defaulting to zero when omitted.
  /// \param No      The field within it, defaulting to zero when omitted.
  /// \return True when this installation carries that field. Consumed missing reads
  ///         return false; discarded missing reads raise `DB:RecordNotFound`.
  /// \throws Error if a discarded read misses or a declaration cannot be projected.
  ///
  /// \note IT READS THE CATALOGUE AND NOT THE DATABASE, which is what makes `Field` VIRTUAL.
  ///       There is no `Field` table in PostgreSQL and there must not be: every row it could hold
  ///       is already `constexpr` data in `.rodata`, emitted beside the table it describes. Asking
  ///       SQL for it produced `relation "Field" does not exist`, which is the right error for the
  ///       wrong question.
  ///
  /// \warning IT HIDES `Table<Field>::Get` RATHER THAN OVERRIDING IT, because `Table` is CRTP and
  ///          has no virtuals -- that is the point of it. A `Field` reached through a `RecordRef`
  ///          therefore retains the live-provider guard until common navigation is qualified.
  detail::Found Get(::agiru::Integer TableNo = 0, ::agiru::Integer No = 0);
};

/// \brief The field table of the virtual `Field` table, as static const data.
inline constexpr auto kFieldFields = WithImplicitFields<Field,
                                                        ::agiru::SystemFieldProfile::Runtime18,
                                                        ::agiru::TableType::Normal,
                                                        false>(std::array<FieldDef, 24>{{
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
    Declare<&Field::Enabled>(
        Field::Field_No::Enabled, "Enabled", "Enabled", offsetof(Field, Enabled)),
    Declare<&Field::TypeName>(
        Field::Field_No::TypeName, "Type Name", "Type Name", offsetof(Field, TypeName)),
    Declare<&Field::ExternalName>(Field::Field_No::ExternalName,
                                  "ExternalName",
                                  "ExternalName",
                                  offsetof(Field, ExternalName)),
    Declare<&Field::FieldCaption>(Field::Field_No::FieldCaption,
                                  "Field Caption",
                                  "Field Caption",
                                  offsetof(Field, FieldCaption)),
    Declare<&Field::RelationTableNo>(Field::Field_No::RelationTableNo,
                                     "RelationTableNo",
                                     "RelationTableNo",
                                     offsetof(Field, RelationTableNo)),
    Declare<&Field::RelationFieldNo>(Field::Field_No::RelationFieldNo,
                                     "RelationFieldNo",
                                     "RelationFieldNo",
                                     offsetof(Field, RelationFieldNo)),
    Declare<&Field::SQLDataType>(
        Field::Field_No::SQLDataType, "SQLDataType", "SQLDataType", offsetof(Field, SQLDataType)),
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
    Declare<&Field::DataClassification>(Field::Field_No::DataClassification,
                                        "DataClassification",
                                        "DataClassification",
                                        offsetof(Field, DataClassification)),
    Declare<&Field::IsPartOfPrimaryKey>(Field::Field_No::IsPartOfPrimaryKey,
                                        "IsPartOfPrimaryKey",
                                        "IsPartOfPrimaryKey",
                                        offsetof(Field, IsPartOfPrimaryKey)),
    Declare<&Field::AppPackageID>(Field::Field_No::AppPackageID,
                                  "App Package ID",
                                  "App Package ID",
                                  offsetof(Field, AppPackageID)),
    Declare<&Field::AppRuntimePackageID>(Field::Field_No::AppRuntimePackageID,
                                         "App Runtime Package ID",
                                         "App Runtime Package ID",
                                         offsetof(Field, AppRuntimePackageID)),
    Declare<&Field::OptimizeForTextSearch>(Field::Field_No::OptimizeForTextSearch,
                                           "OptimizeForTextSearch",
                                           "OptimizeForTextSearch",
                                           offsetof(Field, OptimizeForTextSearch)),
    Declare<&Field::Access>(Field::Field_No::Access, "Access", "Access", offsetof(Field, Access)),
    Declare<&Field::IsAllowedInCustomizations>(Field::Field_No::IsAllowedInCustomizations,
                                               "IsAllowedInCustomizations",
                                               "IsAllowedInCustomizations",
                                               offsetof(Field, IsAllowedInCustomizations)),
}});

/// \brief The keys of the virtual `Field` table.
inline constexpr std::array<KeyDef, 1> kFieldKeys{{
    KeyDef{.name = "pk", .fields = Field::kKey1, .clustered = true},
}};

/// \brief The declaration of the virtual `Field` table.
inline constexpr TableDef kFieldTable{.id = Field::kId,
                                      .name = Field::kName,
                                      .caption = Field::kName,
                                      .fields = kFieldFields,
                                      .keys = kFieldKeys,
                                      .inherentPermissions = "rX"};

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
