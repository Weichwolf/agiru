#include "Check.h"
#include "ResourceCost.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "type/Decimal.h"

#include <cstddef>
#include <cstdint>
#include <string>

using agiru::Field;
using agiru::FieldNo;
using agiru::ValueOf;
using agiru::app::ResourceCost;
using agiru::app::ResourceCostCostType;
using agiru::app::ResourceCostType;

namespace {

/// The declaration the class stands on, reached the way the runtime reaches it.
const agiru::TableDef &Table() {
  return agiru::TableTraits<ResourceCost>::kTable;
}

constexpr std::int32_t kTableId = 202;
constexpr std::uint16_t kCodeLength = 20;
constexpr std::uint16_t kWorkTypeCodeLength = 10;
constexpr std::size_t kFieldCount = 6;
constexpr std::size_t kKeyCount = 2;
constexpr std::int32_t kUndeclaredFieldNumber = 99;

void TheTableDeclaresWhatTheAlSourceDeclares() {
  CHECK_TRUE("table 202", Table().id.Value() == kTableId);
  CHECK_TEXT("the AL name, spaces and all", std::string(Table().name), "Resource Cost");
  CHECK_TRUE("six fields", Table().fields.size() == kFieldCount);
  CHECK_TRUE("two keys", Table().keys.size() == kKeyCount);
  CHECK_TEXT("a field name keeps its spaces",
             std::string(Field(Table(), ResourceCost::FieldNumber::WorkTypeCode)->name),
             "Work Type Code");
  CHECK_TRUE("Code[20] carries its length",
             Field(Table(), ResourceCost::FieldNumber::Code)->length == kCodeLength);
  CHECK_TRUE("Code[10] carries its length",
             Field(Table(), ResourceCost::FieldNumber::WorkTypeCode)->length ==
                 kWorkTypeCodeLength);
  CHECK_TRUE("a Decimal carries no length",
             Field(Table(), ResourceCost::FieldNumber::UnitCost)->length == 0);
  CHECK_TRUE("a number nothing declares has no field",
             Field(Table(), FieldNo{kUndeclaredFieldNumber}) == nullptr);
  CHECK_TRUE("the primary key is clustered", Table().keys[0].clustered);
  CHECK_TRUE("and the secondary is not", !Table().keys[1].clustered);
  CHECK_TRUE("Key2 leads with Cost Type",
             Table().keys[1].fields[0] == ResourceCost::FieldNumber::CostType);
}

void TheOffsetsReachTheRealFields() {
  // The field table is how the runtime reaches a field by number, without a virtual call and
  // without a map. A wrong offset writes into the wrong member, so this writes THROUGH the
  // metadata and reads back through the member.
  ResourceCost rec;
  auto *base = reinterpret_cast<std::byte *>(&rec);

  auto *code = reinterpret_cast<decltype(rec.Code) *>(
      base + Field(Table(), ResourceCost::FieldNumber::Code)->offset);
  *code = "ab-1";
  CHECK_TEXT("the offset of field 2 reaches Code", std::string(rec.Code.Value()), "AB-1");

  auto *cost = reinterpret_cast<decltype(rec.UnitCost) *>(
      base + Field(Table(), ResourceCost::FieldNumber::UnitCost)->offset);
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
  // The field table carries the same names, which is what an error message reads, and it reaches
  // them BY ORDINAL rather than by position -- the same call an Enum field answers.
  CHECK_TEXT(
      "and the field table carries them too",
      std::string(ValueOf(Field(Table(), ResourceCost::FieldNumber::CostType)->values, 1)->name),
      "% Extra");
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
  return gate::Run("TargetImage", [] {
    TheTableDeclaresWhatTheAlSourceDeclares();
    TheOffsetsReachTheRealFields();
    TheOptionsCarryTheirAlSpelling();
    TheDefaultRecordIsBlank();
  });
}
