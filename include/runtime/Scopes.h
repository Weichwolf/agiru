#pragma once

#include "runtime/Error.h"
#include "type/CommitBehavior.h"
#include "type/ErrorBehavior.h"

#include <optional>
#include <string>
#include <vector>

/// \file
/// \brief The behaviours an ATTRIBUTE turns on for the dynamic scope of one method.

namespace agiru {

/// \brief AL `[CommitBehavior(CommitBehavior::X)]` -- what a `Commit` inside this method does.
///
/// \note IT IS A STACK AND NOT A FLAG. Methods nest, and an `Ignore` scope inside an `Error` scope
///       has to restore the outer behaviour on the way out; a flag would leak the inner value to
///       the caller (board:0193).
class CommitScope {
public:
  /// \brief Pushes the behaviour for as long as this object lives.
  /// \param behaviour What a `Commit` inside the scope does.
  explicit CommitScope(::agiru::CommitBehavior behaviour);

  CommitScope(const CommitScope &) = delete;
  CommitScope(CommitScope &&) = delete;
  CommitScope &operator=(const CommitScope &) = delete;
  CommitScope &operator=(CommitScope &&) = delete;

  /// \brief Pops it again, on the way out of the method either way.
  ~CommitScope();

  /// \brief What the innermost scope says a `Commit` does.
  /// \return The behaviour, or nothing when no scope is standing and a `Commit` commits.
  [[nodiscard]] static std::optional<::agiru::CommitBehavior> Standing();
};

/// \brief AL `[ErrorBehavior(ErrorBehavior::Collect)]` -- an error inside this method is COLLECTED
///        rather than raised.
///
/// \note IT DOES NOT TOUCH THE TRANSACTION. `ClearCollectedErrors` does not roll back what was
///       written, so the documented pattern wraps a collecting call in `if Codeunit.Run then`:
///       collecting is about error FLOW, and the write set stays whatever the enclosing boundary
///       decides (board:0195).
class ErrorScope {
public:
  /// \brief Pushes the behaviour for as long as this object lives.
  /// \param behaviour Whether errors are collected inside the scope.
  explicit ErrorScope(::agiru::ErrorBehavior behaviour);

  ErrorScope(const ErrorScope &) = delete;
  ErrorScope(ErrorScope &&) = delete;
  ErrorScope &operator=(const ErrorScope &) = delete;
  ErrorScope &operator=(ErrorScope &&) = delete;

  /// \brief Pops it again, and RAISES ONCE with the aggregation when errors were collected and
  ///        nobody drained them.
  /// \throws Error when the outermost collecting scope ends with errors standing and no exception
  ///         is already on its way out.
  ///
  /// \note IT IS `noexcept(false)`, deliberately. AL's own rule is that an unhandled collection
  ///       stops execution at the end of the scope
  ///       (`attributes/devenv-errorbehavior-attribute.md`), and a destructor that swallowed it
  ///       would turn a refusal into silence.
  ~ErrorScope() noexcept(false);

  /// \brief Whether an error raised right now is collected instead.
  /// \return True inside a collecting scope.
  [[nodiscard]] static bool Collecting();

  /// \brief Collects one error.
  /// \param message What was raised.
  static void Collect(std::string message);

  /// \brief The errors collected so far.
  /// \return The messages, in the order they were collected.
  [[nodiscard]] static const std::vector<std::string> &Collected();

  /// \brief Throws the collected errors away, WITHOUT rolling anything back.
  static void Clear();
};

}
