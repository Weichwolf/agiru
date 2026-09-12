// Generated from Utilities/WorkType.Table.al. Do not edit.

#include "WorkType.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"

namespace agiru::app::tables {

constexpr auto kWorkTypeFields = WithSystemFields<WorkType>(std::array<FieldDef, 3>{{
    Declare<&WorkType::Code>(WorkType::Field_No::Code,
                             "Code",
                             "Code",
                             offsetof(WorkType, Code),
                             Declared{.notBlank = true}),
    Declare<&WorkType::Description>(
        WorkType::Field_No::Description, "Description", "Description", offsetof(WorkType, Description)),
    Declare<&WorkType::UnitOfMeasureCode>(WorkType::Field_No::UnitOfMeasureCode,
                                          "Unit of Measure Code",
                                          "Unit of Measure Code",
                                          offsetof(WorkType, UnitOfMeasureCode),
                                          Declared{.relationTable = "Unit of Measure",
                                                   .relation = "Unit of Measure"}),
}});

constexpr std::array<KeyDef, 1> kWorkTypeKeys{{
    KeyDef{.name = "Key1", .fields = WorkType::kKey1, .clustered = true},
}};

constexpr TableDef kWorkTypeTable{
    .id = WorkType::kId,
    .name = WorkType::kName,
    .caption = WorkType::kName,
    .fields = kWorkTypeFields,
    .keys = kWorkTypeKeys,
    .lookupPageId = ::agiru::PageId{208},
    .drillDownPageId = ::agiru::PageId{208},
};

static_assert(FieldsAreSorted(kWorkTypeTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::app::tables::WorkType, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<WorkType>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kWorkTypeFields.size() == 3 + kSystemFieldCount,
              "table 200 declares 3 fields, and the platform adds its own");
static_assert(!kWorkTypeKeys.empty(), "keys[0] IS the primary key, so a table has one");

} // namespace agiru::app::tables
