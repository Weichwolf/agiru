#pragma once

#include <cstdint>

/// \file
/// \brief AL `PermissionObjectType` -- The different types of objects that can have different
/// permissions assigned.
///
/// The members and their order come from
/// `methods-auto/permissionobjecttype/permissionobjecttype-option.md`, which is the specification:
/// an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `PermissionObjectType`. The different types of objects that can have different
/// permissions assigned.
enum class PermissionObjectType : std::int32_t {
  TableData, ///< The Table Data object type
  Table,     ///< The Table object type
  Report,    ///< The Report object type
  Codeunit,  ///< The Codeunit object type
  XmlPort,   ///< The Xml Port object type
  Page,      ///< The Page object type
  Query,     ///< The Query object type
};

} // namespace agiru
