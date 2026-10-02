#pragma once

#include <cstddef>
#include <string_view>

/// \file
/// \brief Why a reader refused, as a VALUE rather than as an unwind.

namespace agiru {

/// \brief Why a reader refused.
///
/// \note IT IS THE OTHER HALF OF `Error`, AND THE TWO ARE NOT INTERCHANGEABLE. An AL `Error`
///       UNWINDS -- it aborts the call chain and rolls back to the enclosing boundary, which is
///       what AL's own `Error()` does and what `asserterror` catches. A `Refusal` is what a reader
///       hands back when the CALLER decides: `Evaluate` answers `false` in AL, and the reason it
///       answered `false` was thrown away until this type carried it (board:0621).
///
/// \note THE TEXT IS A DECLARED LABEL AND NOT A FREE LITERAL, which is why it is a
///       `std::string_view`: it names a case the reader knows about, so it outlives the call and
///       costs no allocation on a path AL takes constantly.
struct Refusal {
  std::string_view what; ///< What was wrong, named.
  std::size_t at = 0;    ///< Where in the input, counting from ONE; 0 when it has no position.
};

}
