#pragma once

#include <cstdint>

/// \file
/// \brief AL `ObjectType` -- The different types of objects.
///
/// The members and their order come from `methods-auto/objecttype/objecttype-option.md`, which is
/// the specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `ObjectType`. The different types of objects.
enum class ObjectType : std::int32_t {
  Codeunit,  ///< The Codeunit object type
  MenuSuite, ///< The Menusuite object type
  Page,      ///< The Page object type
  Query,     ///< The Query object type
  Report,    ///< The Report object type
  Table,     ///< The Table object type
  XmlPort,   ///< The XMLPort object type
};

} // namespace agiru
