#pragma once

#include <stdexcept>

/// \file
/// \brief The base of every error the agiru runtime raises.

namespace agiru {

/// \brief An AL runtime error.
///
/// AL errors are catchable -- `[TryFunction]` and `asserterror` both see them -- and their TEXT is
/// part of intended behaviour: BC test code compares it, and `Assert.ExpectedError` matches
/// substrings of it. One base lets the runtime catch "an AL error" without also catching a
/// `std::bad_alloc`.
///
/// \note Message wording is therefore never paraphrased. Where a message comes from the platform,
///       the source of that wording is cited at the function that raises it.
class Error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace agiru
