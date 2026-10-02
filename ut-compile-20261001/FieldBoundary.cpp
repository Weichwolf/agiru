#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/Catalogue.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace {

struct ExpectedField {
  int number;
  std::string_view name;
  agiru::FieldType type;
  int length{};
};

constexpr auto kFields = std::to_array<ExpectedField>({
    {1, "TableNo", agiru::FieldType::Integer},
    {2, "No.", agiru::FieldType::Integer},
    {3, "TableName", agiru::FieldType::Text, 30},
    {4, "FieldName", agiru::FieldType::Text, 30},
    {5, "Type", agiru::FieldType::Option},
    {6, "Len", agiru::FieldType::Integer},
    {7, "Class", agiru::FieldType::Option},
    {8, "Enabled", agiru::FieldType::Boolean},
    {9, "Type Name", agiru::FieldType::Text, 30},
    {10, "ExternalName", agiru::FieldType::Text, 100},
    {20, "Field Caption", agiru::FieldType::Text, 80},
    {21, "RelationTableNo", agiru::FieldType::Integer},
    {22, "RelationFieldNo", agiru::FieldType::Integer},
    {23, "SQLDataType", agiru::FieldType::Option},
    {24, "OptionString", agiru::FieldType::Text, 2047},
    {25, "ObsoleteState", agiru::FieldType::Option},
    {26, "ObsoleteReason", agiru::FieldType::Text, 248},
    {27, "DataClassification", agiru::FieldType::Option},
    {28, "IsPartOfPrimaryKey", agiru::FieldType::Boolean},
    {60, "App Package ID", agiru::FieldType::Guid},
    {61, "App Runtime Package ID", agiru::FieldType::Guid},
    {62, "OptimizeForTextSearch", agiru::FieldType::Boolean},
    {63, "Access", agiru::FieldType::Option},
    {64, "IsAllowedInCustomizations", agiru::FieldType::Boolean},
});

constexpr agiru::TableId kFixtureId{50180};
constexpr std::array<agiru::FieldDef, 3> kFixtureFields{{
    {.name = "Code", .no = agiru::FieldNo{1}, .length = 20, .type = agiru::FieldType::Code},
    {.name = "Text", .no = agiru::FieldNo{2}, .length = 100, .type = agiru::FieldType::Text},
    {.name = "Enumeration", .no = agiru::FieldNo{3}, .type = agiru::FieldType::Enum},
}};
constexpr agiru::TableDef kFixture{
    .id = kFixtureId, .name = "Boundary fixture", .fields = kFixtureFields};
constexpr agiru::TableEntry kEntry{.table = &kFixture,
                                  .make = nullptr,
                                  .free = nullptr,
                                  .validate = nullptr,
                                  .copy = nullptr,
                                  .insert = nullptr,
                                  .modify = nullptr,
                                  .remove = nullptr,
                                  .rename = nullptr};

void CheckBoundary() {
  agiru::platform::Field row;
  agiru::RecordRef ref;
  ref.GetTable(row);
  CHECK_TRUE("all twenty-four source fields remain visible", ref.FieldCount() == 24);
  for (const auto &expected : kFields) {
    const auto *field = agiru::Field(agiru::platform::kFieldTable, agiru::FieldNo{expected.number});
    CHECK_TRUE("every source field has its original declaration",
               field != nullptr && field->name == expected.name &&
                   field->caption == expected.name && field->type == expected.type &&
                   field->length == expected.length);
  }
  CHECK_TRUE("the old invented ExternalName number is absent", !ref.FieldExist(29));
  constexpr std::array<int, 21> codes{
      4912, 4988, 11519, 11775, 11776, 11797, 12799, 26207, 26208, 31488, 31489,
      33791, 33793, 34047, 34559, 35071, 35583, 36095, 36863, 37119, 37375};
  constexpr std::array<std::string_view, 21> names{
      "TableFilter", "RecordID", "OemText", "Date", "Time", "DateFormula", "Decimal", "Media",
      "MediaSet", "Text", "Code", "Binary", "BLOB", "Boolean", "Integer", "OemCode", "Option",
      "BigInteger", "Duration", "GUID", "DateTime"};
  const auto *type = agiru::Field(agiru::platform::kFieldTable, agiru::FieldNo{5});
  CHECK_TRUE("native type inventory is compact", type != nullptr && type->values.size() == 21);
  for (std::size_t i = 0; i < codes.size(); ++i) {
    CHECK_TRUE("native code and name retain source position",
               type != nullptr && i < type->values.size() &&
                   type->values[i].ordinal == codes[i] && type->values[i].name == names[i]);
    row.Type = codes[i];
    CHECK_TRUE("all native codes are declared", row.Type.IsDeclared());
    CHECK_TEXT("native options resolve coded values", row.Type.Name(), names[i]);
  }
  row.Type = 31490;
  CHECK_TRUE("FieldRef Code is not declared by Field.Type", !row.Type.IsDeclared());
  constexpr std::array<int, 3> projected{31489, 31488, 35583};
  for (std::size_t i = 0; i < projected.size(); ++i) {
    CHECK_TRUE("typed Get finds the fixture", row.Get(kFixtureId.Value(), static_cast<int>(i + 1)));
    CHECK_TRUE("typed Get converts internal tags at one native boundary",
               row.Type.AsInteger() == projected[i]);
  }
}

}

int main() {
  return gate::Run("FieldBoundary", [] {
    agiru::RegisterTableEntry(&kEntry);
    CheckBoundary();
  });
}
