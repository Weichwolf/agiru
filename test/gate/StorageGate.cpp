#include "agiru/Database.h"
#include "agiru/Decimal.h"
#include "agiru/Error.h"
#include "agiru/Option.h"
#include "agiru/Session.h"
#include "agiru/Storage.h"
#include "agiru/Text.h"

#include "Check.h"
#include "ResourceCost.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

using agiru::CreateTable;
using agiru::Decimal;
using agiru::DropTable;
using agiru::Error;
using agiru::Session;
using agiru::app::ResourceCost;
using agiru::app::ResourceCostCostType;
using agiru::app::ResourceCostType;

namespace {

constexpr std::size_t kFieldCount = 6;

ResourceCost Sample() {
  ResourceCost rec;
  rec.Type = ResourceCostType::GroupResource;
  rec.Code = "wt-100";
  rec.WorkTypeCode = "hours";
  rec.CostType = ResourceCostCostType::PercentExtra;
  rec.DirectUnitCost = Decimal::FromInvariantString("12.34");
  rec.UnitCost = Decimal::FromInvariantString("99.99999999999999999999");
  return rec;
}

std::string Column(const agiru::Result &r, std::size_t row, std::size_t column) {
  const std::optional<std::string_view> v = r.Value(row, column);
  return v.has_value() ? std::string(*v) : std::string("<null>");
}

void TheDeclarationBecomesAColumnPerField() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);

  const std::array<std::optional<std::string>, 1> name{std::optional<std::string>("Resource Cost")};
  const agiru::Result columns = Session::Current().Database().Execute(
      "SELECT column_name, data_type, character_maximum_length, numeric_scale "
      "FROM information_schema.columns WHERE table_name = $1 ORDER BY ordinal_position",
      name);

  CHECK_TRUE("six columns", columns.Rows() == kFieldCount);
  CHECK_TEXT("a column keeps the AL name, spaces and all", Column(columns, 2, 0), "Work Type Code");
  CHECK_TEXT("Code[20] becomes a varchar of twenty", Column(columns, 1, 2), "20");
  CHECK_TEXT("Code[10] becomes a varchar of ten", Column(columns, 2, 2), "10");
  CHECK_TEXT("an option becomes an integer", Column(columns, 0, 1), "integer");
  CHECK_TEXT("a decimal keeps twenty places of scale", Column(columns, 5, 3), "20");
  CHECK_TEXT("and a code has no numeric scale of its own", Column(columns, 1, 3), "<null>");
}

/// EVERY LINE BELOW IS THE AL LINE. `Rec.Insert()`, `Rec.Get(a, b, c)`, `Rec.Modify()` -- no
/// connection, no column, no row. That is the point of the base class: the generated table says
/// what the `.al` file says and the platform half is somewhere else.
void ARecordSurvivesTheRoundTrip() {
  const ResourceCost written = Sample();
  written.Insert();

  ResourceCost read;
  CHECK_TRUE("the record is found by its primary key",
             read.Get(written.Type, written.Code, written.WorkTypeCode));
  CHECK_TEXT(
      "a code comes back uppercased, as it went in", std::string(read.Code.Value()), "WT-100");
  CHECK_TEXT("and so does the second one", std::string(read.WorkTypeCode.Value()), "HOURS");
  CHECK_TRUE("an option comes back as itself", read.CostType == written.CostType);

  // THE POINT OF THE WHOLE DECIMAL INVARIANT: twenty decimal places through PostgreSQL and back,
  // digit for digit. A double would have lost this before the row was written.
  CHECK_TEXT("a decimal survives twenty places",
             read.UnitCost.ToInvariantString(),
             "99.99999999999999999999");

  // A DECIMAL THAT HAS BEEN THROUGH THE DATABASE CARRIES THE STORAGE SCALE. The column is
  // numeric(38,20) -- which is what BC stores too -- and a fixed-scale column pads. 12.34 comes
  // back as 12.34000000000000000000: the same VALUE, a different SCALE.
  CHECK_TEXT("the scale after a read is the storage scale",
             read.DirectUnitCost.ToInvariantString(),
             "12.34000000000000000000");
  CHECK_TRUE("and the value is unchanged", read.DirectUnitCost == written.DirectUnitCost);
}

void AMissingKeyIsAnAnswerRatherThanAnError() {
  ResourceCost read;
  CHECK_TRUE("a key that matches nothing returns false",
             !read.Get(agiru::Option<ResourceCostType>{ResourceCostType::All},
                       agiru::Code<20>("NOTHERE"),
                       agiru::Code<10>("")));
}

void ModifyOverwritesTheRowItsKeySelects() {
  ResourceCost rec = Sample();
  rec.UnitCost = Decimal::FromInvariantString("1.00");
  rec.Modify();

  ResourceCost read;
  CHECK_TRUE("and it is found again", read.Get(rec.Type, rec.Code, rec.WorkTypeCode));
  CHECK_TRUE("carrying the new value", read.UnitCost == Decimal::FromInvariantString("1.00"));

  ResourceCost absent = Sample();
  absent.Code = "ABSENT";
  bool threw = false;
  try {
    absent.Modify();
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("modify on a key that matches nothing raises", threw);
}

void TheKeyIsEnforcedByTheDatabase() {
  const ResourceCost again = Sample();
  bool threw = false;
  try {
    again.Insert();
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("a second row with the same primary key is refused", threw);
}

void DeleteRemovesIt() {
  const ResourceCost rec = Sample();
  rec.Delete();
  ResourceCost read;
  CHECK_TRUE("and it is gone", !read.Get(rec.Type, rec.Code, rec.WorkTypeCode));
  bool threw = false;
  try {
    rec.Delete();
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("deleting it twice raises", threw);
}

void ARecordOutsideASessionSaysSoRatherThanCrashing() {
  bool threw = false;
  try {
    const ResourceCost rec;
    rec.Insert();
  } catch (const agiru::SessionError &) { threw = true; }
  CHECK_TRUE("a record operation without a session raises", threw);
}

} // namespace

int main() {
  // A GATE THAT CANNOT REACH ITS DATABASE IS RED, NOT SKIPPED. A skipped case reports green and
  // proves nothing, which is the first trap on CLAUDE.md's list -- so the handler is the shared one
  // in Check.h and the failure carries the reason.
  return gate::Run("Storage", [] {
    {
      const Session session(AGIRU_TEST_DSN);
      TheDeclarationBecomesAColumnPerField();
      ARecordSurvivesTheRoundTrip();
      AMissingKeyIsAnAnswerRatherThanAnError();
      ModifyOverwritesTheRowItsKeySelects();
      TheKeyIsEnforcedByTheDatabase();
      DeleteRemovesIt();
    }
    ARecordOutsideASessionSaysSoRatherThanCrashing();
  });
}
