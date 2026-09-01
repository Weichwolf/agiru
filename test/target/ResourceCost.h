// THE TARGET IMAGE, declaration half. Written BY HAND as the C++ the generator must emit from
// Projects/Resources/Pricing/ResourceCost.Table.al. It is not generated output and it does not live
// under src/app/; it is what src/app/ shall look like.
//
// EVERY DECLARATION IS HERE. The .cpp carries the trigger bodies and nothing else -- what AL puts
// in a `field` or `key` block is a declaration, and what it puts in a `trigger` or `procedure` is
// code.
//
// EACH PROPERTY IS STATED ONCE. A field says its number, its AL name, its caption and its type; the
// type tag, the declared length and an option's member names are derived from the type by
// agiru::Declare. The identifier appears twice -- as the member and inside `offsetof` -- and no
// standard C++ removes that. It is a checksum rather than a duplication: a generator writes both
// from one AST node and the compiler holds them together.

#pragma once

#include "agiru/Decimal.h"
#include "agiru/Declare.h"
#include "agiru/Ids.h"
#include "agiru/Option.h"
#include "agiru/Table.h"
#include "agiru/TableDef.h"
#include "agiru/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::app {

/// field(1; Type; Option) -- OptionMembers = Resource,"Group(Resource)",All;
///
/// The AL member `"Group(Resource)"` cannot be spelled as a C++ identifier, so the enumerator is
/// renamed and the name table keeps what AL wrote. An error message quotes the AL spelling.
enum class ResourceCostType : std::int32_t {
  Resource = 0,
  GroupResource = 1,
  All = 2,
};

/// field(4; "Cost Type"; Option) -- OptionMembers = "Fixed","% Extra","LCY Extra";
enum class ResourceCostCostType : std::int32_t {
  Fixed = 0,
  PercentExtra = 1,
  LCYExtra = 2,
};

} // namespace agiru::app

/// OptionCaption = 'Resource,Group(Resource),All';
template <> struct agiru::OptionTraits<agiru::app::ResourceCostType> {
  static constexpr std::array<std::string_view, 3> kMembers{"Resource", "Group(Resource)", "All"};
  static constexpr std::array<std::string_view, 3> kCaptions{"Resource", "Group(Resource)", "All"};
};

/// OptionCaption = 'Fixed,% Extra,LCY Extra';
template <> struct agiru::OptionTraits<agiru::app::ResourceCostCostType> {
  static constexpr std::array<std::string_view, 3> kMembers{"Fixed", "% Extra", "LCY Extra"};
  static constexpr std::array<std::string_view, 3> kCaptions{"Fixed", "% Extra", "LCY Extra"};
};

namespace agiru::app {

/// table 202 "Resource Cost"
///
/// THE FIELDS ARE PUBLIC DATA, AND THAT IS THE FAITHFUL SHAPE. In AL, `Rec."Unit Cost" := 5` and
/// `Rec.Validate("Unit Cost", 5)` are two different operations: the first assigns, the second runs
/// OnValidate. A private member behind an accessor collapses them, and a proxy object per field
/// would put a back pointer in each of `Sales Header`'s 183 fields.
///
/// The base carries what AL's platform carries -- Insert, Modify, Delete, Get, FieldError,
/// TestField, FieldCaption -- so that none of it appears here.
class ResourceCost : public Table<ResourceCost> {
public:
  static constexpr TableId kId{202};
  static constexpr std::string_view kName{"Resource Cost"};

  /// fields
  /// {
  ///     field(1; Type;              Option)   { Caption = 'Type'; }
  ///     field(2; "Code";            Code[20]) { Caption = 'Code'; }
  ///     field(3; "Work Type Code";  Code[10]) { Caption = 'Work Type Code'; }
  ///     field(4; "Cost Type";       Option)   { Caption = 'Cost Type'; }
  ///     field(5; "Direct Unit Cost"; Decimal) { Caption = 'Direct Unit Cost'; }
  ///     field(6; "Unit Cost";       Decimal)  { Caption = 'Unit Cost'; }
  /// }
  Option<ResourceCostType> Type;
  ::agiru::Code<20> Code;
  ::agiru::Code<10> WorkTypeCode;
  Option<ResourceCostCostType> CostType;
  Decimal DirectUnitCost;
  Decimal UnitCost;

