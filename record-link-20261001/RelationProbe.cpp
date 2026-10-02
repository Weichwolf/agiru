#include "meta/TableDef.h"
#include "platform/RecordLink.h"
#include "runtime/Relation.h"

#include "Check.h"
#include "RelationBranches.h"

#include <optional>
#include <vector>

int main() {
  return gate::Run("QualifiedRecordLinkRelation", [] {
    agiru::platform::RecordLink record;
    const auto &table = agiru::platform::kRecordLinkTable;
    const auto *company = agiru::Field(table, agiru::platform::RecordLink::Field_No::Company);
    CHECK_TRUE("source Company field retained", company != nullptr);
    const agiru::FieldDef declared = company != nullptr ? *company : agiru::FieldDef{};
    const auto branches = agiru::detail::RelationBranches(declared);
    CHECK_TRUE("one original relation branch", branches.size() == 1);
    CHECK_TEXT("declared qualified table", branches.size() == 1 ? branches[0].table : "?",
               "System.Environment.Company");
    CHECK_TEXT("declared target field", branches.size() == 1 ? branches[0].field : "?", "Name");
    const auto resolved = agiru::detail::ResolveRelation(&record, table, declared);
    CHECK_TRUE("original relation resolves", resolved.has_value());
    CHECK_TEXT("resolved qualified table", resolved.has_value() ? resolved->table : "?",
               "System.Environment.Company");
    CHECK_TEXT("resolved target field", resolved.has_value() ? resolved->field : "?", "Name");
    agiru::FieldDef split = declared;
    split.relation = {};
    const auto controlBranches = agiru::detail::RelationBranches(split);
    const auto controlResolved = agiru::detail::ResolveRelation(&record, table, split);
    CHECK_TRUE("split control retains the qualified table",
               controlBranches.size() == 1 && controlBranches[0].table == split.relationTable);
    CHECK_TRUE("split control retains the target field",
               controlBranches.size() == 1 && controlBranches[0].field == split.relationField);
    CHECK_TRUE("split resolution retains the qualified table",
               controlResolved.has_value() && controlResolved->table == split.relationTable);
    CHECK_TRUE("split resolution retains the target field",
               controlResolved.has_value() && controlResolved->field == split.relationField);
  });
}
