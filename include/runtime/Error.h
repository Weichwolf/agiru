#pragma once

#include "runtime/Transaction.h"

#include <stdexcept>
#include <string>

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

/// \brief AL `asserterror <statement>` -- the statement is expected to raise.
///
/// \tparam Body The statement, as a callable.
/// \param  body The statement.
/// \throws Error when the statement does NOT raise, because that is what asserterror asserts.
///
/// From the AL test framework: the error is EXPECTED. Instead of propagating, its text is captured
/// where `GetLastErrorText()` reads it and execution carries on with the next statement -- and the
/// write set the statement made is discarded, which is the half a try/catch would not do. A test
/// that asserts an error and then counts rows depends on both.
///
/// \note IT RAISES WHEN NOTHING RAISED. "The statement did not observe an error" is itself a test
///       failure, and a silent pass there would make an asserterror that stopped working invisible.
template <typename Body> void AssertError(Body body) {
  detail::Scope scope;
  try {
    body();
  } catch (const Error &e) {
    scope.Discard(e.what());
    return;
  }
  scope.Keep();
  throw Error("the asserterror statement did not observe an error");
}

/// \brief AL `GetLastErrorText()`.
/// \return The text of the last error a boundary rolled back, or empty.
[[nodiscard]] std::string GetLastErrorText();

/// \brief AL `ClearLastError()`.
void ClearLastError();

/// \brief AL `Commit()` -- everything written so far survives any later rollback.
/// \note It MOVES the enclosing boundaries rather than releasing them; `runtime/Transaction.h`
///       says why, and the predecessor paid for the difference.
void Commit();

} // namespace agiru
