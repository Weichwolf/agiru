#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Code.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "Check.h"
#include "ResourceCost.h"
#include "WorkType.h"
#include "options/Types.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

using agiru::CreateTable;
using agiru::Decimal;
using agiru::DropTable;
using agiru::Session;
using ResourceCost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
using ResourceCostCostType = agiru::options::OptionFixedPercentExtraLCYExtra;
using ResourceCostType = agiru::options::OptionResourceGroupResourceAll;
using agiru::app::tables::WorkType;

namespace {

template <bool Keyed> struct CascadeLine : agiru::Table<CascadeLine<Keyed>> {
  agiru::detail::StateHandle State_Block;
  agiru::Code<10> Parent;
  agiru::Integer LineNo;
  agiru::Decimal Amount;
  static constexpr agiru::TableId kId{Keyed ? 50101 : 50102};
  static constexpr std::string_view kName = Keyed ? "Keyed Cascade Line" : "Cascade Line";
};

}

template <bool Keyed> struct agiru::TableTraits<CascadeLine<Keyed>> {
  using Line = CascadeLine<Keyed>;
  static constexpr std::array<agiru::FieldDef, 3> kFields{{
      agiru::Declare<&Line::Parent>(
          agiru::FieldNo{1},
          "Parent",
          "Parent",
          offsetof(Line, Parent),
          agiru::Declared{.relationTable = "Work Type", .relation = "Work Type"}),
      agiru::Declare<&Line::LineNo>(
          agiru::FieldNo{2}, "Line No.", "Line No.", offsetof(Line, LineNo)),
      agiru::Declare<&Line::Amount>(agiru::FieldNo{3}, "Amount", "Amount", offsetof(Line, Amount)),
  }};
  static constexpr std::array<agiru::FieldNo, Keyed ? 2 : 1> kKey = [] {
    if constexpr (Keyed) {
      return std::array{agiru::FieldNo{1}, agiru::FieldNo{2}};
    } else {
      return std::array{agiru::FieldNo{2}};
    }
  }();
  static constexpr std::array<agiru::KeyDef, 1> kKeys{{
      agiru::KeyDef{.name = "Primary", .fields = kKey},
  }};
  static constexpr agiru::TableDef kTable{.id = Line::kId,
                                          .name = Line::kName,
                                          .caption = Line::kName,
                                          .fields = kFields,
                                          .keys = kKeys};
};

namespace {

const agiru::RegisterTable<CascadeLine<true>> kKeyedLines;
const agiru::RegisterTable<CascadeLine<false>> kPlainLines;

void Fresh() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  DropTable(Session::Current().Database(), agiru::TableTraits<WorkType>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<WorkType>::kTable);
  for (const auto *table : {&agiru::TableDefinition<CascadeLine<true>>(),
                            &agiru::TableDefinition<CascadeLine<false>>()}) {
    DropTable(Session::Current().Database(), *table);
    CreateTable(Session::Current().Database(), *table);
  }
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
  cost.SetRange(cost.Code, decltype(cost.Code){code});
  if (!cost.FindFirst()) { return "<missing>"; }
  return std::string(cost.WorkTypeCode.Value());
}

/// A RENAME FOLLOWS EVERY RELATION TO THE KEY. `record-rename-method.md`: "renaming a record
/// changes the primary key and updates the primary key value in all related tables";
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
  type.SetRange(type.Description, decltype(type.Description){"Work type A"});
  CHECK_TRUE("the filtered record finds its row", type.FindFirst());
  type.Rename(agiru::Code<10>("B"));
  CHECK_TEXT("the referring row follows", WorkTypeOf("R1"), "B");
}

void ARecordRefRenamesTheSameRowAndRelatedRows() {
  Fresh();
  WorkTypeNamed("HOURS");
  CostFor("R1", "HOURS");

  WorkType type;
  CHECK_TRUE("RecordRef source row exists", type.Get(agiru::Code<10>("HOURS")));
  agiru::RecordRef reference;
  reference.GetTable(type);
  CHECK_TRUE("RecordRef.Rename reports success",
             reference.Rename(agiru::Variant(agiru::Code<10>("H"))));
  CHECK_TEXT("RecordRef.Rename changes the selected key", reference.Field(1).ToText(), "H");
  CHECK_TEXT("RecordRef.Rename cascades to related rows", WorkTypeOf("R1"), "H");
  WorkType old;
  CHECK_TRUE("RecordRef.Rename removes the old key", !old.Get(agiru::Code<10>("HOURS")));
}

template <bool Keyed>
void CascadeRetainsItsReadAnchor(bool reflected, bool forward, std::int32_t count) {
  Fresh();
  using Line = CascadeLine<Keyed>;
  const char *oldKey = forward ? "A" : "Z";
  const char *newKey = forward ? "Z" : "A";
  WorkTypeNamed(oldKey);
  WorkTypeNamed("OTHER");
  Line row;
  for (std::int32_t index = 1; index <= count + 1; ++index) {
    row.Init();
    row.Parent = index <= count ? oldKey : "OTHER";
    row.LineNo = index;
    row.Amount = Decimal{index};
    row.Insert();
  }
  WorkType parent;
  CHECK_TRUE("cascade source exists", parent.Get(agiru::Code<10>{oldKey}));
  if (reflected) {
    agiru::RecordRef reference;
    reference.GetTable(parent);
    CHECK_TRUE("reflected cascade succeeds", reference.Rename(agiru::Code<10>{newKey}));
  } else {
    CHECK_TRUE("typed cascade succeeds", parent.Rename(agiru::Code<10>{newKey}));
  }
  row.SetRange(row.Parent, agiru::Code<10>{newKey});
  CHECK_TRUE("every referring row follows the renamed key", row.Count() == count);
  row.CalcSums(row.Amount);
  CHECK_TRUE("the cascade retains the complete exact aggregate",
             row.Amount == Decimal{count * (count + 1) / 2});
  row.Reset();
  for (std::int32_t index = 1; index <= count; ++index) {
    const bool found = [&] {
      if constexpr (Keyed) {
        return row.Get(agiru::Code<10>{newKey}, index);
      } else {
        return row.Get(index);
      }
    }();
    CHECK_TRUE("every original line identity survives the cascade", found);
    CHECK_TEXT("every original line has the new parent", std::string(row.Parent.Value()), newKey);
    CHECK_TRUE("every original line retains its exact amount", row.Amount == Decimal{index});
  }
  row.SetRange(row.Parent, agiru::Code<10>{oldKey});
  CHECK_TRUE("no referring line retains the old parent", row.Count() == 0);
  row.SetRange(row.Parent, agiru::Code<10>{"OTHER"});
  CHECK_TRUE("an unrelated parent retains its line", row.FindFirst());
  CHECK_TRUE("the unrelated line retains its exact amount", row.Amount == Decimal{count + 1});
  CHECK_TRUE("the cascade does not duplicate or delete lines", row.Count() == 1);
}

} // namespace

int main() {
  return gate::Run("Rename", [] {
    const Session session(AGIRU_TEST_DSN);
    ARenameCarriesTheRowsThatReferToTheKey();
    AFilteredRecordRenamesTheSame();
    ARecordRefRenamesTheSameRowAndRelatedRows();
    for (const bool reflected : {false, true}) {
      for (const bool forward : {false, true}) {
        for (const std::int32_t count : {1, 64, 130}) {
          CascadeRetainsItsReadAnchor<true>(reflected, forward, count);
          CascadeRetainsItsReadAnchor<false>(reflected, forward, count);
        }
      }
    }
  });
}
