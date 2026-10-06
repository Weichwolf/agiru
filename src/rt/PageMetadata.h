#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace agiru {

struct PageDef;
struct TableDef;

namespace platform {
class PageMetadata_Table;
}

namespace detail {

struct CatalogueScan;

[[nodiscard]] bool ScanInstalledPageMetadata(const TableDef &table, const CatalogueScan &scan);

[[nodiscard]] platform::PageMetadata_Table ProjectPageMetadata(const PageDef &source);

[[nodiscard]] bool IsInstalledPageMetadataProvider(const TableDef &table);

[[nodiscard]] std::optional<bool> GetInstalledPageMetadata(void *record, const TableDef &table);

[[nodiscard]] std::optional<bool>
FindInstalledPageMetadata(void *record, const TableDef &table, std::string_view which);

[[nodiscard]] std::optional<std::int32_t>
NextInstalledPageMetadata(void *record, const TableDef &table, std::int32_t steps);

[[nodiscard]] std::optional<std::int32_t>
CountInstalledPageMetadata(const void *record, const TableDef &table, bool firstOnly = false);

}

}
