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
/// \brief AL `Debugger` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `Debugger`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/debugger/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class Debugger {
public:
  /// \brief AL `Debugger.Activate()`. Activates the debugger and attaches the debugger to the next
  /// session that is started.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Activate();

  /// \brief AL `Debugger.Attach(Integer)`. Activates the debugger and attaches it to the specified
  /// session.
  /// \param SessionID The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Attach(::agiru::Integer SessionID);

  /// \brief AL `Debugger.Break()`. Breaks code execution of a debugging session.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Break();

  /// \brief AL `Debugger.BreakOnError(Boolean)`. Sets whether the debugger breaks on errors.
  /// \param Ok The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean BreakOnError(::agiru::Boolean Ok);

  /// \brief AL `Debugger.BreakOnRecordChanges(Boolean)`. Breaks execution before a change to a
  /// record occurs.
  /// \param Ok The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean BreakOnRecordChanges(::agiru::Boolean Ok);

  /// \brief AL `Debugger.Continue()`. Executes code until the next breakpoint or until execution
  /// ends.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Continue();

  /// \brief AL `Debugger.Deactivate()`. Deactivates the debugger.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Deactivate();

  /// \brief AL `Debugger.DebuggedSessionID()`. Gets the ID of the previous session that the
  /// debugger was attached to.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Integer DebuggedSessionID();

  /// \brief AL `Debugger.DebuggingSessionID()`. Gets the ID of the session that the debugger is
  /// currently attached to.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Integer DebuggingSessionID();

  /// \brief AL `Debugger.EnableSqlTrace(Integer, Boolean)`. Enables or verifies SQL tracing. If you
  /// enable SQL tracing, then SQL Server events for selected sessions on the server instance are
  /// collected.
  /// \param SessionID The AL `Integer`.
  /// \param NewIsEnabled The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean EnableSqlTrace(::agiru::Integer SessionID, ::agiru::Boolean NewIsEnabled);

  /// \brief AL `Debugger.GetLastErrorText()`. Gets the last error that occurred in the debugger.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static std::string GetLastErrorText();

  /// \brief AL `Debugger.IsActive()`. Indicates whether the debugger is active.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsActive();

  /// \brief AL `Debugger.IsAttached()`. Specifies if the debugger is attached to a session.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsAttached();

  /// \brief AL `Debugger.IsBreakpointHit()`. Specifies if a breakpoint is hit in a debugging
  /// session.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsBreakpointHit();

  /// \brief AL `Debugger.SkipSystemTriggers(Boolean)`. Enables the debugger to skip code that is
  /// inside system triggers.
  /// \param Ok The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean SkipSystemTriggers(::agiru::Boolean Ok);

  /// \brief AL `Debugger.StepInto()`. Executes a method call and then stops at the first line of
  /// code inside the method.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean StepInto();

  /// \brief AL `Debugger.StepOut()`. Enables debugging to return to the calling method after it
  /// steps into a method call.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean StepOut();

  /// \brief AL `Debugger.StepOver()`. Executes a method call and then stops at the first line
  /// outside the method call.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean StepOver();

  /// \brief AL `Debugger.Stop()`. Stops execution as if the code hits an error.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Stop();
};

}
