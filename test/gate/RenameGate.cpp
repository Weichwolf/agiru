#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Code.h"
#include "type/Decimal.h"

#include "Check.h"
#include "ResourceCost.h"
#include "WorkType.h"

#include <string>

using agiru::CreateTable;
using agiru::Decimal;
using agiru::DropTable;
using agiru::Session;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostCostType;
using agiru::app::tables::ResourceCostType;
using agiru::app::tables::WorkType;

namespace {

void Fresh() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  DropTable(Session::Current().Database(), agiru::TableTraits<WorkType>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<WorkType>::kTable);
}

void WorkTypeNamed(const char *code) {
  WorkType type;
  type.Code = code;
  type.Description = std::string("Work type ") + code;
  type.Insert();
}

void CostFor(const char *code, const char *workType) {
  ResourceCost cost;
  cost.Type = ResourceCostType::Resource;
  cost.Code = code;
  cost.WorkTypeCode = workType;
  cost.CostType = ResourceCostCostType::Fixed;
  cost.UnitCost = Decimal::FromInvariantString("1.00");
  cost.Insert();
}

// THE WORK TYPE IS PART OF THE COST'S PRIMARY KEY, so the line is found by its code alone and
// the key field it carries is what the cascade rewrote -- through `Rename`, not `Modify`.
std::string WorkTypeOf(const char *code) {
  ResourceCost cost;
  cost.SetRange(cost.Code, agiru::Code<20>(code));
  if (!cost.FindFirst()) { return "<missing>"; }
  return std::string(cost.WorkTypeCode.Value());
}

/// A RENAME FOLLOWS EVERY RELATION TO THE KEY. `record-rename-method.md`: "renaming a record changes
/// the primary key and updates the primary key value in all related tables";
/// `devenv-set-relationships-between-tables.md`: "if you change one of the currency codes in the
/// Currency Code table, then the change is automatically propagated to all tables that refer to
/// this code." `Resource Cost."Work Type Code"` relates to `Work Type`, so renaming the work type
/// carries the cost lines with it; the posted credit memo's lines follow the header's number the
/// same way (ERM Sales Cr. Memo Aggr. UT, TestRenamePostedCrMemo, 2026-09-12; openerp WI-1162,
/// measured +3).
void ARenameCarriesTheRowsThatReferToTheKey() {
  Fresh();
  WorkTypeNamed("HOURS");
  WorkTypeNamed("DAYS");
  CostFor("R1", "HOURS");
  CostFor("R2", "HOURS");
  CostFor("R3", "DAYS");

  WorkType type;
  CHECK_TRUE("the work type is there", type.Get(agiru::Code<10>("HOURS")));
  type.Rename(agiru::Code<10>("H"));
  CHECK_TEXT("the renamed record carries the new key", std::string(type.Code.Value()), "H");

  CHECK_TEXT("a cost line that referred to it follows", WorkTypeOf("R1"), "H");
  CHECK_TEXT("and so does the second", WorkTypeOf("R2"), "H");
  // THE NEGATIVE CONTROL: a row that referred to ANOTHER key is not touched.
  CHECK_TEXT("a line referring to another work type stays", WorkTypeOf("R3"), "DAYS");

  WorkType old;
  CHECK_TRUE("and the old key is gone", !old.Get(agiru::Code<10>("HOURS")));
  WorkType renamed;
  CHECK_TRUE("while the new one is found", renamed.Get(agiru::Code<10>("H")));
}

/// THE CASCADE FILTERS ON THE OLD VALUE AND NOT ON THE ROWS' OWN FILTERS: a record variable with a
/// filter set on it renames the row and the related rows all the same.
void AFilteredRecordRenamesTheSame() {
  Fresh();
  WorkTypeNamed("A");
  CostFor("R1", "A");

  WorkType type;
  type.SetRange(type.Description, agiru::Text<100>("Work type A"));
  CHECK_TRUE("the filtered record finds its row", type.FindFirst());
  type.Rename(agiru::Code<10>("B"));
  CHECK_TEXT("the referring row follows", WorkTypeOf("R1"), "B");
}

} // namespace

int main() {
  return gate::Run("Rename", [] {
    const Session session(AGIRU_TEST_DSN);
    ARenameCarriesTheRowsThatReferToTheKey();
    AFilteredRecordRenamesTheSame();
  });
}
