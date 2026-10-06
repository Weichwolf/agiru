#include "CatalogueFlowField.h"

#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Decimal.h"
#include "type/FieldClass.h"

#include "CatalogueNavigation.h"
#include "FieldMetadata.h"
#include "FlowFormula.h"
#include "PageMetadata.h"
#include "TableMetadata.h"

#include <algorithm>
#include <cctype>
#include <compare>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>

namespace agiru::detail {
namespace {

using Buffer = std::unique_ptr<void, void (*)(void *)>;

const TableEntry &Entry(const TableDef &target) {
  const auto *entry = FindTable(target.id);
  if (entry == nullptr || entry->table != &target || entry->make == nullptr ||
      entry->free == nullptr) {
    throw Error("Catalogue FlowField requires an owned native record binding: " +
                std::string(target.name));
  }
  return *entry;
}

const FieldDef &Column(const TableDef &target, const FlowFormula &formula) {
  for (const auto &field : target.fields) {
    if (std::ranges::equal(field.name, formula.field, [](unsigned char a, unsigned char b) {
          return std::tolower(a) == std::tolower(b);
        })) {
      if (IsInstalledFieldProvider(target)) { RequireFieldMetadataProjection(field.no); }
      return field;
    }
  }
  throw Error("Catalogue FlowField reads an undeclared field: " + std::string(target.name) + "." +
              formula.field);
}

std::string Value(void *record, const TableDef &target, const FieldDef &field) {
  if (field.fieldClass == FieldClass::FlowField) {
    CalcField(record, target, reinterpret_cast<StateHandle *>(record)->Peek(), field.no);
  }
  return StorageText(record, field);
}

bool Numeric(const FieldDef &field) {
  return field.type == FieldType::Decimal || field.type == FieldType::Integer ||
         field.type == FieldType::BigInteger || field.type == FieldType::Duration;
}

class Calculation {
public:
  Calculation(const TableDef &target, const FlowFormula &formula, const FieldDef &asked)
      : target_(target), formula_(formula), asked_(asked) {
    Entry(target);
    if (formula.kind == FlowFormula::Kind::Count || formula.kind == FlowFormula::Kind::Exist) {
      return;
    }
    column_ = &Column(target, formula);
    if ((formula.kind == FlowFormula::Kind::Sum || formula.kind == FlowFormula::Kind::Average) &&
        (!Numeric(*column_) || !Numeric(asked))) {
      throw Error("Catalogue Sum/Average requires declared numeric fields");
    }
    if (formula.kind == FlowFormula::Kind::Min || formula.kind == FlowFormula::Kind::Max) {
      if (column_->fieldClass != FieldClass::Normal) {
        throw Error("Catalogue Min/Max requires a stored value column");
      }
      const auto &entry = Entry(target);
      best_ = Buffer(entry.make(), entry.free);
    }
  }

  bool Observe(void *row) {
    ++count_;
    switch (formula_.kind) {
      case FlowFormula::Kind::Count:
        if (count_ > std::numeric_limits<std::int32_t>::max()) {
          throw Error(std::string(target_.name) + " catalogue count exceeds AL Integer");
        }
        return true;
      case FlowFormula::Kind::Exist: return false;
      case FlowFormula::Kind::Lookup: value_ = Value(row, target_, *column_); return false;
      case FlowFormula::Kind::Sum:
      case FlowFormula::Kind::Average:
        total_ += Decimal::FromInvariantString(Value(row, target_, *column_));
        return true;
      case FlowFormula::Kind::Min:
      case FlowFormula::Kind::Max: SelectExtreme(row); return true;
    }
    throw Error("Unknown catalogue FlowField calculation");
  }

  std::string Result() const {
    switch (formula_.kind) {
      case FlowFormula::Kind::Count: return std::to_string(count_);
      case FlowFormula::Kind::Exist: return (count_ != 0) != formula_.reverseSign ? "t" : "f";
      case FlowFormula::Kind::Sum:
      case FlowFormula::Kind::Average: return Total().ToInvariantString();
      case FlowFormula::Kind::Lookup:
      case FlowFormula::Kind::Min:
      case FlowFormula::Kind::Max:
        if (count_ == 0) { return FlowFieldZero(asked_); }
        if (formula_.reverseSign && Numeric(*column_) &&
            formula_.kind != FlowFormula::Kind::Lookup) {
          return (-Decimal::FromInvariantString(value_)).ToInvariantString();
        }
        return value_;
    }
    throw Error("Unknown catalogue FlowField calculation");
  }

private:
  void SelectExtreme(void *row) {
    const auto compared = CompareField(row, best_.get(), *column_);
    if (count_ != 1 && !(formula_.kind == FlowFormula::Kind::Min ? compared < 0 : compared > 0)) {
      return;
    }
    value_ = Value(row, target_, *column_);
    SetFieldText(best_.get(), *column_, value_);
  }

  Decimal Total() const {
    Decimal result = total_;
    if (formula_.kind == FlowFormula::Kind::Average && count_ != 0) {
      result /= Decimal{count_};
      if (asked_.type == FieldType::Integer || asked_.type == FieldType::BigInteger) {
        result = Round(result, Decimal{1});
      }
    }
    return formula_.reverseSign ? -result : result;
  }

  const TableDef &target_;
  const FlowFormula &formula_;
  const FieldDef &asked_;
  const FieldDef *column_ = nullptr;
  Buffer best_{nullptr, nullptr};
  Decimal total_;
  std::int64_t count_ = 0;
  std::string value_;
};

}

std::string FlowFieldZero(const FieldDef &field) {
  switch (field.type) {
    case FieldType::Decimal:
    case FieldType::Integer:
    case FieldType::BigInteger:
    case FieldType::Duration:
    case FieldType::Option:
    case FieldType::Enum: return "0";
    case FieldType::Boolean: return "f";
    default: return {};
  }
}

bool IsCatalogueFlowFieldTarget(const TableDef &table) {
  return IsInstalledFieldProvider(table) || IsInstalledTableMetadataProvider(table) ||
         IsInstalledPageMetadataProvider(table);
}

void CalcCatalogueFlowField(void *record,
                            const FieldDef &asked,
                            const TableDef &target,
                            const FlowFormula &formula,
                            std::span<const ColumnPredicate> filters) {
  Calculation calculation(target, formula, asked);
  for (const auto &filter : filters) {
    const auto *field = Field(target, filter.field);
    if (field != nullptr && field->fieldClass == FieldClass::FlowField) {
      throw Error("Catalogue FlowField predicate requires an unqualified calculated column: " +
                  std::string(field->name));
    }
  }
  const CatalogueScan scan{.filters = filters,
                           .project = formula.kind != FlowFormula::Kind::Count &&
                                      formula.kind != FlowFormula::Kind::Exist,
                           .context = &calculation,
                           .visit = [](void *context, void *row) {
                             return static_cast<Calculation *>(context)->Observe(row);
                           }};
  if (!ScanInstalledFields(target, scan) && !ScanInstalledTableMetadata(target, scan) &&
      !ScanInstalledPageMetadata(target, scan)) {
    throw Error("Catalogue FlowField target has no qualified reader: " + std::string(target.name));
  }
  SetFieldText(record, asked, calculation.Result());
}

}
