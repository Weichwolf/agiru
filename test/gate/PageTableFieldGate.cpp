#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/PageTableField.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <string>
#include <string_view>

namespace {

using Row = agiru::platform::PageTableField;
constexpr const auto &kTable = agiru::TableTraits<Row>::kTable;
constexpr std::array<std::string_view, 15> kNames{"Page ID",
                                                  "Index",
                                                  "Type",
                                                  "Length",
                                                  "Caption",
                                                  "Status",
                                                  "IsTableField",
                                                  "Scope",
                                                  "Tooltip",
                                                  "FieldKind",
                                                  "Name",
                                                  "Field ID",
                                                  "Table No",
                                                  "Description",
                                                  "Table Field Id"};
constexpr std::array<std::string_view, 21> kTypes{
    "TableFilter", "RecordID", "OemText",  "Date",    "Time",   "DateFormula",
    "Decimal",     "Media",    "MediaSet", "Text",    "Code",   "NotSupported_Binary",
    "BLOB",        "Boolean",  "Integer",  "OemCode", "Option", "BigInteger",
    "Duration",    "GUID",     "DateTime"};
constexpr std::array<int, 21> kTypeCodes{4912,  4988,  11519, 11775, 11776, 11797, 12799,
                                         26207, 26208, 31488, 31489, 33791, 33793, 34047,
                                         34559, 35071, 35583, 36095, 36863, 37119, 37375};

static_assert(Row::kId.Value() == 2000000171);
static_assert(kTable.fields.size() == 15 + agiru::kSystemFieldCount);

void DeclarationAndReflection() {
  CHECK_TEXT("original table name", kTable.name, "Page Table Field");
  CHECK_TEXT("OnPrem is availability, not cloud integration", Row::kScope, "OnPrem");
  CHECK_TRUE("original tenant-wide scope", !kTable.dataPerCompany);
  CHECK_TRUE("one primary key", kTable.keys.size() == 1);
  CHECK_TEXT("original key name", kTable.keys.front().name, "pk");
  CHECK_TRUE("primary key is clustered by default", kTable.keys.front().clustered);
  CHECK_TRUE("composite primary key", kTable.keys.front().fields.size() == 2);
  CHECK_TRUE("Page ID precedes Index",
             kTable.keys.front().fields[0].Value() == 1 &&
                 kTable.keys.front().fields[1].Value() == 2);
  CHECK_TRUE("original Brick field order",
             Row::kBrick[0].Value() == 3 && Row::kBrick[1].Value() == 5 &&
                 Row::kBrick[2].Value() == 6 && Row::kBrick[3].Value() == 8);
  const auto *entry = agiru::FindTable(Row::kId);
  CHECK_TRUE("declaration is registered, not just a header", entry != nullptr);
  CHECK_TRUE("registered identity shares immutable declaration",
             entry != nullptr && entry->table == &kTable);
  Row row;
  agiru::RecordRef reflected;
  reflected.GetTable(row);
  for (std::size_t i = 0; i < kNames.size(); ++i) {
    const int number = static_cast<int>(i) + 1;
    const auto *field = agiru::Field(kTable, agiru::FieldNo{number});
    CHECK_TRUE("all original fields are present", field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT("original field name", field->name, kNames[i]);
    CHECK_TEXT("default field caption", field->caption, kNames[i]);
    CHECK_TRUE("reflection retains each source field", reflected.FieldExist(number));
    CHECK_TRUE("no invented calculation", field->fieldClass == agiru::FieldClass::Normal);
    if (field->type == agiru::FieldType::Text) {
      const int length = number == 5 ? 80 : number == 11 ? 256 : 2048;
      CHECK_TRUE("original text limit", field->length == length);
      const std::string value(static_cast<std::size_t>(length), 'x');
      reflected.Field(number).Value(agiru::Variant(value));
      CHECK_TEXT("full declared text survives reflection", reflected.Field(number).ToText(), value);
    }
  }
  reflected.SetTable(row);
  CHECK_TEXT("Caption is a value, never Page.Caption",
             row.FieldFormat(agiru::FieldNo{5}),
             std::string(80, 'x'));
  const auto *type = agiru::Field(kTable, agiru::FieldNo{3});
  CHECK_TRUE("all native Type members retained", type != nullptr && type->values.size() == 21);
  if (type != nullptr) {
    for (std::size_t i = 0; i < type->values.size() && i < kTypes.size(); ++i) {
      CHECK_TEXT("original native Type name", type->values[i].name, kTypes[i]);
      CHECK_TEXT("original native Type caption", type->values[i].caption, kTypes[i]);
      CHECK_TRUE("explicit System native code", type->values[i].ordinal == kTypeCodes[i]);
    }
  }
  for (const int number : {6, 7}) {
    const auto *field = agiru::Field(kTable, agiru::FieldNo{number});
    CHECK_TRUE("obsolete field remains", field != nullptr);
    CHECK_TEXT("Pending is not Removed", field == nullptr ? "?" : field->obsoleteState, "Pending");
    CHECK_TRUE("original obsolete reason retained",
               field != nullptr && !field->obsoleteReason.empty());
  }
  CHECK_TRUE("undeclared number is absent", agiru::Field(kTable, agiru::FieldNo{16}) == nullptr);
}

template <typename Operation> void RefusesProvider(Operation operation) {
  bool refused = false;
  try {
    operation();
  } catch (const agiru::Error &error) {
    const std::string message = error.what();
    refused = message.contains("Page Table Field (2000000171)") &&
              message.contains("live page/table field projection is unavailable");
  }
  CHECK_TRUE("missing live provider refuses before SQL, session use or default success", refused);
}

void LiveProviderCannotPretendToBeEmpty() {
  Row row;
  const auto original = row.SystemId;
  RefusesProvider([&] { agiru::RequireTableProvider(kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeFind(&row, kTable, "-"); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeFindSet(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeNext(&row, kTable, 1); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeCount(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeIsEmpty(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeGet(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeGetBySystemId(&row, kTable, original); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeInsert(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeModify(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeRename(&row, &row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeDelete(&row, kTable); });
  RefusesProvider([&] { (void)agiru::detail::RuntimeDeleteAll(&row, kTable); });
  CHECK_TRUE("refused insertion never stamps the record", row.SystemId == original);
  auto ordinary = kTable;
  ordinary.providerRefusal = {};
  agiru::RequireTableProvider(ordinary);
  CHECK_TRUE("ordinary storage is not refused", ordinary.providerRefusal.empty());
  auto other = kTable;
  other.id = agiru::TableId{50100};
  other.name = "Different Fixture";
  bool generic = false;
  try {
    agiru::RequireTableProvider(other);
  } catch (const agiru::Error &error) {
    generic = std::string(error.what()).contains("Different Fixture (50100)");
  }
  CHECK_TRUE("provider boundary is not object-specific", generic);
}

void TemporaryStorageRemainsIndependent() {
  Row row;
  agiru::detail::RuntimeMakeTemporary(&row, &agiru::kTempOps<Row>);
  row.PageID = 9630;
  row.Index = 1;
  row.Caption = "Declared value";
  CHECK_TRUE("temporary rows are explicit", row.IsTemporary());
  CHECK_TRUE("temporary row insertion", row.Insert());
  row.Caption = "Changed value";
  CHECK_TRUE("temporary row modification", row.Modify());
  CHECK_TRUE("temporary rows can be navigated", row.FindFirst());
  CHECK_TEXT("typed temporary Caption retains value", row.Caption, "Changed value");
  CHECK_TRUE("temporary count uses its own store", row.Count() == 1);
  CHECK_TRUE("temporary rows can be deleted", row.Delete());
  CHECK_TRUE("empty temporary store is a real result", row.IsEmpty());
}

}

int main() {
  return gate::Run("PageTableField", [] {
    DeclarationAndReflection();
    LiveProviderCannotPretendToBeEmpty();
    TemporaryStorageRemainsIndependent();
  });
}
