#pragma once

#include <cstdint>

/// \file
/// \brief AL `FieldClass` -- Represents the type of a field class.
///
/// The members and their order come from `methods-auto/fieldclass/fieldclass-option.md`, which is
/// the specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `FieldClass`. Represents the type of a field class.
enum class FieldClass : std::int32_t {
  Normal,     ///< A normal field.
  FlowField,  ///< A flow field.
  FlowFilter, ///< A flow filter.
};

} // namespace agiru
