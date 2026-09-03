#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `SessionInformation` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `SessionInformation`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/sessioninformation/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class SessionInformation {
public:
  /// \brief AL `SessionInformation.AITokensUsed()`. Gets the total amount of AI tokens consumed on
  /// the session, since the session started.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger AITokensUsed();

  /// \brief AL `SessionInformation.Callstack()`. Gets the current callstack.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static std::string Callstack();

  /// \brief AL `SessionInformation.SqlRowsRead()`. Gets the amount of SQL rows read on the session,
  /// since the session started.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger SqlRowsRead();

  /// \brief AL `SessionInformation.SqlStatementsExecuted()`. Gets the amount of SQL statements
  /// executed on the session, since the session started.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger SqlStatementsExecuted();
};

}
