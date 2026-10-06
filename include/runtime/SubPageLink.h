#pragma once

#include <string_view>

namespace agiru {

struct TableDef;

namespace detail {

/// \brief Applies a SubPageLink's field, constant and filter clauses to the subpage record.
/// \param sub The subpage record.
/// \param subTable Its declaration.
/// \param parent The current parent record.
/// \param parentTable Its declaration.
/// \param link The declared property text.
/// \throws Error when a clause names an unknown field, table or clause kind.
void ApplySubPageLink(void *sub,
                      const TableDef &subTable,
                      const void *parent,
                      const TableDef &parentTable,
                      std::string_view link);

}
}
