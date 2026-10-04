#pragma once

#include "meta/TableDef.h"
#include "runtime/RecordState.h"

#include <compare>
#include <span>
#include <vector>

namespace agiru::detail {

class RecordOrder {
public:
  struct Column {
    const FieldDef *field;
    bool ascending;
  };

  RecordOrder(const TableDef &table, std::span<const SortField> key, bool ascending = true);

  [[nodiscard]] std::span<const Column> Columns() const { return columns_; }

  [[nodiscard]] std::strong_ordering Compare(const void *left, const void *right) const;

private:
  std::vector<Column> columns_;
};

}
