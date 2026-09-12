#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Code.h"
#include "type/Decimal.h"
#include "type/Option.h"

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
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostCostType;
using agiru::app::tables::ResourceCostType;

namespace {

// THE AL NUMBER AND THE PLATFORM'S ADDITION ARE SAID APART, so that the first can be checked
// against `ResourceCost.Table.al` and the second against `devenv-table-system-fields.md`. A single
// total of eleven could be compared with neither.
constexpr std::size_t kDeclaredFields = 6;
constexpr std::size_t kFieldCount = kDeclaredFields + agiru::kSystemFieldCount;

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

  CHECK_TRUE("a column per declared field, and one per system field",
             columns.Rows() == kFieldCount);
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
  ResourceCost written = Sample();
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
  ResourceCost again = Sample();
  bool threw = false;
  try {
    again.Insert();
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("a second row with the same primary key is refused", threw);
}

void DeleteRemovesIt() {
  // NOT `const`: AL has no const record, and `Delete(RunTrigger)` runs `OnDelete`, which is AL code
  // that may write into the row it is about to remove. One overload const and the other not would
  // make which one is chosen depend on the caller's declaration.
  ResourceCost rec = Sample();
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
    ResourceCost rec;
    rec.Insert();
  } catch (const agiru::SessionError &) { threw = true; }
  CHECK_TRUE("a record operation without a session raises", threw);
}

/// THE PLATFORM STAMPS THE SYSTEM FIELDS, and a record that went in with a blank SystemId comes
/// back out with one. `devenv-table-system-fields.md`: "When a new record is created, before
/// calling Insert, the audit fields are given blank GUIDs and blank dates as values. When a record
/// is first inserted, the fields are populated with actual values ... The $systemCreatedBy and
/// $systemModifiedBy fields are given the same value. So are the $systemCreatedAt and
/// $systemModifiedAt fields."
void ThePlatformStampsWhatItWrites() {
  ResourceCost rec = Sample();
  rec.Code = "stamped";
  CHECK_TRUE("a record starts with a blank SystemId", rec.SystemId.IsNull());
  CHECK_TRUE("and a blank created instant", rec.SystemCreatedAt.IsUndefined());

  rec.Insert();
  CHECK_TRUE("Insert gives it a SystemId", !rec.SystemId.IsNull());
  CHECK_TRUE("and a created instant", !rec.SystemCreatedAt.IsUndefined());
  CHECK_TRUE("created and modified are the same instant on insert",
             rec.SystemCreatedAt == rec.SystemModifiedAt);
  CHECK_TRUE("and the two SIDs are the same", rec.SystemCreatedBy == rec.SystemModifiedBy);

  // IT REACHED THE DATABASE AND NOT ONLY THE RECORD, which is the half that would otherwise pass
  // while every row on disk carried the blank GUID.
  ResourceCost read;
  CHECK_TRUE("the row is found", read.Get(rec.Type, rec.Code, rec.WorkTypeCode));
  CHECK_TRUE("and carries the same SystemId", read.SystemId == rec.SystemId);
  CHECK_TRUE("and the same created instant", read.SystemCreatedAt == rec.SystemCreatedAt);

  const agiru::Guid identity = rec.SystemId;
  const agiru::DateTime created = rec.SystemCreatedAt;
  rec.UnitCost = Decimal::FromInvariantString("7.00");
  rec.Modify();
  CHECK_TRUE("Modify leaves the SystemId where it was", rec.SystemId == identity);
  CHECK_TRUE("and the created instant", rec.SystemCreatedAt == created);
  CHECK_TRUE("and never moves the modified one backwards", rec.SystemModifiedAt >= created);

  rec.Delete();
}

/// THE NEGATIVE CONTROL, and it is the surprising half of the rule rather than a mirror of the
/// case above: a SystemId assigned before a plain `Insert()` is THROWN AWAY.
/// `record-insert-boolean-boolean-method.md` on `InsertWithSystemId`: "If this parameter is false,
/// the SystemId field is given a value that is auto-generated by the platform. The default value is
/// false." A reader who assumed the assignment survives would be wrong, and so would a runtime that
/// only filled a blank one.
void AnAssignedSystemIdDoesNotSurviveAPlainInsert() {
  const agiru::Guid chosen = *agiru::Guid::FromText("{B6666666-F5A2-E911-8180-001DD8B7338E}");
  ResourceCost rec = Sample();
  rec.Code = "chosen";
  rec.SystemId = chosen;
  CHECK_TRUE("the record carries the chosen SystemId going in", rec.SystemId == chosen);

  rec.Insert();
  CHECK_TRUE("and a plain Insert replaces it", rec.SystemId != chosen);
  CHECK_TRUE("with one of its own", !rec.SystemId.IsNull());
  rec.Delete();
}

/// MODIFYALL ASSIGNS AND NEVER VALIDATES. `record-modifyall-method.md`: "The OnValidate field
/// trigger is never run when ModifyAll is used" -- `RunTrigger` is `OnModify` alone. `Resource
/// Cost."Cost Type"` validates as `TestField("Cost Type", Fixed)` when the work type is blank, so
/// a validating ModifyAll could not set `% Extra` on such a row, and the negative control shows
/// that `Validate` still refuses it.
void ModifyAllAssignsWithoutValidating() {
  ResourceCost rec = Sample();
  rec.Code = "bulk";
  rec.WorkTypeCode = "";
  rec.CostType = ResourceCostCostType::Fixed;
  rec.Insert();

  ResourceCost all;
  all.SetRange(all.Code, agiru::Code<20>("bulk"));
  all.ModifyAll(all.CostType, ResourceCostCostType::PercentExtra, true);
  ResourceCost read;
  CHECK_TRUE("the row is found", read.Get(rec.Type, rec.Code, rec.WorkTypeCode));
  CHECK_TRUE("and carries the assigned cost type",
             read.CostType == ResourceCostCostType::PercentExtra);

  bool refused = false;
  try {
    read.Validate(read.CostType, ResourceCostCostType::LCYExtra);
  } catch (const Error &) { refused = true; }
  CHECK_TRUE("while Validate on the same row runs the trigger and refuses", refused);
  rec.Delete();
}

