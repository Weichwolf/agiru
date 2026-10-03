#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Table.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/table/ImplicitRow.h"

namespace {

using Row = agiru::Fixture::ImplicitRow_Table;
constexpr agiru::Integer kFirstKey = 7;

void CheckKeys() {
  const auto &table = agiru::TableTraits<Row>::kTable;
  CHECK_TRUE("extension keys never replace the implicit primary key", table.keys.size() == 2);
  if (table.keys.empty()) { return; }
  const auto &primary = table.keys.front();
  CHECK_TEXT("the primary key retains the original AL field name", primary.name, "Primary ID");
  CHECK_TRUE("the primary key belongs to the base table, not the lower-ID extension field",
             primary.fields.size() == 1 && primary.fields.front() == agiru::FieldNo{10});
  CHECK_TRUE("the primary key is clustered by default", primary.clustered);
  if (table.keys.size() == 2) {
    CHECK_TEXT("the extension retains its secondary key name", table.keys[1].name, "ExtensionKey");
    CHECK_TRUE("the extension key keeps its own field",
               table.keys[1].fields.size() == 1 &&
                   table.keys[1].fields.front() == agiru::FieldNo{1});
    CHECK_TRUE("an extension key is not implicitly clustered", !table.keys[1].clustered);
  }
  if (primary.fields.size() != 1 || primary.fields.front() != agiru::FieldNo{10}) { return; }
  agiru::Temporary<Row> row;
  row.PrimaryID = kFirstKey;
  row.EarlierExtension = "same";
  row.Later = "first";
  row.Insert();
  row.PrimaryID = 3;
  row.Later = "second";
  row.Insert();
  CHECK_TRUE("the implicit primary key retrieves a stored record",
             static_cast<bool>(row.Get(kFirstKey)));
  CHECK_TEXT("Get restores the record selected by the primary ID", row.Later.Value(), "first");
  CHECK_TRUE("a missing primary key stays missing", !row.Get(8));
  bool refused = false;
  try {
    row.PrimaryID = kFirstKey;
    row.Insert();
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("temporary duplicate detection uses the implicit primary key", refused);
  CHECK_TRUE("duplicate refusal leaves the row count unchanged", row.Count() == 2);
  CHECK_TRUE("primary-order navigation finds the lowest ID first",
             static_cast<bool>(row.FindFirst()));
  CHECK_TRUE("primary-order navigation ignores secondary text and declaration order",
             row.PrimaryID == 3);
}

void CheckSourceIdentity() {
  const auto &table = agiru::TableTraits<Row>::kTable;
  CHECK_TRUE("table metadata retains the original app identity", table.module != nullptr);
  if (table.module == nullptr) { return; }
  CHECK_TEXT("table owner comes from the source manifest",
             table.module->id,
             "118874ab-44bc-4ccb-9daf-59763539ab16");
  CHECK_TEXT("the original app name is not the configured output name",
             table.module->name,
             "Table Declaration Fixture");
  CHECK_TEXT("the original app publisher is retained", table.module->publisher, "agiru tests");
  CHECK_TEXT("the original app version is retained", table.module->version, "1.0.0.0");
  CHECK_TEXT(
      "table namespace is AL spelling, not C++ spelling", table.nameSpace, "Microsoft.Fixture");
  CHECK_TEXT("table name is independent of the caption", table.name, "Implicit Row");
  CHECK_TEXT("table caption is retained separately", table.caption, "Different table caption");
  CHECK_TEXT("table scope is retained from source", table.scope, "Cloud");
  CHECK_TEXT(
      "table obsolete reason is retained", table.obsoleteReason, "Source-owned reflection fixture");
  CHECK_TEXT("table classification is retained", table.dataClassification, "AccountData");
  CHECK_TRUE("an absent linked-object declaration keeps the documented default",
             !table.linkedObject);
}

}

int main() {
  return gate::Run("GeneratedTableKeys", [] {
    CheckKeys();
    CheckSourceIdentity();
  });
}
