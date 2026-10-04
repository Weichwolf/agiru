#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"

namespace agiru {

/// \brief The compile-time ABI declaration specialized by each generated/native table.
/// \tparam T The record class.
template <typename T> struct TableTraits;

namespace detail {

/// \brief Resolves native ABI metadata through the frozen installed catalogue.
/// \param binding A non-null, process-lifetime immutable ABI declaration.
/// \return The source-qualified declaration, or the unchanged unqualified/ordinary binding.
/// \throws std::invalid_argument for null; std::logic_error for an invalid composition.
[[nodiscard]] const TableDef &InstalledTableDefinition(const TableDef *binding);

}

/// \brief Runtime metadata shared by native typed records and RecordRef.
/// \tparam T The record class; its compile-time ABI remains in TableTraits.
/// \return An immutable borrowed declaration, cached once per native class, not per session.
template <typename T> [[nodiscard]] const TableDef &TableDefinition() {
  if constexpr (requires { T::kId; }) {
    if constexpr (!IsPlatformTable(T::kId)) { return TableTraits<T>::kTable; }
  }
  static const TableDef &definition = detail::InstalledTableDefinition(&TableTraits<T>::kTable);
  return definition;
}

}
