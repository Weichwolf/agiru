// Generated from SourceRow.Table.al. Do not edit.

#include "SourceRow.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Table.h"

#include "fixture/table/SourceRow.h"

namespace agiru::Fixture {

constexpr auto kSourceRowFields = WithSystemFields<SourceRow_Table>(std::array<FieldDef, 2>{{
    Declare<&SourceRow_Table::ID>(SourceRow_Table::Field_No::ID, "ID", "ID", offsetof(SourceRow_Table, ID)),
    Declare<&SourceRow_Table::AddedFlag>(SourceRow_Table::Field_No::AddedFlag, "Added Flag", "Added Flag", offsetof(SourceRow_Table, AddedFlag)),
}});

constexpr std::array<KeyDef, 0> kSourceRowKeys{{
}};

constexpr TableDef kSourceRowTable{
    .id = SourceRow_Table::kId,
    .name = SourceRow_Table::kName,
    .caption = SourceRow_Table::kName,
    .fields = kSourceRowFields,
    .keys = kSourceRowKeys,
};

static_assert(FieldsAreSorted(kSourceRowTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(offsetof(agiru::Fixture::SourceRow_Table, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<SourceRow_Table>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kSourceRowFields.size() == 2 + kSystemFieldCount, "table 50170 declares 2 fields, and the platform adds its own");

static_assert(kSourceRowKeys.size() <= ::agiru::kMaximumKeys,
              "a table declares at most 40 keys (devenv-table-keys.md)");

} // namespace agiru::Fixture

namespace agiru::Fixture {

namespace {
namespace SourceRow_unit {
const RegisterTable<SourceRow_Table> kInCatalogue;
} // namespace SourceRow_unit
} // namespace

} // namespace agiru::Fixture
