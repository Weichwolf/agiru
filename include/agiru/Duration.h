#pragma once

#include <cstdint>
#include <string>

/// \file
/// \brief AL `Duration`, a span of milliseconds.

namespace agiru {

/// \brief AL `Duration`, a span of milliseconds. \see `duration-data-type.md`
using Duration = std::int64_t;

} // namespace agiru
