#include "dotnet/DataTable.h"

#include <cctype>
#include <string>
#include <string_view>

namespace agiru::dotnet {

bool detail::SameColumnName(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

std::size_t DataRow::Columns::IndexOfOrRefuse(std::string_view name) const {
  for (std::size_t i = 0; i < columns->size(); ++i) {
    if (detail::SameColumnName((*columns)[i].ColumnName(), name)) { return i; }
  }
  throw Error("DataRow: there is no column '" + std::string(name) + "'");
}

}
