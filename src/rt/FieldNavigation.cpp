#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"

#include "CatalogueNavigation.h"
#include "FieldMetadata.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

using FieldRow = platform::Field;

struct LocatedField {
  const TableDef *table;
  const FieldDef *field;
};

auto Key(const LocatedField &row) {
  return std::pair{row.table->id.Value(), row.field->no.Value()};
}

std::span<const LocatedField> InstalledFields() {
  static const auto fields = [] {
    std::vector<LocatedField> result;
    for (const auto *entry : InstalledTables()) {
      for (const auto &field : entry->table->fields) {
        if (field.no.Value() <= 0) { continue; }
        result.push_back({.table = entry->table, .field = &field});
      }
    }
    std::ranges::sort(result,
                      [](const auto &left, const auto &right) { return Key(left) < Key(right); });
    for (std::size_t i = 1; i < result.size(); ++i) {
      if (Key(result[i - 1]) == Key(result[i])) {
        throw Error("Field catalogue contains a duplicate installed field key");
      }
    }
    return result;
  }();
  return fields;
}

void RequireProjected(FieldNo no) {
  RequireFieldMetadataProjection(no);
}

void LoadKey(void *record, std::size_t index) {
  const auto &located = InstalledFields()[index];
  auto &row = *static_cast<FieldRow *>(record);
  row.TableNo = located.table->id.Value();
  row.No = located.field->no.Value();
}

void Project(void *record, std::size_t index) {
  const auto &located = InstalledFields()[index];
  auto &row = *static_cast<FieldRow *>(record);
  row = FieldRow{};
  LoadFieldMetadata(row, *located.table, *located.field);
}

struct FieldReader {
  FieldRow candidate;
  FieldRow selected;
  FieldRow anchor;

  CatalogueReader Borrow() {
    return {.size = InstalledFields().size(),
            .candidate = &candidate,
            .selected = &selected,
            .anchor = &anchor,
            .key = LoadKey,
            .project = Project,
            .require = RequireProjected};
  }
};

}

void RequireFieldMetadataProjection(FieldNo no) {
  using No = FieldRow::Field_No;
  if (no == No::DataClassification || no == No::SQLDataType || no == No::AppPackageID ||
      no == No::AppRuntimePackageID) {
    throw Error("Field catalogue filter/order requires an unqualified metadata attribute: " +
                std::to_string(no.Value()));
  }
}

bool ScanInstalledFields(const TableDef &table, const CatalogueScan &scan) {
  if (!IsInstalledFieldProvider(table)) { return false; }
  FieldReader rows;
  ScanCatalogue(table, rows.Borrow(), scan);
  return true;
}

std::optional<bool>
FindInstalledFields(void *record, const TableDef &table, std::string_view which) {
  if (!IsInstalledFieldProvider(table)) { return std::nullopt; }
  FieldReader rows;
  return FindCatalogue(record, table, which, rows.Borrow());
}

std::optional<std::int32_t>
NextInstalledField(void *record, const TableDef &table, std::int32_t steps) {
  if (!IsInstalledFieldProvider(table)) { return std::nullopt; }
  FieldReader rows;
  return NextCatalogue(record, table, steps, rows.Borrow());
}

std::optional<std::int32_t>
CountInstalledFields(const void *record, const TableDef &table, bool firstOnly) {
  if (!IsInstalledFieldProvider(table)) { return std::nullopt; }
  FieldReader rows;
  return CountCatalogue(record, table, firstOnly, rows.Borrow());
}

}
