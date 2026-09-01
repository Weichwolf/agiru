#pragma once

#include "runtime/Error.h"
#include "type/Integer.h"

#include <array>
#include <cstddef>
#include <string>

/// \file
/// \brief AL indexes from ONE, whatever is being indexed.

namespace agiru {

/// \brief An error raised when an index falls outside what it indexes.
class IndexError : public Error {
public:
  using Error::Error;
};

/// \brief AL `X[i]` -- the i-th element, counting from one.
///
/// \tparam T The element type.
/// \tparam N The declared length of the array.
/// \param  values The array.
/// \param  index  The AL index, 1 to N.
/// \return The element.
/// \throws IndexError when the index falls outside the array.
///
/// ONE FUNCTION FOR EVERY INDEXABLE THING, and the reason is that the generator cannot tell them
/// apart. `Values[1]` is an array element, a List entry or a character of a string depending on how
/// `Values` was declared, and the statement translator does not resolve types -- it resolves names.
/// So it writes `At(Values, 1)` and the OVERLOAD SET decides, which is a C++ compiler's job and not
/// a transpiler's.
///
/// \note The subtraction is the whole point. AL counts from one and C++ from zero, and an emitter
///       that passed the index through would read one element early, silently, everywhere.
template <typename T, std::size_t N>
[[nodiscard]] constexpr T &At(std::array<T, N> &values, Integer index) {
  if (index < 1 || static_cast<std::size_t>(index) > N) {
    throw IndexError("the array index " + std::to_string(index) + " is outside 1.." +
                     std::to_string(N));
  }
  return values[static_cast<std::size_t>(index) - 1];
}

/// \brief AL `X[i]` on a const array.
/// \tparam T The element type.
/// \tparam N The declared length of the array.
/// \param  values The array.
/// \param  index  The AL index, 1 to N.
/// \return The element.
/// \throws IndexError when the index falls outside the array.
template <typename T, std::size_t N>
[[nodiscard]] constexpr const T &At(const std::array<T, N> &values, Integer index) {
  if (index < 1 || static_cast<std::size_t>(index) > N) {
    throw IndexError("the array index " + std::to_string(index) + " is outside 1.." +
                     std::to_string(N));
  }
  return values[static_cast<std::size_t>(index) - 1];
}

} // namespace agiru
