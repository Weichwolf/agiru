#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/TableMetadata.h"
#include "runtime/Catalogue.h"

#include "CatalogueNavigation.h"
#include "TableMetadata.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace agiru::detail {

namespace {

using TableRow = platform::TableMetadata_Table;

void LoadKey(void *record, std::size_t index) {
  auto &row = *static_cast<TableRow *>(record);
  row.ID = InstalledTables()[index]->table->id.Value();
}

void Project(void *record, std::size_t index) {
  auto &row = *static_cast<TableRow *>(record);
  row = ProjectTableMetadata(*InstalledTables()[index]->table);
}

struct TableReader {
  TableRow candidate;
  TableRow selected;
  TableRow anchor;

  CatalogueReader Borrow() {
    return {.size = InstalledTables().size(),
            .candidate = &candidate,
            .selected = &selected,
            .anchor = &anchor,
            .key = LoadKey,
            .project = Project};
  }
};

}

bool ScanInstalledTableMetadata(const TableDef &table, const CatalogueScan &scan) {
  if (!IsInstalledTableMetadataProvider(table)) { return false; }
  TableReader rows;
  ScanCatalogue(table, rows.Borrow(), scan);
  return true;
}

std::optional<bool>
FindInstalledTableMetadata(void *record, const TableDef &table, std::string_view which) {
  if (!IsInstalledTableMetadataProvider(table)) { return std::nullopt; }
  TableReader rows;
  return FindCatalogue(record, table, which, rows.Borrow());
}

std::optional<std::int32_t>
NextInstalledTableMetadata(void *record, const TableDef &table, std::int32_t steps) {
  if (!IsInstalledTableMetadataProvider(table)) { return std::nullopt; }
  TableReader rows;
  return NextCatalogue(record, table, steps, rows.Borrow());
}

std::optional<std::int32_t>
CountInstalledTableMetadata(const void *record, const TableDef &table, bool firstOnly) {
  if (!IsInstalledTableMetadataProvider(table)) { return std::nullopt; }
  TableReader rows;
  return CountCatalogue(record, table, firstOnly, rows.Borrow());
}

}
