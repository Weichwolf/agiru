#pragma once

#include "meta/Ids.h"

#include <optional>

namespace agiru {

struct TableDef;

namespace platform {
class TableMetadata_Table;
}

namespace detail {

[[nodiscard]] platform::TableMetadata_Table ProjectTableMetadata(const TableDef &source);

[[nodiscard]] std::optional<platform::TableMetadata_Table> InstalledTableMetadata(TableId id);

}

}
