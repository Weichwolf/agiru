#pragma once

#include "agiru/Database.h"
#include "agiru/TableDef.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

using FieldValues = std::vector<std::optional<std::string>>;

[[nodiscard]] std::string_view Required(const std::optional<std::string> &value,
                                        const FieldDef &def);

void InsertRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values);

[[nodiscard]] std::optional<FieldValues> GetRow(const Connection &connection,
                                                const TableDef &table,
                                                std::span<const std::optional<std::string>> key);

bool ModifyRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values);

bool DeleteRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> key);

} // namespace agiru
