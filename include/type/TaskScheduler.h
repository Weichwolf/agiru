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
/// \brief AL `TaskScheduler` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `TaskScheduler`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/taskscheduler/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class TaskScheduler {
public:
  /// \brief AL `TaskScheduler.CancelTask(Guid)`. Cancels and deletes a scheduled task that runs a
  /// specific codeunit.
  /// \param Task The AL `Guid`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean CancelTask(::agiru::Guid Task);

  /// \brief AL `TaskScheduler.CanCreateTask()`. Checks whether it is possible to schedule tasks in
  /// this session (depends on the user/app entitlements).
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean CanCreateTask();

  /// \brief AL `TaskScheduler.CreateTask(Integer, Integer, Boolean, Text, DateTime, RecordId,
  /// Duration)`. Adds a task to be run by the task scheduler. The task is ensured to not run before
  /// the specified time. This method also allows you to set a timeout for the task.
  /// \param CodeunitId The AL `Integer`.
  /// \param FailureCodeunitId The AL `Integer`.
  /// \param IsReady The AL `Boolean`.
  /// \param Company The AL `Text`.
  /// \param NotBefore The AL `DateTime`.
  /// \param RecordID The AL `RecordId`.
  /// \param Timeout The AL `Duration`.
  /// \return The AL `Guid`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Guid CreateTask(::agiru::Integer CodeunitId,
                                  ::agiru::Integer FailureCodeunitId,
                                  ::agiru::Boolean IsReady,
                                  std::string_view Company,
                                  ::agiru::DateTime NotBefore,
                                  ::agiru::RecordId RecordID,
                                  ::agiru::Duration Timeout);

  /// \brief AL `TaskScheduler.CreateTask(Integer, Integer, Boolean, Text, DateTime, RecordId)`.
  /// Adds a task to be run by the task scheduler. The task is ensured to not run before the
  /// specified time.
  /// \param CodeunitId The AL `Integer`.
  /// \param FailureCodeunitId The AL `Integer`.
  /// \param IsReady The AL `Boolean`.
  /// \param Company The AL `Text`.
  /// \param NotBefore The AL `DateTime`.
  /// \param RecordID The AL `RecordId`.
  /// \return The AL `Guid`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Guid CreateTask(::agiru::Integer CodeunitId,
                                  ::agiru::Integer FailureCodeunitId,
                                  ::agiru::Boolean IsReady,
                                  std::string_view Company,
                                  ::agiru::DateTime NotBefore,
                                  ::agiru::RecordId RecordID);

  /// \brief AL `TaskScheduler.SetTaskReady(Guid, DateTime)`. Sets a task that runs a codeunit to
  /// the ready state. The task will not run unless it is in the ready state.
  /// \param Task The AL `Guid`.
  /// \param NotBefore The AL `DateTime`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean SetTaskReady(::agiru::Guid Task, ::agiru::DateTime NotBefore);

  /// \brief AL `TaskScheduler.TaskExists(Guid)`. Checks whether a specific task exists.
  /// \param Task The AL `Guid`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean TaskExists(::agiru::Guid Task);
};

}
