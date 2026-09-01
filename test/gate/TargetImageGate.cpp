#include "agiru/Decimal.h"
#include "agiru/Ids.h"
#include "agiru/TableDef.h"
#include "agiru/Text.h"

#include "Check.h"
#include "ResourceCost.h"

#include <cstddef>
#include <cstdint>
#include <string>

using agiru::Field;
using agiru::FieldNo;
using agiru::app::kResourceCostTable;
using agiru::app::ResourceCost;
using agiru::app::ResourceCostCostType;
using agiru::app::ResourceCostType;

namespace {

// The numbers table 202 declares, named so that a wrong one reads as a wrong claim rather than as
// a stray literal.
constexpr std::int32_t kTableId = 202;
constexpr std::uint16_t kCodeLength = 20;
constexpr std::uint16_t kWorkTypeCodeLength = 10;
constexpr std::size_t kFieldCount = 6;
constexpr std::size_t kKeyCount = 2;
constexpr std::int32_t kUndeclaredFieldNumber = 99;

void TheTableDeclaresWhatTheAlSourceDeclares() {
  CHECK_TRUE("table 202", kResourceCostTable.id.Value() == kTableId);
  CHECK_TEXT("the AL name, spaces and all", std::string(kResourceCostTable.name), "Resource Cost");
  CHECK_TRUE("six fields", kResourceCostTable.fields.size() == kFieldCount);
  CHECK_TRUE("two keys", kResourceCostTable.keys.size() == kKeyCount);
  CHECK_TEXT("a field name keeps its spaces",
             std::string(Field(kResourceCostTable, ResourceCost::FieldNumber::WorkTypeCode)->name),
             "Work Type Code");
  CHECK_TRUE("Code[20] carries its length",
             Field(kResourceCostTable, ResourceCost::FieldNumber::Code)->length == kCodeLength);
  CHECK_TRUE("Code[10] carries its length",
             Field(kResourceCostTable, ResourceCost::FieldNumber::WorkTypeCode)->length ==
                 kWorkTypeCodeLength);
  CHECK_TRUE("a Decimal carries no length",
             Field(kResourceCostTable, ResourceCost::FieldNumber::UnitCost)->length == 0);
  CHECK_TRUE("a number nothing declares has no field",
             Field(kResourceCostTable, FieldNo{kUndeclaredFieldNumber}) == nullptr);
  CHECK_TRUE("the primary key is clustered", kResourceCostTable.keys[0].clustered);
  CHECK_TRUE("and the secondary is not", !kResourceCostTable.keys[1].clustered);
  CHECK_TRUE("Key2 leads with Cost Type",
             kResourceCostTable.keys[1].fields[0] == ResourceCost::FieldNumber::CostType);
}

void TheOffsetsReachTheRealFields() {
  // The field table is how RecordRef will reach a field by number, without a virtual call and
  // without a map. A wrong offset writes into the wrong member, so this writes THROUGH the
  // metadata and reads back through the member.
  ResourceCost rec;
  auto *base = reinterpret_cast<std::byte *>(&rec);

  auto *code = reinterpret_cast<decltype(rec.Code) *>(
      base + Field(kResourceCostTable, ResourceCost::FieldNumber::Code)->offset);
  *code = "ab-1";
  CHECK_TEXT("the offset of field 2 reaches Code", std::string(rec.Code.Value()), "AB-1");

  auto *cost = reinterpret_cast<decltype(rec.UnitCost) *>(
      base + Field(kResourceCostTable, ResourceCost::FieldNumber::UnitCost)->offset);
  *cost = agiru::Decimal::FromInvariantString("12.50");
  CHECK_TEXT("the offset of field 6 reaches Unit Cost", rec.UnitCost.ToInvariantString(), "12.50");
}

void TheOptionsCarryTheirAlSpelling() {
  ResourceCost rec;
  rec.Type = ResourceCostType::GroupResource;
  CHECK_TEXT("an option member that is no identifier keeps its AL name",
             std::string(rec.Type.Name()),
             "Group(Resource)");
  rec.CostType = ResourceCostCostType::PercentExtra;
  CHECK_TEXT("and so does '% Extra'", std::string(rec.CostType.Name()), "% Extra");
}

void TheDefaultRecordIsBlank() {
  const ResourceCost rec;
  CHECK_TRUE("a code starts empty", rec.Code.IsEmpty());
  CHECK_TRUE("a decimal starts at zero", rec.UnitCost.IsZero());
  CHECK_TRUE("an option starts at its first member", rec.Type.AsInteger() == 0);
  CHECK_TRUE("and so does the second one", rec.CostType.AsInteger() == 0);
}

} // namespace

int main() {
  TheTableDeclaresWhatTheAlSourceDeclares();
  TheOffsetsReachTheRealFields();
  TheOptionsCarryTheirAlSpelling();
  TheDefaultRecordIsBlank();
  return gate::Done("TargetImage");
}
