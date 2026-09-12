// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#include "ResourceCost.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"

namespace agiru::app::tables {

constexpr auto kResourceCostFields = WithSystemFields<ResourceCost>(std::array<FieldDef, 6>{{
    Declare<&ResourceCost::Type>(
        ResourceCost::Field_No::Type, "Type", "Type", offsetof(ResourceCost, Type)),
    Declare<&ResourceCost::Code>(
        ResourceCost::Field_No::Code,
        "Code",
        "Code",
        offsetof(ResourceCost, Code),
        Declared{.relation = "if (Type = const(Resource)) Resource else if (Type = "
                             "const(\"Group(Resource)\")) \"Resource Group\""}),
    Declare<&ResourceCost::WorkTypeCode>(
        ResourceCost::Field_No::WorkTypeCode,
        "Work Type Code",
        "Work Type Code",
        offsetof(ResourceCost, WorkTypeCode),
        Declared{.relationTable = "Work Type", .relation = "Work Type"}),
    Declare<&ResourceCost::CostType>(ResourceCost::Field_No::CostType,
                                     "Cost Type",
                                     "Cost Type",
                                     offsetof(ResourceCost, CostType)),
    Declare<&ResourceCost::DirectUnitCost>(ResourceCost::Field_No::DirectUnitCost,
                                           "Direct Unit Cost",
                                           "Direct Unit Cost",
                                           offsetof(ResourceCost, DirectUnitCost),
                                           Declared{.autoFormatType = "2"}),
    Declare<&ResourceCost::UnitCost>(ResourceCost::Field_No::UnitCost,
                                     "Unit Cost",
                                     "Unit Cost",
                                     offsetof(ResourceCost, UnitCost),
                                     Declared{.autoFormatType = "2"}),
}});

constexpr std::array<KeyDef, 2> kResourceCostKeys{{
    KeyDef{.name = "Key1", .fields = ResourceCost::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = ResourceCost::kKey2, .clustered = false},
}};

constexpr TableDef kResourceCostTable{
    .id = ResourceCost::kId,
    .name = ResourceCost::kName,
    .caption = ResourceCost::kName,
    .fields = kResourceCostFields,
    .keys = kResourceCostKeys,
};

static_assert(FieldsAreSorted(kResourceCostTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::app::tables::ResourceCost, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<ResourceCost>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kResourceCostFields.size() == 6 + kSystemFieldCount,
              "table 202 declares 6 fields, and the platform adds its own");

static_assert(kResourceCostKeys.size() <= ::agiru::kMaximumKeys,
              "a table declares at most 40 keys (devenv-table-keys.md)");
static_assert(ResourceCost::kKey1.size() <= ::agiru::kMaximumPrimaryKeyFields,
              "a primary key names at most 16 fields (devenv-table-keys.md)");
static_assert(!kResourceCostKeys.empty(), "keys[0] IS the primary key, so a table has one");

} // namespace agiru::app::tables
