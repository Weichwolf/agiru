// THE TARGET IMAGE. This file is written BY HAND and is the specification the generator has to
// reproduce -- byte for byte, from Projects/Resources/Pricing/ResourceCost.Table.al. It is not
// generated output and it does not live under src/app/; it is what src/app/ shall look like.
//
// Writing the target before the generator is deliberate. A generator developed against a vague idea
// emits code nobody ever designed, and its correctness has nothing to compare against. With this
// file in place the proof is a file comparison and the negative control is a changed `.al` that has
// to change the output correspondingly (board:0012).
//
// AL source, table 202 "Resource Cost": six fields, two keys, a conditional TableRelation, two
// OnValidate triggers, one Label. Everything below has a line in that file behind it.

#pragma once

#include "agiru/Decimal.h"
#include "agiru/Ids.h"
#include "agiru/Option.h"
#include "agiru/TableDef.h"
#include "agiru/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace agiru::app {

/// field(1; Type; Option) -- OptionMembers = Resource,"Group(Resource)",All;
///
/// The AL member `"Group(Resource)"` cannot be spelled as a C++ identifier, so the enumerator is
/// renamed and the name table below keeps what AL wrote. An error message quotes the AL spelling,
/// never ours.
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

template <> struct agiru::OptionTraits<agiru::app::ResourceCostType> {
  static constexpr std::array<std::string_view, 3> kMembers{"Resource", "Group(Resource)", "All"};
  static constexpr std::array<std::string_view, 3> kCaptions{"Resource", "Group(Resource)", "All"};
};

template <> struct agiru::OptionTraits<agiru::app::ResourceCostCostType> {
  static constexpr std::array<std::string_view, 3> kMembers{"Fixed", "% Extra", "LCY Extra"};
  static constexpr std::array<std::string_view, 3> kCaptions{"Fixed", "% Extra", "LCY Extra"};
};

namespace agiru::app {

/// table 202 "Resource Cost"
///
/// THE FIELDS ARE PUBLIC DATA, AND THAT IS THE FAITHFUL SHAPE. In AL, `Rec."Unit Cost" := 5` and
/// `Rec.Validate("Unit Cost", 5)` are two different operations: the first assigns, the second runs
/// OnValidate. Modelling a field as a private member behind an accessor would collapse them, and
/// modelling it as a proxy object would put a back pointer in every field -- 183 of them in
/// `Sales Header` -- which the 512 MB target cannot afford. So: plain members for assignment,
/// `Validate` for validation, exactly as AL splits them.
///
/// Type names are fully qualified on purpose. The AL field named `Code` becomes a member named
/// `Code`, which would otherwise shadow the type `agiru::Code` inside this class.
class ResourceCost {
public:
  static constexpr TableId kId{202};
  static constexpr std::string_view kName{"Resource Cost"};

  ::agiru::Option<ResourceCostType> Type;         ///< field(1; Type; Option)
  ::agiru::Code<20> Code;                         ///< field(2; "Code"; Code[20])
  ::agiru::Code<10> WorkTypeCode;                 ///< field(3; "Work Type Code"; Code[10])
  ::agiru::Option<ResourceCostCostType> CostType; ///< field(4; "Cost Type"; Option)
  ::agiru::Decimal DirectUnitCost;                ///< field(5; "Direct Unit Cost"; Decimal)
  ::agiru::Decimal UnitCost;                      ///< field(6; "Unit Cost"; Decimal)

  /// trigger OnValidate() on field(2; "Code")
  void OnValidateCode();

  /// trigger OnValidate() on field(4; "Cost Type")
  void OnValidateCostType();

  /// AL `Rec.FieldNo(<field name>)`. The generator emits one constant per field, because AL code
  /// addresses fields by number and writing the number at the call site is how a renumbered field
  /// becomes a silent bug rather than a compile error.
  struct FieldNumber {
    static constexpr ::agiru::FieldNo Type{1};
    static constexpr ::agiru::FieldNo Code{2};
    static constexpr ::agiru::FieldNo WorkTypeCode{3};
    static constexpr ::agiru::FieldNo CostType{4};
    static constexpr ::agiru::FieldNo DirectUnitCost{5};
    static constexpr ::agiru::FieldNo UnitCost{6};
  };

  /// Text000: Label 'cannot be specified when %1 is %2';
  static constexpr std::string_view kText000{"cannot be specified when %1 is %2"};
};

/// key(Key1; Type, "Code", "Work Type Code") { Clustered = true; }
inline constexpr std::array<FieldNo, 3> kResourceCostKey1{FieldNo{1}, FieldNo{2}, FieldNo{3}};

/// key(Key2; "Cost Type", "Code", "Work Type Code")
inline constexpr std::array<FieldNo, 3> kResourceCostKey2{FieldNo{4}, FieldNo{2}, FieldNo{3}};

inline constexpr std::array<KeyDef, 2> kResourceCostKeys{
    KeyDef{.name = "Key1", .fields = kResourceCostKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = kResourceCostKey2, .clustered = false},
};

inline constexpr std::array<FieldDef, 6> kResourceCostFields{
    FieldDef{.no = FieldNo{1},
             .name = "Type",
             .caption = "Type",
             .type = FieldType::Option,
             .length = 0,
             .offset = offsetof(ResourceCost, Type)},
    FieldDef{.no = FieldNo{2},
             .name = "Code",
             .caption = "Code",
             .type = FieldType::Code,
             .length = 20,
             .offset = offsetof(ResourceCost, Code)},
    FieldDef{.no = FieldNo{3},
             .name = "Work Type Code",
             .caption = "Work Type Code",
             .type = FieldType::Code,
             .length = 10,
             .offset = offsetof(ResourceCost, WorkTypeCode)},
    FieldDef{.no = FieldNo{4},
             .name = "Cost Type",
             .caption = "Cost Type",
             .type = FieldType::Option,
             .length = 0,
             .offset = offsetof(ResourceCost, CostType)},
    FieldDef{.no = FieldNo{5},
             .name = "Direct Unit Cost",
             .caption = "Direct Unit Cost",
             .type = FieldType::Decimal,
             .length = 0,
             .offset = offsetof(ResourceCost, DirectUnitCost)},
    FieldDef{.no = FieldNo{6},
             .name = "Unit Cost",
             .caption = "Unit Cost",
             .type = FieldType::Decimal,
             .length = 0,
             .offset = offsetof(ResourceCost, UnitCost)},
};

inline constexpr TableDef kResourceCostTable{
    .id = ResourceCost::kId,
    .name = ResourceCost::kName,
    .caption = "Resource Cost",
    .fields = kResourceCostFields,
    .keys = kResourceCostKeys,
};

// WHAT THE COMPILER CAN DECIDE IS A static_assert AND NEVER A CASE. The generator emits these
// beside every table it writes, so a mis-generated field table is a TRANSLATION error rather than a
// runtime surprise -- which is the whole reason this project left Python.
static_assert(kResourceCostFields.size() == 6, "table 202 declares six fields");
static_assert(kResourceCostFields[0].no == ResourceCost::FieldNumber::Type,
              "field numbers are emitted in order");
static_assert(Field(kResourceCostTable, ResourceCost::FieldNumber::UnitCost) != nullptr,
              "every declared field number resolves, checked at compile time");
static_assert(kResourceCostKeys[0].clustered, "Key1 is the clustered primary key");
static_assert(kResourceCostFields[1].length == 20, "field 2 is Code[20]");
static_assert(std::is_standard_layout_v<ResourceCost>,
              "offsetof over the field table requires standard layout, which is why the record has "
              "no base class and no private data beside its fields");

} // namespace agiru::app
