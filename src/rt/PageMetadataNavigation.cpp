#include "meta/PageDef.h"
#include "platform/PageMetadata.h"
#include "runtime/Catalogue.h"

#include "CatalogueNavigation.h"
#include "PageMetadata.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace agiru::detail {

namespace {

using Row = platform::PageMetadata_Table;

void LoadKey(void *record, std::size_t index) {
  static_cast<Row *>(record)->ID = InstalledPages()[index]->page->id.Value();
}

void Project(void *record, std::size_t index) {
  *static_cast<Row *>(record) = ProjectPageMetadata(*InstalledPages()[index]->page);
}

struct PageReader {
  Row candidate;
  Row selected;
  Row anchor;

  CatalogueReader Borrow() {
    return {.size = InstalledPages().size(),
            .candidate = &candidate,
            .selected = &selected,
            .anchor = &anchor,
            .key = LoadKey,
            .project = Project};
  }
};

}

std::optional<bool>
FindInstalledPageMetadata(void *record, const TableDef &table, std::string_view which) {
  if (!IsInstalledPageMetadataProvider(table)) { return std::nullopt; }
  PageReader rows;
  return FindCatalogue(record, table, which, rows.Borrow());
}

std::optional<std::int32_t>
NextInstalledPageMetadata(void *record, const TableDef &table, std::int32_t steps) {
  if (!IsInstalledPageMetadataProvider(table)) { return std::nullopt; }
  PageReader rows;
  return NextCatalogue(record, table, steps, rows.Borrow());
}

std::optional<std::int32_t>
CountInstalledPageMetadata(const void *record, const TableDef &table, bool firstOnly) {
  if (!IsInstalledPageMetadataProvider(table)) { return std::nullopt; }
  PageReader rows;
  return CountCatalogue(record, table, firstOnly, rows.Borrow());
}

}
