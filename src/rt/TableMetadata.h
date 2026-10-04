#pragma once

namespace agiru {

struct TableDef;

namespace platform {
class TableMetadata_Table;
}

namespace detail {

[[nodiscard]] platform::TableMetadata_Table ProjectTableMetadata(const TableDef &source);

}

}
