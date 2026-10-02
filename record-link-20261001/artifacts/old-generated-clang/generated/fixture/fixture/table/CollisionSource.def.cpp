// Generated from Collision.Table.al. Do not edit.

#include "CollisionSource.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Table.h"

#include "fixture/table/CollisionSource.h"

namespace agiru::Fixture {

constexpr auto kCollisionSourceFields = WithSystemFields<CollisionSource_Table>(std::array<FieldDef, 2>{{
    Declare<&CollisionSource_Table::EntryNo>(CollisionSource_Table::Field_No::EntryNo, "Entry No.", "Entry No.", offsetof(CollisionSource_Table, EntryNo)),
    Declare<&CollisionSource_Table::RecordID>(CollisionSource_Table::Field_No::RecordID, "Record ID", "Record ID", offsetof(CollisionSource_Table, RecordID)),
}});

constexpr std::array<KeyDef, 1> kCollisionSourceKeys{{
    KeyDef{.name = "PK", .fields = CollisionSource_Table::kKey1, .clustered = true},
}};

constexpr TableDef kCollisionSourceTable{
    .id = CollisionSource_Table::kId,
    .name = CollisionSource_Table::kName,
    .caption = CollisionSource_Table::kName,
    .fields = kCollisionSourceFields,
    .keys = kCollisionSourceKeys,
};

static_assert(FieldsAreSorted(kCollisionSourceTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::Fixture::CollisionSource_Table, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<CollisionSource_Table>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kCollisionSourceFields.size() == 2 + kSystemFieldCount, "table 50197 declares 2 fields, and the platform adds its own");

static_assert(kCollisionSourceKeys.size() <= ::agiru::kMaximumKeys,
              "a table declares at most 40 keys (devenv-table-keys.md)");
static_assert(CollisionSource_Table::kKey1.size() <= ::agiru::kMaximumPrimaryKeyFields,
              "a primary key names at most 16 fields (devenv-table-keys.md)");
static_assert(!kCollisionSourceKeys.empty(), "keys[0] IS the primary key, so a table has one");

} // namespace agiru::Fixture

namespace agiru::Fixture {

namespace {
namespace CollisionSource_unit {
const RegisterTable<CollisionSource_Table> kInCatalogue;
} // namespace CollisionSource_unit
} // namespace

} // namespace agiru::Fixture
