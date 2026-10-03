// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#include "ResourceCost.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Table.h"

namespace agiru::Projects::Resources::Pricing {

constexpr auto kResourceCostFields = WithSystemFields<ResourceCost_Table>(std::array<FieldDef, 6>{{
    Declare<&ResourceCost_Table::Type>(ResourceCost_Table::Field_No::Type,
                                       "Type",
                                       "Type",
                                       offsetof(ResourceCost_Table, Type),
                                       Declared{.toolTip = "Specifies the type."}),
    Declare<&ResourceCost_Table::Code>(
        ResourceCost_Table::Field_No::Code,
        "Code",
        "Code",
        offsetof(ResourceCost_Table, Code),
        Declared{.relation = "if ( Type = const ( Resource ) ) Resource else if ( Type = const ( "
                             "Group(Resource) ) ) Resource Group",
                 .toolTip = "Specifies the code."}),
    Declare<&ResourceCost_Table::WorkTypeCode>(
        ResourceCost_Table::Field_No::WorkTypeCode,
        "Work Type Code",
        "Work Type Code",
        offsetof(ResourceCost_Table, WorkTypeCode),
        Declared{.relationTable = "Work Type",
                 .relation = "Work Type",
                 .toolTip = "Specifies the code for the type of work. You can also assign a unit "
                            "price to a work type."}),
    Declare<&ResourceCost_Table::CostType>(ResourceCost_Table::Field_No::CostType,
                                           "Cost Type",
                                           "Cost Type",
                                           offsetof(ResourceCost_Table, CostType),
                                           Declared{.toolTip = "Specifies the type of cost."}),
    Declare<&ResourceCost_Table::DirectUnitCost>(
        ResourceCost_Table::Field_No::DirectUnitCost,
        "Direct Unit Cost",
        "Direct Unit Cost",
        offsetof(ResourceCost_Table, DirectUnitCost),
        Declared{.toolTip = "Specifies the cost of one unit of the selected item or resource.",
                 .autoFormatType = "2"}),
    Declare<&ResourceCost_Table::UnitCost>(
        ResourceCost_Table::Field_No::UnitCost,
        "Unit Cost",
        "Unit Cost",
        offsetof(ResourceCost_Table, UnitCost),
        Declared{.toolTip = "Specifies the cost of one unit of the item or resource on the line.",
                 .autoFormatType = "2"}),
}});

constexpr std::array<KeyDef, 2> kResourceCostKeys{{
    KeyDef{.name = "Key1", .fields = ResourceCost_Table::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = ResourceCost_Table::kKey2, .clustered = false},
}};

constexpr TableDef kResourceCostTable{
    .id = ResourceCost_Table::kId,
    .name = ResourceCost_Table::kName,
    .caption = "Resource Cost",
    .fields = kResourceCostFields,
    .keys = kResourceCostKeys,
    .nameSpace = "Microsoft.Projects.Resources.Pricing",
    .dataClassification = "CustomerContent",
};

static_assert(FieldsAreSorted(kResourceCostTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::Projects::Resources::Pricing::ResourceCost_Table, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<ResourceCost_Table>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kResourceCostFields.size() == 6 + kSystemFieldCount,
              "table 202 declares 6 fields, and the platform adds its own");

static_assert(kResourceCostKeys.size() <= ::agiru::kMaximumKeys,
              "a table declares at most 40 keys (devenv-table-keys.md)");
static_assert(ResourceCost_Table::kKey1.size() <= ::agiru::kMaximumPrimaryKeyFields,
              "a primary key names at most 16 fields (devenv-table-keys.md)");
static_assert(!kResourceCostKeys.empty(), "keys[0] IS the primary key, so a table has one");

} // namespace agiru::Projects::Resources::Pricing

namespace agiru::Projects::Resources::Pricing {

namespace {
namespace ResourceCost_unit {
const RegisterTable<ResourceCost_Table> kInCatalogue;
} // namespace ResourceCost_unit
} // namespace

} // namespace agiru::Projects::Resources::Pricing
