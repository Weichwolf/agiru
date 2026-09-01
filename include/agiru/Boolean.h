#pragma once

#include <cstdint>
#include <string>

/// \file
/// \brief AL `Boolean`.

namespace agiru {

/// \brief AL `Boolean`. \see `boolean-data-type.md`
using Boolean = bool;

/// \brief AL `Boolean.ToText()`.
///
/// \param value The truth value.
/// \return `Yes` or `No`, which is what AL's own `Format` yields -- not `true` or `false`. The
///         predecessor records this as a measured failure: a test that rebuilt an expected error
///         through `Format(false)` looked for `No` in a message that said `False`.
/// \see `boolean-totext--method.md`
[[nodiscard]] std::string ToText(Boolean value);

} // namespace agiru
