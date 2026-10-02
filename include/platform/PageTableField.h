#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief System Page Table Field (2000000171), declaration only; live projection is unavailable.
/// \see System symbols src/Virtual Tables/PageTableField.Table.al.

namespace agiru::platform {

/// \brief Original PageTableFieldType vocabulary from the System declaration.
enum class PageTableFieldType : std::int32_t {
  TableFilter = 4912,          ///< Original TableFilter value.
  RecordId = 4988,             ///< Original RecordID value.
  OemText = 11519,             ///< Original OemText value.
  Date = 11775,                ///< Original Date value.
  Time = 11776,                ///< Original Time value.
  DateFormula = 11797,         ///< Original DateFormula value.
  Decimal = 12799,             ///< Original Decimal value.
  Media = 26207,               ///< Original Media value.
  MediaSet = 26208,            ///< Original MediaSet value.
  Text = 31488,                ///< Original Text value.
  Code = 31489,                ///< Original Code value.
  NotSupported_Binary = 33791, ///< Original NotSupported_Binary value.
  Blob = 33793,                ///< Original BLOB value.
  Boolean = 34047,             ///< Original Boolean value.
  Integer = 34559,             ///< Original Integer value.
  OemCode = 35071,             ///< Original OemCode value.
  Option = 35583,              ///< Original Option value.
  BigInteger = 36095,          ///< Original BigInteger value.
  Duration = 36863,            ///< Original Duration value.
  Guid = 37119,                ///< Original GUID value.
  DateTime = 37375,            ///< Original DateTime value.
};

/// \brief Original PageTableFieldStatus vocabulary from the System declaration.
enum class PageTableFieldStatus : std::int32_t {
  New = 0,    ///< Original New value.
  Ready = 1,  ///< Original Ready value.
  Placed = 2, ///< Original Placed value.
};

/// \brief Original PageTableFieldScope vocabulary from the System declaration.
enum class PageTableFieldScope : std::int32_t {
  TableFieldVisibleOnPage = 0,          ///< Original TableFieldVisibleOnPage value.
  TableFieldHiddenOnPage = 1,           ///< Original TableFieldHiddenOnPage value.
  TableFieldNotOnPage = 2,              ///< Original TableFieldNotOnPage value.
  TableExtensionFieldVisibleOnPage = 3, ///< Original TableExtensionFieldVisibleOnPage value.
  TableExtensionFieldHiddenOnPage = 4,  ///< Original TableExtensionFieldHiddenOnPage value.
  TableExtensionFieldNotOnPage = 5,     ///< Original TableExtensionFieldNotOnPage value.
  PageFieldVisible = 6,                 ///< Original PageFieldVisible value.
  PageFieldHidden = 7,                  ///< Original PageFieldHidden value.
  PageExtensionFieldVisible = 8,        ///< Original PageExtensionFieldVisible value.
  PageExtensionFieldHidden = 9,         ///< Original PageExtensionFieldHidden value.
};

/// \brief Original PageTableFieldKind vocabulary from the System declaration.
enum class PageTableFieldKind : std::int32_t {
  TableField = 0,                 ///< Original TableField value.
  PageFieldBoundToTable = 1,      ///< Original PageFieldBoundToTable value.
  PageFieldBoundToExpression = 2, ///< Original PageFieldBoundToExpression value.
};

}

/// \brief Source-declared PageTableFieldType names and values.
template <> struct agiru::OptionTraits<agiru::platform::PageTableFieldType> {
  /// \brief Preserve the explicit System native codes, not metadata FieldType tags.
  static constexpr bool kCodedOrdinals = true;
  /// \brief Members in declaration order.
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
      {.ordinal = 33791, .name = "NotSupported_Binary", .caption = "NotSupported_Binary"},
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

/// \brief Source-declared PageTableFieldStatus names and values.
template <> struct agiru::OptionTraits<agiru::platform::PageTableFieldStatus> {
  /// \brief Members in declaration order.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "New", .caption = "New"},
      {.ordinal = 1, .name = "Ready", .caption = "Ready"},
      {.ordinal = 2, .name = "Placed", .caption = "Placed"},
  }};
};

/// \brief Source-declared PageTableFieldScope names and values.
template <> struct agiru::OptionTraits<agiru::platform::PageTableFieldScope> {
  /// \brief Members in declaration order.
  static constexpr std::array<agiru::EnumValueDef, 10> kValues{{
      {.ordinal = 0, .name = "TableFieldVisibleOnPage", .caption = "TableFieldVisibleOnPage"},
      {.ordinal = 1, .name = "TableFieldHiddenOnPage", .caption = "TableFieldHiddenOnPage"},
      {.ordinal = 2, .name = "TableFieldNotOnPage", .caption = "TableFieldNotOnPage"},
      {.ordinal = 3,
       .name = "TableExtensionFieldVisibleOnPage",
       .caption = "TableExtensionFieldVisibleOnPage"},
      {.ordinal = 4,
       .name = "TableExtensionFieldHiddenOnPage",
       .caption = "TableExtensionFieldHiddenOnPage"},
      {.ordinal = 5,
       .name = "TableExtensionFieldNotOnPage",
       .caption = "TableExtensionFieldNotOnPage"},
      {.ordinal = 6, .name = "PageFieldVisible", .caption = "PageFieldVisible"},
      {.ordinal = 7, .name = "PageFieldHidden", .caption = "PageFieldHidden"},
      {.ordinal = 8, .name = "PageExtensionFieldVisible", .caption = "PageExtensionFieldVisible"},
      {.ordinal = 9, .name = "PageExtensionFieldHidden", .caption = "PageExtensionFieldHidden"},
  }};
};