/// THE OTHER OVERLOAD KEEPS WHAT WAS ASSIGNED. `record-insert-boolean-boolean-method.md`: "If this
/// parameter is true, the SystemId field of the record is given a value that you explicitly assign.
/// If a value is not assigned, then the platform assigns one." `Sales-Post` writes a posted line
/// with the sales line's SystemId this way, and the aggregate buffers find it again by that id.
void AnAssignedSystemIdSurvivesInsertWithSystemId() {
  const agiru::Guid chosen = *agiru::Guid::FromText("{B7777777-F5A2-E911-8180-001DD8B7338E}");
  ResourceCost rec = Sample();
  rec.Code = "kept";
  rec.SystemId = chosen;
  rec.Insert(true, true);
  CHECK_TRUE("Insert(true, true) keeps the chosen SystemId", rec.SystemId == chosen);
  ResourceCost read;
  CHECK_TRUE("the row is found", read.Get(rec.Type, rec.Code, rec.WorkTypeCode));
  CHECK_TRUE("and the row carries it", read.SystemId == chosen);
  rec.Delete();

  ResourceCost blank = Sample();
  blank.Code = "unassigned";
  blank.Insert(false, true);
  CHECK_TRUE("and a blank one is still filled by the platform", !blank.SystemId.IsNull());
  blank.Delete();
}

/// THE ROW'S IDENTITY AND ITS CREATION STAMP ARE THE PLATFORM'S, WHATEVER THE RECORD SAYS.
/// `devenv-table-system-fields.md`: "You can assign the values, but the values written to the
/// database are always provided by the platform" and "The $systemCreatedBy and $systemCreatedAt
/// fields won't change after this point." A Modify after `TransferFields` from another table's row
/// would otherwise write that row's SystemId over this one's.
void ModifyLeavesTheIdentityAndTheCreationStampToThePlatform() {
  ResourceCost rec = Sample();
  rec.Code = "owned";
  rec.Insert();
  const agiru::Guid identity = rec.SystemId;
  const agiru::DateTime created = rec.SystemCreatedAt;
  const agiru::Guid createdBy = rec.SystemCreatedBy;

  rec.SystemId = *agiru::Guid::FromText("{B8888888-F5A2-E911-8180-001DD8B7338E}");
  rec.SystemCreatedAt = agiru::DateTime{};
  rec.SystemCreatedBy = agiru::Guid{};
  rec.UnitCost = Decimal::FromInvariantString("8.00");
  rec.Modify();
  CHECK_TRUE("after Modify the record carries the row's SystemId again", rec.SystemId == identity);
  CHECK_TRUE("and the row's creation instant", rec.SystemCreatedAt == created);
  CHECK_TRUE("and the row's creator", rec.SystemCreatedBy == createdBy);

  ResourceCost read;
  CHECK_TRUE("the row is found by its key", read.Get(rec.Type, rec.Code, rec.WorkTypeCode));
  CHECK_TRUE("with its identity intact", read.SystemId == identity);
  CHECK_TRUE("its creation instant intact", read.SystemCreatedAt == created);
  CHECK_TRUE("and the new value written", read.UnitCost == Decimal::FromInvariantString("8.00"));
  rec.Delete();
}

/// TRANSFERFIELDS COPIES THE DECLARED FIELDS AND NOT THE SYSTEM FIELDS. The BaseApp says so nine
/// times over by assigning `SystemId` right after a `TransferFields` (`GraphMgtSalCrMemoBuf
/// .LoadSalesCreditMemoLines` and its siblings), and `OnBeforeSalesCrMemoHeaderInsert` reads the
/// posted header's SystemId BEFORE its insert to tell it from the draft's -- a copied one would
/// have made the two equal and the draft buffer would never be renamed (ERM Sales Cr. Memo Aggr.
/// UT, 2026-09-12).
void TransferFieldsLeavesTheSystemFieldsAlone() {
  ResourceCost source = Sample();
  source.Code = "source";
  source.Insert();
  CHECK_TRUE("the source has an identity", !source.SystemId.IsNull());

  ResourceCost target;
  target.TransferFields(source);
  CHECK_TRUE("TransferFields copies a declared field", target.UnitCost == source.UnitCost);
  CHECK_TRUE("and the primary key", target.Code == source.Code);
  CHECK_TRUE("but not the SystemId", target.SystemId.IsNull());
  CHECK_TRUE("nor the creation instant", target.SystemCreatedAt.IsUndefined());
  source.Delete();
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
      ThePlatformStampsWhatItWrites();
      AnAssignedSystemIdDoesNotSurviveAPlainInsert();
      AnAssignedSystemIdSurvivesInsertWithSystemId();
      ModifyLeavesTheIdentityAndTheCreationStampToThePlatform();
      TransferFieldsLeavesTheSystemFieldsAlone();
      ModifyAllAssignsWithoutValidating();
      DeleteRemovesIt();
    }
    ARecordOutsideASessionSaysSoRatherThanCrashing();
  });
}
