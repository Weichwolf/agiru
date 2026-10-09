#pragma once

#include "meta/TableDef.h"

#include <string>
#include <string_view>

namespace agiru::detail {

inline constexpr std::string_view kRowVersionColumn = "timestamp";
inline constexpr std::string_view kWriteOwnerColumn = "agiru$write_owner_v1";

[[nodiscard]] constexpr std::string_view ColumnName(const FieldDef &field) {
  return field.sqlTimestamp ? kRowVersionColumn : field.name;
}

[[nodiscard]] std::string SqlColumn(const FieldDef &field);

[[nodiscard]] std::string Quoted(std::string_view identifier);

[[nodiscard]] bool HasRowVersion(const TableDef &table);

}
