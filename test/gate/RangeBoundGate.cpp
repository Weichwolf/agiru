#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Decimal.h"
#include "type/Variant.h"

#include "Check.h"
#include "ResourceCost.h"

#include <string_view>

namespace {

using ResourceCost = agiru::Projects::Resources::Pricing::ResourceCost_Table;

template <typename Read> void MustRaise(std::string_view claim, Read read) {
  bool raised = false;
  try {
    static_cast<void>(read());
  } catch (const agiru::Error &) { raised = true; }
  CHECK_TRUE(claim, raised);
}

template <typename Record> void TypedBounds(Record &record) {
  MustRaise("a fresh numeric field has no minimum",
            [&] { return record.GetRangeMin(record.UnitCost); });
  MustRaise("a fresh text field has no maximum", [&] { return record.GetRangeMax(record.Code); });
  record.SetRange(record.Code, "A");
  MustRaise("another field's filter supplies no minimum",
            [&] { return record.GetRangeMin(record.UnitCost); });
  MustRaise("another field's filter supplies no maximum",
            [&] { return record.GetRangeMax(record.UnitCost); });
  record.SetFilter(record.UnitCost, "1.25..2.50");
  CHECK_TEXT("a numeric minimum retains its Decimal value and scale",
             record.GetRangeMin(record.UnitCost).ToInvariantString(),
             "1.25");
  CHECK_TEXT("a numeric maximum retains its Decimal value and scale",
             record.GetRangeMax(record.UnitCost).ToInvariantString(),
             "2.50");
  record.SetRange(record.UnitCost);
  MustRaise("clearing a field's filter removes its minimum",
            [&] { return record.GetRangeMin(record.UnitCost); });
  MustRaise("clearing a field's filter removes its maximum",
            [&] { return record.GetRangeMax(record.UnitCost); });
  record.SetFilter(record.Code, "''");
  CHECK_TRUE("an explicit blank equality has a blank minimum",
             record.GetRangeMin(record.Code).Value().empty());
  CHECK_TRUE("an explicit blank equality has a blank maximum",
             record.GetRangeMax(record.Code).Value().empty());
}

void FieldBounds(agiru::RecordRef &record) {
  const auto field = record.Field(ResourceCost::Field_No::UnitCost.Value());
  MustRaise("a fresh FieldRef has no minimum", [&] { return field.GetRangeMin(); });
  MustRaise("a fresh FieldRef has no maximum", [&] { return field.GetRangeMax(); });
  field.SetFilter("1.25..2.50");
  CHECK_TEXT("FieldRef minimum is a Decimal with its scale",
             field.GetRangeMin().Get<agiru::Decimal>().ToInvariantString(),
             "1.25");
  CHECK_TEXT("FieldRef maximum is a Decimal with its scale",
             field.GetRangeMax().Get<agiru::Decimal>().ToInvariantString(),
             "2.50");
  field.SetRange();
  MustRaise("clearing a FieldRef filter removes its minimum", [&] { return field.GetRangeMin(); });
  MustRaise("clearing a FieldRef filter removes its maximum", [&] { return field.GetRangeMax(); });
}

}

int main() {
  return gate::Run("RangeBound", [] {
    ResourceCost regular;
    TypedBounds(regular);
    agiru::Temporary<ResourceCost> temporary;
    TypedBounds(temporary);
    agiru::RecordRef borrowed;
    borrowed.GetTable(regular);
    FieldBounds(borrowed);
    agiru::RecordRef owned;
    owned.Open(ResourceCost::kId.Value(), true);
    FieldBounds(owned);
  });
}
