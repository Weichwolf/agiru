#pragma once

#include <span>

namespace agiru {
struct FieldDef;
struct TableDef;

namespace detail {
struct ColumnPredicate;
struct FlowFormula;

[[nodiscard]] bool IsCatalogueFlowFieldTarget(const TableDef &table);

void CalcCatalogueFlowField(void *record,
                            const FieldDef &asked,
                            const TableDef &target,
                            const FlowFormula &formula,
                            std::span<const ColumnPredicate> filters);
}
}