/// \brief Source-declared PageTableFieldKind names and values.
template <> struct agiru::OptionTraits<agiru::platform::PageTableFieldKind> {
  /// \brief Members in declaration order.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "TableField", .caption = "TableField"},
      {.ordinal = 1, .name = "PageFieldBoundToTable", .caption = "PageFieldBoundToTable"},
      {.ordinal = 2, .name = "PageFieldBoundToExpression", .caption = "PageFieldBoundToExpression"},
  }};
};

namespace agiru::platform {

/// \brief Original tenant-wide tooling record. Typed values are not a live metadata provider.
class PageTableField_Table : public Table<PageTableField_Table> {
public:
  /// \brief Original System table ID.
  static constexpr TableId kId{2000000171};
  /// \brief Original AL name.
  static constexpr std::string_view kName{"Page Table Field"};
  /// \brief Original extension availability, not a cloud-service dependency.
  static constexpr std::string_view kScope{"OnPrem"};
  /// \brief Source Brick fieldgroup, not yet represented by TableDef.
  static constexpr std::array<::agiru::FieldNo, 4> kBrick{
      {::agiru::FieldNo{3}, ::agiru::FieldNo{5}, ::agiru::FieldNo{6}, ::agiru::FieldNo{8}}};
  /// \brief Record state at the required first offset.
  detail::StateHandle State_Block;
  /// \brief Original AL Page ID field.
  ::agiru::Integer PageID;
  /// \brief Original AL Index field.
  ::agiru::Integer Index;
  /// \brief Original AL Type field.
  Option<PageTableFieldType> Type;
  /// \brief Original AL Length field.
  ::agiru::Integer Length;
  /// \brief Original AL Caption field.
  Text<80> Caption;
  /// \brief Original AL Status field.
  Option<PageTableFieldStatus> Status;
  /// \brief Original AL IsTableField field.
  Boolean IsTableField;
  /// \brief Original AL Scope field.
  Option<PageTableFieldScope> Scope;
  /// \brief Original AL Tooltip field.
  Text<2048> Tooltip;
  /// \brief Original AL FieldKind field.
  Option<PageTableFieldKind> FieldKind;
  /// \brief Original AL Name field.
  Text<256> Name;
  /// \brief Original AL Field ID field.
  ::agiru::Integer FieldID;
  /// \brief Original AL Table No field.
  ::agiru::Integer TableNo;
  /// \brief Original AL Description field.
  Text<2048> Description;
  /// \brief Original AL Table Field Id field.
  ::agiru::Integer TableFieldId;
  /// \brief Common AL SystemId field.
  Guid SystemId;
  /// \brief Common AL SystemCreatedAt field.
  DateTime SystemCreatedAt;
  /// \brief Common AL SystemCreatedBy field.
  Guid SystemCreatedBy;
  /// \brief Common AL SystemModifiedAt field.
  DateTime SystemModifiedAt;
  /// \brief Common AL SystemModifiedBy field.
  Guid SystemModifiedBy;

  /// \brief Original field numbers.
  struct Field_No : SystemFieldNumbers {
    /// \brief AL Page ID.
    static constexpr ::agiru::FieldNo PageID{1};
    /// \brief AL Index.
    static constexpr ::agiru::FieldNo Index{2};
    /// \brief AL Type.
    static constexpr ::agiru::FieldNo Type{3};
    /// \brief AL Length.
    static constexpr ::agiru::FieldNo Length{4};
    /// \brief AL Caption.
    static constexpr ::agiru::FieldNo Caption{5};
    /// \brief AL Status.
    static constexpr ::agiru::FieldNo Status{6};
    /// \brief AL IsTableField.
    static constexpr ::agiru::FieldNo IsTableField{7};
    /// \brief AL Scope.
    static constexpr ::agiru::FieldNo Scope{8};
    /// \brief AL Tooltip.
    static constexpr ::agiru::FieldNo Tooltip{9};
    /// \brief AL FieldKind.
    static constexpr ::agiru::FieldNo FieldKind{10};
    /// \brief AL Name.
    static constexpr ::agiru::FieldNo Name{11};
    /// \brief AL Field ID.
    static constexpr ::agiru::FieldNo FieldID{12};
    /// \brief AL Table No.
    static constexpr ::agiru::FieldNo TableNo{13};
    /// \brief AL Description.
    static constexpr ::agiru::FieldNo Description{14};
    /// \brief AL Table Field Id.
    static constexpr ::agiru::FieldNo TableFieldId{15};
  };

