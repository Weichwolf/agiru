#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Relation.h"

#include "Check.h"
#include "RelationBranches.h"
#include "ResourceCost.h"

#include <optional>
#include <string>
#include <vector>

using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostType;

namespace {

/// A CONDITIONAL `TableRelation` IS READ AGAINST THE RECORD: `Resource Cost.Code` declares
/// `if (Type = const(Resource)) Resource else if (Type = const("Group(Resource)")) "Resource
/// Group"`
/// (`devenv-set-relationships-between-tables.md`), and which table the code relates to is a
/// question about THIS record's `Type` (board:0658; 736 conditional declarations in the BaseApp).
void AConditionalRelationFollowsTheRecordsType() {
  const agiru::FieldDef *code =
      agiru::Field(agiru::app::tables::kResourceCostTable, ResourceCost::Field_No::Code);
  CHECK_TRUE("the target image carries the whole declaration",
             code != nullptr && !code->relation.empty());
  ResourceCost rec;
  rec.Type = ResourceCostType::Resource;
  std::optional<agiru::detail::ResolvedRelation> found =
      agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, *code);
  CHECK_TRUE("a resource relates to Resource", found.has_value() && found->table == "Resource");
  CHECK_TEXT("by its primary key", found.has_value() ? found->field : std::string("?"), "");
  rec.Type = ResourceCostType::GroupResource;
  found = agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, *code);
  CHECK_TRUE("a group relates to Resource Group",
             found.has_value() && found->table == "Resource Group");
  rec.Type = ResourceCostType::All;
  found = agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, *code);
  CHECK_TRUE("and All matches no branch, so no relation", !found.has_value());
}

/// A `where(...)` CLAUSE IS EVALUATED TOO: `field(X)` reads this record's X, `const(V)` is V and
/// `filter(F)` is the filter text, each as the related table's `SetFilter` takes it -- the three
/// terms a `SubPageLink` also uses (2 502 filtered declarations).
void AWhereClauseReadsTheRecordsFields() {
  ResourceCost rec;
  rec.Code = "R100";
  const agiru::FieldDef declared{
      .name = "Unit of Measure Code",
      .relation = "\"Item Unit of Measure\".Code where(\"Item No.\" = field(Code), "
                  "Type = const(Inventory), \"Qty. per Unit of Measure\" = filter(<>0))"};
  const std::optional<agiru::detail::ResolvedRelation> found =
      agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, declared);
  CHECK_TRUE("the table and its field are read",
             found.has_value() && found->table == "Item Unit of Measure" && found->field == "Code");
  CHECK_TRUE("three terms", found.has_value() && found->filters.size() == 3);
  if (found.has_value() && found->filters.size() == 3) {
    CHECK_TEXT("field(Code) is this record's Code, literally",
               found->filters[0].text,
               agiru::detail::Literally("R100"));
    CHECK_TEXT("naming the related field", found->filters[0].field, "Item No.");
    CHECK_TEXT("const(Inventory) is the value, literally",
               found->filters[1].text,
               agiru::detail::Literally("Inventory"));
    CHECK_TEXT("filter(<>0) is the filter text as written", found->filters[2].text, "<>0");
  }
  const agiru::FieldDef simple{.name = "Work Type Code", .relationTable = "Work Type"};
  const std::optional<agiru::detail::ResolvedRelation> bare =
      agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, simple);
  CHECK_TRUE("the simple form still answers through its two fields",
             bare.has_value() && bare->table == "Work Type" && bare->filters.empty());
}

/// THE GENERATOR WRITES THE RELATION AS ITS TOKENS JOINED BY SPACES, so the table/field separator
/// is ` . ` with a space on either side and a quoted identifier arrives WITHOUT its quotes:
/// `Sales Header . No. where ( Document Type = field ( Document Type ) )`, and a table whose own
/// name carries dots -- `Purch. Cr. Memo Hdr.` -- is one name. Splitting at the first dot read
/// `Purch` as the table, so no rename of a posted credit memo reached its lines and no lookup on
/// such a field found its table (ERM Purch. Cr. Memo Aggr. UT.TestRenamePostedCrMemo,
/// 2026-09-12).
void TheSpacedDotSeparatesTableAndField() {
  ResourceCost rec;
  rec.Type = ResourceCostType::Resource;
  const agiru::FieldDef posted{.name = "Document No.", .relation = "Purch. Cr. Memo Hdr."};
  std::optional<agiru::detail::ResolvedRelation> found =
      agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, posted);
  CHECK_TRUE("a table name with dots is one name",
             found.has_value() && found->table == "Purch. Cr. Memo Hdr." && found->field.empty());

  const agiru::FieldDef linked{.name = "Document No.",
                               .relation =
                                   "Sales Header . No. where ( Document Type = const ( Order ) )"};
  found = agiru::detail::ResolveRelation(&rec, agiru::app::tables::kResourceCostTable, linked);
  CHECK_TRUE("the spaced dot splits table and field",
             found.has_value() && found->table == "Sales Header" && found->field == "No.");
  CHECK_TRUE("and the where term is read",
             found.has_value() && found->filters.size() == 1 &&
                 found->filters[0].field == "Document Type");

  const std::vector<agiru::detail::RelationBranch> branches =
      agiru::detail::RelationBranches(posted);
  CHECK_TRUE("the declared form reads the same name",
             branches.size() == 1 && branches[0].table == "Purch. Cr. Memo Hdr.");
  const std::vector<agiru::detail::RelationBranch> split = agiru::detail::RelationBranches(linked);
  CHECK_TRUE("and the same split",
             split.size() == 1 && split[0].table == "Sales Header" && split[0].field == "No." &&
                 split[0].filters.size() == 1);
}

} // namespace

int main() {
  return gate::Run("Relation", [] {
    AConditionalRelationFollowsTheRecordsType();
    AWhereClauseReadsTheRecordsFields();
    TheSpacedDotSeparatesTableAndField();
  });
}
