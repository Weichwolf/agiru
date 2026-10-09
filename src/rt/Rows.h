#pragma once

#include "meta/TableDef.h"
#include "runtime/Database.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

using FieldValues = std::vector<std::optional<std::string>>;

[[nodiscard]] std::string_view Required(const std::optional<std::string> &value,
                                        const FieldDef &def);

[[nodiscard]] std::optional<FieldValues>
InsertRow(const Connection &connection,
          const TableDef &table,
          std::span<const std::optional<std::string>> values);

[[nodiscard]] std::optional<FieldValues> GetRow(const Connection &connection,
                                                const TableDef &table,
                                                std::span<const std::optional<std::string>> key);

[[nodiscard]] std::optional<FieldValues> GetRowWhere(const Connection &connection,
                                                     const TableDef &table,
                                                     const FieldDef &column,
                                                     std::string_view value);

[[nodiscard]] bool PlatformOwned(const FieldDef &field);

[[nodiscard]] std::optional<FieldValues>
ModifyRow(const Connection &connection,
          const TableDef &table,
          std::span<const std::optional<std::string>> values,
          std::int64_t observedVersion);

[[nodiscard]] std::optional<FieldValues>
RenameRow(const Connection &connection,
          const TableDef &table,
          std::span<const std::optional<std::string>> values,
          std::span<const std::optional<std::string>> oldKey,
          std::int64_t observedVersion);

bool DeleteRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> key,
               std::int64_t observedVersion);

namespace detail {
void ObserveSqlVersion(void *record, const FieldDef &field, std::string_view value);
}

}
