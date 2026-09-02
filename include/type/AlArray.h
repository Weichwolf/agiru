#pragma once

#include "runtime/Error.h"
#include "type/Integer.h"

#include <array>
#include <cstddef>
#include <string>

/// \file
/// \brief AL's `array[N] of T` -- a fixed run of values, indexed from ONE.

namespace agiru {

/// \brief AL `array[N] of T`.
///
/// \tparam T The element type.
/// \tparam N How many, which AL writes in the declaration.
///
/// \note AL HAS NO NAME FOR THIS TYPE, so neither does the name here: `array` is a keyword and not
///       an identifier, and `Array` would read as a type AL declares. What matters is the SHAPE --
///       `A[1]` is the first element, because AL indexes from one and reading `A[0]` is an error
///       rather than the element before it.
///
/// \note THE DIMENSION IS PART OF THE SIGNATURE. `ERMDimensionShortcuts` declares `CreateDimSet`
///       over an `array[6] of Record "Dimension Value"` and again over one record; without the
///       dimension in the type, C++ sees one member declared twice.
template <typename T, std::size_t N> class AlArray {
public:
  /// \brief The element at an AL index.
  /// \param index The ONE-BASED position.
  /// \return The element.
  /// \throws Error when the index is outside 1..N.
  T &operator[](Integer index) { return At(index); }

  /// \brief The element at an AL index.
  /// \param index The ONE-BASED position.
  /// \return The element.
  /// \throws Error when the index is outside 1..N.
  const T &operator[](Integer index) const {
    return const_cast<AlArray *>(this)->At(index);
  }

  /// \brief AL `ArrayLen(A)`.
  /// \return How many elements the declaration gave it.
  [[nodiscard]] static constexpr Integer Length() { return static_cast<Integer>(N); }

private:
  T &At(Integer index) {
    if (index < 1 || static_cast<std::size_t>(index) > N) {
      throw Error("the array index " + std::to_string(index) + " is outside 1.." +
                  std::to_string(N));
    }
    return held_[static_cast<std::size_t>(index) - 1];
  }

  std::array<T, N> held_{};
};

} // namespace agiru
