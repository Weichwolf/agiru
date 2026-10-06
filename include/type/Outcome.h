#pragma once

#include <expected>
#include <utility>

namespace agiru {

/// \brief A value or an owned error, using the standard C++23 expected type unchanged.
/// \tparam Value The successful value. \tparam Error The refusal value.
template <typename Value, typename Error> using Outcome = std::expected<Value, Error>;

/// \brief Constructs an owned error alternative without unwinding.
/// \tparam Error The incoming error type. \param error The error to copy or move.
/// \return The standard unexpected value, with its usual deduction and ownership.
template <typename Error>
[[nodiscard]] constexpr auto
Failed(Error &&error) noexcept(noexcept(std::unexpected(std::forward<Error>(error)))) {
  return std::unexpected(std::forward<Error>(error));
}

}
