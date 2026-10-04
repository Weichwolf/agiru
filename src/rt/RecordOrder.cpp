#include "RecordOrder.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"

#include <algorithm>
#include <compare>
#include <span>
#include <string>

namespace agiru::detail {

RecordOrder::RecordOrder(const TableDef &table, std::span<const SortField> key, bool ascending) {
  const auto append = [&](FieldNo no, bool direction) {
    const auto field = std::ranges::find(table.fields, no, &FieldDef::no);
    if (field == table.fields.end()) {
      throw Error("the sort path names absent field " + std::to_string(no.Value()));
    }
    columns_.push_back(Column{.field = &*field, .ascending = ascending == direction});
  };
  columns_.reserve(key.size() + (table.keys.empty() ? 0 : table.keys[0].fields.size()));
  for (const SortField &field : key) { append(field.field, field.ascending); }
  if (table.keys.empty()) { return; }
  for (const FieldNo no : table.keys[0].fields) {
    if (std::ranges::none_of(columns_,
                             [no](const Column &column) { return column.field->no == no; })) {
      append(no, true);
    }
  }
}

std::strong_ordering RecordOrder::Compare(const void *left, const void *right) const {
  for (const Column &column : columns_) {
    const std::strong_ordering compared = CompareField(left, right, *column.field);
    if (compared == 0) { continue; }
    if (column.ascending) { return compared; }
    return compared < 0 ? std::strong_ordering::greater : std::strong_ordering::less;
  }
  return std::strong_ordering::equal;
}

}
