// THE TARGET IMAGE, definition half. Written BY HAND as the C++ the generator must emit from
// Projects/Resources/Pricing/ResourceCost.Table.al. See ResourceCost.h for what this file is.
//
// Two things live here and nowhere else: the trigger bodies, which are AL, and the field and key
// tables, which are machinery. Keeping the machinery out of the header is what lets a runtime
// change recompile the runtime instead of all 9 300 tables.

#include "ResourceCost.h"

#include "agiru/Ids.h"
#include "agiru/Record.h"
#include "agiru/TableDef.h"

#include <array>
#include <cstddef>
#include <type_traits>

namespace agiru::app {

/// AL:
///     trigger OnValidate()
///     begin
///         if (Code <> '') and (Type = Type::All) then
///             FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
///     end;
void ResourceCost::OnValidateCode() {
  if (Code != "" && Type == ResourceCostType::All) {
    FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
  }
}

/// AL:
///     trigger OnValidate()
///     begin
///         if "Work Type Code" = '' then
///             TestField("Cost Type", "Cost Type"::Fixed);
///     end;
void ResourceCost::OnValidateCostType() {
  if (WorkTypeCode == "") { TestField(CostType, ResourceCostCostType::Fixed); }
}

namespace {

/// key(Key1; Type, "Code", "Work Type Code") { Clustered = true; }
constexpr std::array kKey1{ResourceCost::FieldNumber::Type,
                           ResourceCost::FieldNumber::Code,
                           ResourceCost::FieldNumber::WorkTypeCode};

/// key(Key2; "Cost Type", "Code", "Work Type Code")
constexpr std::array kKey2{ResourceCost::FieldNumber::CostType,
                           ResourceCost::FieldNumber::Code,
                           ResourceCost::FieldNumber::WorkTypeCode};

constexpr std::array kKeys{
    KeyDef{.name = "Key1", .fields = kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = kKey2, .clustered = false},
};

constexpr std::array kFields{
    FieldDef{.no = ResourceCost::FieldNumber::Type,
             .name = "Type",
             .caption = "Type",
             .type = FieldType::Option,
             .length = 0,
             .offset = offsetof(ResourceCost, Type),
             .members = OptionTraits<ResourceCostType>::kMembers},
    FieldDef{.no = ResourceCost::FieldNumber::Code,
             .name = "Code",
             .caption = "Code",
             .type = FieldType::Code,
             .length = ResourceCost::FieldLength::Code,
             .offset = offsetof(ResourceCost, Code),
             .members = {}},
    FieldDef{.no = ResourceCost::FieldNumber::WorkTypeCode,
             .name = "Work Type Code",
             .caption = "Work Type Code",
             .type = FieldType::Code,
             .length = ResourceCost::FieldLength::WorkTypeCode,
             .offset = offsetof(ResourceCost, WorkTypeCode),
             .members = {}},
    FieldDef{.no = ResourceCost::FieldNumber::CostType,
             .name = "Cost Type",
             .caption = "Cost Type",
             .type = FieldType::Option,
             .length = 0,
             .offset = offsetof(ResourceCost, CostType),
             .members = OptionTraits<ResourceCostCostType>::kMembers},
    FieldDef{.no = ResourceCost::FieldNumber::DirectUnitCost,
             .name = "Direct Unit Cost",
             .caption = "Direct Unit Cost",
             .type = FieldType::Decimal,
             .length = 0,
             .offset = offsetof(ResourceCost, DirectUnitCost),
             .members = {}},
    FieldDef{.no = ResourceCost::FieldNumber::UnitCost,
             .name = "Unit Cost",
             .caption = "Unit Cost",
             .type = FieldType::Decimal,
             .length = 0,
             .offset = offsetof(ResourceCost, UnitCost),
             .members = {}},
};

constexpr TableDef kTable{
    .id = ResourceCost::kId,
    .name = ResourceCost::kName,
    .caption = "Resource Cost",
    .fields = kFields,
    .keys = kKeys,
};

// WHAT THE COMPILER CAN DECIDE IS A static_assert AND NEVER A CASE. The generator emits these
// beside every table it writes, so a mis-generated field table is a TRANSLATION error rather than a
// runtime surprise -- which is the whole reason this project left Python.
constexpr std::size_t kDeclaredFieldCount = 6;
static_assert(kFields.size() == kDeclaredFieldCount, "table 202 declares six fields");
static_assert(FieldsAreSorted(kTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(kKeys[0].clustered, "Key1 is the clustered primary key");
static_assert(kFields[1].length == ResourceCost::FieldLength::Code, "field 2 is Code[20]");
static_assert(Field(kTable, ResourceCost::FieldNumber::UnitCost) != nullptr,
              "every declared field number resolves, checked at compile time");
static_assert(std::is_standard_layout_v<ResourceCost>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so -- an empty base does not break the rule and a base with "
              "a single member would");

} // namespace

} // namespace agiru::app

const agiru::TableDef &agiru::TableTraits<agiru::app::ResourceCost>::kTable = agiru::app::kTable;
