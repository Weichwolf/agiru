#include "RecordFilter.h"

#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "type/FieldClass.h"

#include "Filter.h"

#include <span>
#include <string>

namespace agiru::detail {

RecordFilter::RecordFilter(std::span<const FieldFilter> filters, const TableDef &table) {
  filters_.reserve(filters.size());
  for (const FieldFilter &filter : filters) {
    const FieldDef *field = Field(table, filter.field);
    if (field == nullptr) {
      throw Error("the table carries no field " + std::to_string(filter.field.Value()));
    }
    if (field->fieldClass == FieldClass::FlowFilter) { continue; }
    filters_.push_back(Bound{.field = field,
                             .expression = ParseFilter(filter.text),
                             .crossColumn = filter.group == kCrossColumnGroup});
  }
}

bool RecordFilter::Matches(const void *row) const {
  bool crossColumn = false;
  bool crossColumnHit = false;
  for (const Bound &filter : filters_) {
    const FieldDef &field = *filter.field;
    const std::string value =
        field.type == FieldType::RecordId || field.type == FieldType::DateFormula
            ? StorageText(row, field)
            : FieldText(row, field);
    const bool passes = detail::Matches(filter.expression, value, field);
    if (filter.crossColumn) {
      crossColumn = true;
      crossColumnHit = crossColumnHit || passes;
      continue;
    }
    if (!passes) { return false; }
  }
  return !crossColumn || crossColumnHit;
}

}
