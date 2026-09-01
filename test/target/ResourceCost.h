// THE TARGET IMAGE, declaration half. Written BY HAND as the C++ the generator must emit from
// Projects/Resources/Pricing/ResourceCost.Table.al. It is not generated output and it does not live
// under src/app/; it is what src/app/ shall look like.
//
// EVERYTHING IN THIS FILE HAS A LINE IN THE .al FILE BEHIND IT. The field table, the key arrays and
// the offsets are machinery, so they are in the .cpp -- a header that reads like the AL object is
// the whole point, and it is also what keeps a runtime change from recompiling every table.

#pragma once

#include "agiru/Decimal.h"
#include "agiru/Ids.h"
#include "agiru/Option.h"
#include "agiru/Table.h"
#include "agiru/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace agiru::app {

/// field(1; Type; Option) -- OptionMembers = Resource,"Group(Resource)",All;
///
/// The AL member `"Group(Resource)"` cannot be spelled as a C++ identifier, so the enumerator is
/// renamed and the name table below keeps what AL wrote. An error message quotes the AL spelling.
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

  /// The declared length of each string field, as AL wrote it.
  struct FieldLength {
    static constexpr std::size_t Code = 20;
    static constexpr std::size_t WorkTypeCode = 10;
  };

  ::agiru::Option<ResourceCostType> Type;                ///< field(1; Type; Option)
  ::agiru::Code<FieldLength::Code> Code;                 ///< field(2; "Code"; Code[20])
  ::agiru::Code<FieldLength::WorkTypeCode> WorkTypeCode; ///< field(3; "Work Type Code"; Code[10])
  ::agiru::Option<ResourceCostCostType> CostType;        ///< field(4; "Cost Type"; Option)
  ::agiru::Decimal DirectUnitCost;                       ///< field(5; "Direct Unit Cost"; Decimal)
  ::agiru::Decimal UnitCost;                             ///< field(6; "Unit Cost"; Decimal)

  /// AL: `Text000: Label 'cannot be specified when %1 is %2';`
  static constexpr std::string_view Text000{"cannot be specified when %1 is %2"};

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

  /// trigger OnValidate() on field(2; "Code")
  void OnValidateCode();

  /// trigger OnValidate() on field(4; "Cost Type")
  void OnValidateCostType();
};

} // namespace agiru::app

/// The declaration this table stands on. Declared here and DEFINED in the .cpp, so that the field
/// table with its offsets stays out of every translation unit that only wants the class.
template <> struct agiru::TableTraits<agiru::app::ResourceCost> {
  static const TableDef &kTable;
};
