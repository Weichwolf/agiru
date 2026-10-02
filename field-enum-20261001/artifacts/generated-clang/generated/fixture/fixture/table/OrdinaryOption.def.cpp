// Generated from Option.Table.al. Do not edit.

#include "OrdinaryOption.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Option.h"

#include "fixture/table/OrdinaryOption.h"

namespace agiru::Fixture {

constexpr auto kOrdinaryOptionFields = WithSystemFields<OrdinaryOption_Table>(std::array<FieldDef, 2>{{
    Declare<&OrdinaryOption_Table::ID>(OrdinaryOption_Table::Field_No::ID, "ID", "ID", offsetof(OrdinaryOption_Table, ID)),
    Declare<&OrdinaryOption_Table::State_2>(OrdinaryOption_Table::Field_No::State_2, "State", "State", offsetof(OrdinaryOption_Table, State_2)),
}});

constexpr std::array<KeyDef, 1> kOrdinaryOptionKeys{{
    KeyDef{.name = "PK", .fields = OrdinaryOption_Table::kKey1, .clustered = true},
}};

constexpr TableDef kOrdinaryOptionTable{
    .id = OrdinaryOption_Table::kId,
    .name = OrdinaryOption_Table::kName,
    .caption = OrdinaryOption_Table::kName,
    .fields = kOrdinaryOptionFields,
    .keys = kOrdinaryOptionKeys,
};

static_assert(FieldsAreSorted(kOrdinaryOptionTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::Fixture::OrdinaryOption_Table, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<OrdinaryOption_Table>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kOrdinaryOptionFields.size() == 2 + kSystemFieldCount, "table 50222 declares 2 fields, and the platform adds its own");

static_assert(kOrdinaryOptionKeys.size() <= ::agiru::kMaximumKeys,
              "a table declares at most 40 keys (devenv-table-keys.md)");
static_assert(OrdinaryOption_Table::kKey1.size() <= ::agiru::kMaximumPrimaryKeyFields,
              "a primary key names at most 16 fields (devenv-table-keys.md)");
static_assert(!kOrdinaryOptionKeys.empty(), "keys[0] IS the primary key, so a table has one");

} // namespace agiru::Fixture

namespace agiru::Fixture {

namespace {
namespace OrdinaryOption_unit {
const RegisterTable<OrdinaryOption_Table> kInCatalogue;
} // namespace OrdinaryOption_unit
} // namespace

} // namespace agiru::Fixture
