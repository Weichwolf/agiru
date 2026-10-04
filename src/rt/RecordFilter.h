#pragma once

#include "meta/TableDef.h"

#include "Filter.h"

#include <span>
#include <vector>

namespace agiru::detail {

struct FieldFilter;

class RecordFilter {
public:
  RecordFilter(std::span<const FieldFilter> filters, const TableDef &table);
  [[nodiscard]] bool Matches(const void *row) const;

private:
  struct Bound {
    const FieldDef *field;
    Expression expression;
    bool crossColumn;
  };

  std::vector<Bound> filters_;
};

}