  /// \brief Original pk field order.
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{{Field_No::PageID, Field_No::Index}};
};

/// \brief Source-owned native ABI alias.
using PageTableField = PageTableField_Table;

/// \brief All fifteen source fields, original obsolete properties and common system fields.
inline constexpr auto kPageTableFieldFields = WithSystemFields<PageTableField>([] {
  std::array<FieldDef, 15> fields{{
      Declare<&PageTableField::PageID>(
          PageTableField::Field_No::PageID, "Page ID", "Page ID", offsetof(PageTableField, PageID)),
      Declare<&PageTableField::Index>(
          PageTableField::Field_No::Index, "Index", "Index", offsetof(PageTableField, Index)),
      Declare<&PageTableField::Type>(
          PageTableField::Field_No::Type, "Type", "Type", offsetof(PageTableField, Type)),
      Declare<&PageTableField::Length>(
          PageTableField::Field_No::Length, "Length", "Length", offsetof(PageTableField, Length)),
      Declare<&PageTableField::Caption>(PageTableField::Field_No::Caption,
                                        "Caption",
                                        "Caption",
                                        offsetof(PageTableField, Caption)),
      Declare<&PageTableField::Status>(
          PageTableField::Field_No::Status, "Status", "Status", offsetof(PageTableField, Status)),
      Declare<&PageTableField::IsTableField>(PageTableField::Field_No::IsTableField,
                                             "IsTableField",
                                             "IsTableField",
                                             offsetof(PageTableField, IsTableField)),
      Declare<&PageTableField::Scope>(
          PageTableField::Field_No::Scope, "Scope", "Scope", offsetof(PageTableField, Scope)),
      Declare<&PageTableField::Tooltip>(PageTableField::Field_No::Tooltip,
                                        "Tooltip",
                                        "Tooltip",
                                        offsetof(PageTableField, Tooltip)),
      Declare<&PageTableField::FieldKind>(PageTableField::Field_No::FieldKind,
                                          "FieldKind",
                                          "FieldKind",
                                          offsetof(PageTableField, FieldKind)),
      Declare<&PageTableField::Name>(
          PageTableField::Field_No::Name, "Name", "Name", offsetof(PageTableField, Name)),
      Declare<&PageTableField::FieldID>(PageTableField::Field_No::FieldID,
                                        "Field ID",
                                        "Field ID",
                                        offsetof(PageTableField, FieldID)),
      Declare<&PageTableField::TableNo>(PageTableField::Field_No::TableNo,
                                        "Table No",
                                        "Table No",
                                        offsetof(PageTableField, TableNo)),
      Declare<&PageTableField::Description>(PageTableField::Field_No::Description,
                                            "Description",
                                            "Description",
                                            offsetof(PageTableField, Description)),
      Declare<&PageTableField::TableFieldId>(PageTableField::Field_No::TableFieldId,
                                             "Table Field Id",
                                             "Table Field Id",
                                             offsetof(PageTableField, TableFieldId)),
  }};
  fields[2].optionOrdinalValues =
      "4912, 4988, 11519, 11775, 11776, 11797, 12799, 26207, 26208, 31488, 31489, 33791, 33793, "
      "34047, 34559, 35071, 35583, 36095, 36863, 37119, 37375";
  fields[5].obsoleteState = "Pending";
  fields[5].obsoleteReason =
      "This is being removed in favor of the Scope field which provides more granular information.";
  fields[6].obsoleteState = "Pending";
  fields[6].obsoleteReason = "This is being removed in favor of the FieldKind field which provides "
                             "more granular information.";
  return fields;
}());
/// \brief Original primary key, clustered by the shared AL default.
inline constexpr std::array<KeyDef, 1> kPageTableFieldKeys{{
    KeyDef{.name = "pk", .fields = PageTableField::kKey1, .clustered = true},
}};
/// \brief Original represented declaration; no physical replacement for this virtual table.
inline constexpr TableDef kPageTableFieldTable{
    .id = PageTableField::kId,
    .name = PageTableField::kName,
    .caption = PageTableField::kName,
    .fields = kPageTableFieldFields,
    .keys = kPageTableFieldKeys,
    .dataPerCompany = false,
    .providerRefusal = "live page/table field projection is unavailable (board:0034/0044)",
};

static_assert(FieldsAreSorted(kPageTableFieldTable));
static_assert(offsetof(PageTableField, State_Block) == 0);

}

/// \brief Page Table Field's source-owned metadata.
template <> struct agiru::TableTraits<agiru::platform::PageTableField_Table> {
  /// \brief Original represented table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPageTableFieldTable;
};
