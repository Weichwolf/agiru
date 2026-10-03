#include "meta/Ids.h"
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

}

int main() {
  return gate::Run("GeneratedTableKeys", CheckKeys);
}
