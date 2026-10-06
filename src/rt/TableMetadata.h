#pragma once

#include "meta/Ids.h"

#include <cstdint>
#include <optional>
#include <string_view>

namespace agiru {

struct TableDef;

namespace platform {
class TableMetadata_Table;
}

namespace detail {

struct CatalogueScan;

[[nodiscard]] bool ScanInstalledTableMetadata(const TableDef &table, const CatalogueScan &scan);

[[nodiscard]] platform::TableMetadata_Table ProjectTableMetadata(const TableDef &source);

[[nodiscard]] std::optional<platform::TableMetadata_Table> InstalledTableMetadata(TableId id);

[[nodiscard]] std::optional<bool> GetInstalledTableMetadata(void *record, const TableDef &table);

[[nodiscard]] bool IsInstalledTableMetadataProvider(const TableDef &table);

[[nodiscard]] std::optional<bool>
FindInstalledTableMetadata(void *record, const TableDef &table, std::string_view which);

[[nodiscard]] std::optional<std::int32_t>
NextInstalledTableMetadata(void *record, const TableDef &table, std::int32_t steps);

[[nodiscard]] std::optional<std::int32_t>
CountInstalledTableMetadata(const void *record, const TableDef &table, bool firstOnly = false);

}

}
