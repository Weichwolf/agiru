// Generated from StoredFlag.Table.al. Do not edit.

#include "StoredFlag.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Table.h"

#include "fixture/table/StoredFlag.h"

namespace agiru::Fixture {

constexpr auto kStoredFlagFields = WithSystemFields<StoredFlag_Table>(std::array<FieldDef, 2>{{
    Declare<&StoredFlag_Table::ID>(StoredFlag_Table::Field_No::ID, "ID", "ID", offsetof(StoredFlag_Table, ID)),
    Declare<&StoredFlag_Table::Temporary>(StoredFlag_Table::Field_No::Temporary, "Temporary", "Temporary", offsetof(StoredFlag_Table, Temporary)),
}});

constexpr std::array<KeyDef, 1> kStoredFlagKeys{{
    KeyDef{.name = "PK", .fields = StoredFlag_Table::kKey1, .clustered = true},
}};

constexpr TableDef kStoredFlagTable{
    .id = StoredFlag_Table::kId,
    .name = StoredFlag_Table::kName,
    .caption = StoredFlag_Table::kName,
    .fields = kStoredFlagFields,
    .keys = kStoredFlagKeys,
};

static_assert(FieldsAreSorted(kStoredFlagTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::Fixture::StoredFlag_Table, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<StoredFlag_Table>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kStoredFlagFields.size() == 2 + kSystemFieldCount, "table 50231 declares 2 fields, and the platform adds its own");

static_assert(kStoredFlagKeys.size() <= ::agiru::kMaximumKeys,
              "a table declares at most 40 keys (devenv-table-keys.md)");
static_assert(StoredFlag_Table::kKey1.size() <= ::agiru::kMaximumPrimaryKeyFields,
              "a primary key names at most 16 fields (devenv-table-keys.md)");
static_assert(!kStoredFlagKeys.empty(), "keys[0] IS the primary key, so a table has one");

} // namespace agiru::Fixture

namespace agiru::Fixture {

namespace {
namespace StoredFlag_unit {
const RegisterTable<StoredFlag_Table> kInCatalogue;
} // namespace StoredFlag_unit
} // namespace

} // namespace agiru::Fixture