  /// AL `Rec.FieldNo(<field name>)`. AL code addresses a field by number, and writing the number at
  /// the call site is how a renumbered field becomes a silent bug rather than a compile error.
  struct FieldNumber {
    static constexpr FieldNo Type{1};
    static constexpr FieldNo Code{2};
    static constexpr FieldNo WorkTypeCode{3};
    static constexpr FieldNo CostType{4};
    static constexpr FieldNo DirectUnitCost{5};
    static constexpr FieldNo UnitCost{6};
  };

  /// keys
  /// {
  ///     key(Key1; Type, "Code", "Work Type Code") { Clustered = true; }
  ///     key(Key2; "Cost Type", "Code", "Work Type Code")
  /// }
  static constexpr std::array kKey1{
      FieldNumber::Type, FieldNumber::Code, FieldNumber::WorkTypeCode};
  static constexpr std::array kKey2{
      FieldNumber::CostType, FieldNumber::Code, FieldNumber::WorkTypeCode};

  /// AL: `Text000: Label 'cannot be specified when %1 is %2';`
  static constexpr std::string_view Text000{"cannot be specified when %1 is %2"};

  /// trigger OnValidate() on field(2; "Code")
  void OnValidateCode();

  /// trigger OnValidate() on field(4; "Cost Type")
  void OnValidateCostType();
};

inline constexpr std::array kResourceCostFields{
    Declare<&ResourceCost::Type>(
        ResourceCost::FieldNumber::Type, "Type", "Type", offsetof(ResourceCost, Type)),
    Declare<&ResourceCost::Code>(
        ResourceCost::FieldNumber::Code, "Code", "Code", offsetof(ResourceCost, Code)),
    Declare<&ResourceCost::WorkTypeCode>(ResourceCost::FieldNumber::WorkTypeCode,
                                         "Work Type Code",
                                         "Work Type Code",
                                         offsetof(ResourceCost, WorkTypeCode)),
    Declare<&ResourceCost::CostType>(ResourceCost::FieldNumber::CostType,
                                     "Cost Type",
                                     "Cost Type",
                                     offsetof(ResourceCost, CostType)),
    Declare<&ResourceCost::DirectUnitCost>(ResourceCost::FieldNumber::DirectUnitCost,
                                           "Direct Unit Cost",
                                           "Direct Unit Cost",
                                           offsetof(ResourceCost, DirectUnitCost)),
    Declare<&ResourceCost::UnitCost>(ResourceCost::FieldNumber::UnitCost,
                                     "Unit Cost",
                                     "Unit Cost",
                                     offsetof(ResourceCost, UnitCost)),
};

inline constexpr std::array kResourceCostKeys{
    KeyDef{.name = "Key1", .fields = ResourceCost::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = ResourceCost::kKey2, .clustered = false},
};

inline constexpr TableDef kResourceCostTable{
    .id = ResourceCost::kId,
    .name = ResourceCost::kName,
    .caption = ResourceCost::kName,
    .fields = kResourceCostFields,
    .keys = kResourceCostKeys,
};

// WHAT THE COMPILER CAN DECIDE IS A static_assert AND NEVER A CASE. A mis-sorted field table would
// be a lookup that quietly finds nothing, and a record that is not standard-layout would make every
// offset in the table meaningless. Both are translation errors instead.
static_assert(FieldsAreSorted(kResourceCostTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(std::is_standard_layout_v<ResourceCost>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kResourceCostFields.size() == 6, "table 202 declares six fields");

} // namespace agiru::app

/// The declaration this table stands on.
template <> struct agiru::TableTraits<agiru::app::ResourceCost> {
  static constexpr const TableDef &kTable = agiru::app::kResourceCostTable;
};
