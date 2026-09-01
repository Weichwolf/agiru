#pragma once

#include <stdexcept>

namespace agiru {

/// The base of every error this runtime raises on its own behalf.
///
/// AL errors are catchable (`[TryFunction]`, `asserterror`) and their TEXT is part of intended
/// behaviour -- tests compare it, `Assert.ExpectedError` matches substrings of it. One base means
/// the runtime can catch "an AL error" without catching a `std::bad_alloc` alongside it.
class Error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace agiru
