#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief AL's transaction boundary -- what an error rolls back to, and what `Commit` moves.

namespace agiru {

class Connection;

/// \brief The nested boundaries a session is inside, innermost last.
///
/// AL DOES NOT CATCH ERRORS, IT ROLLS THEM BACK. `Error()` does not unwind to a handler that
/// decides what to do; it abandons the write set and returns control to whoever opened the
/// boundary. `Codeunit.Run` opens one and reports `false`; `asserterror` opens one and expects it
/// to be used. That is why `IF NOT CODEUNIT.RUN(...) THEN` is AL's idiom for "try this" and why a
/// test can assert an error and then count rows.
///
/// Each boundary is a PostgreSQL `SAVEPOINT` on the session's own pinned connection -- the same
/// connection the statements run on, because a savepoint taken on another one rolls back nothing
/// (board:0012).
class Boundaries {
public:
  /// \brief Opens a boundary.
  /// \param connection The session's connection.
  /// \return The new depth, which the closer must be handed back.
  /// \throws DatabaseError when the savepoint cannot be taken.
  std::size_t Open(const Connection &connection);

  /// \brief Closes a boundary, keeping everything written inside it.
  /// \param connection The session's connection.
  /// \param depth      The depth Open() returned.
  void Release(const Connection &connection, std::size_t depth);

  /// \brief Closes a boundary, discarding everything written inside it.
  /// \param connection The session's connection.
  /// \param depth      The depth Open() returned.
  void Rollback(const Connection &connection, std::size_t depth);

  /// \brief AL `Commit()` -- everything written so far survives any later rollback.
  ///
  /// \param connection The session's connection.
  ///
  /// \warning A COMMIT DOES NOT RELEASE THE ENCLOSING BOUNDARY, IT MOVES IT. Releasing it would
  ///          leave the block with nothing to roll back to, so everything written AFTER an inner
  ///          `Commit` would survive an error that should have discarded it -- the predecessor
  ///          records exactly that defect and the reverse of it. Every open savepoint is therefore
  ///          released and retaken HERE, at the commit point, so a later error rolls back to the
  ///          commit and no further.
  void Commit(const Connection &connection);

  /// \return How many boundaries are open.
  [[nodiscard]] std::size_t Depth() const { return names_.size(); }

  /// \brief The message of the last error a boundary rolled back, for AL `GetLastErrorText()`.
  /// \return The text, or empty when nothing has failed in this session.
  [[nodiscard]] std::string_view LastError() const { return lastError_; }

  /// \brief Records the message a boundary is rolling back.
  /// \param text The error's text.
  void SetLastError(std::string text) { lastError_ = std::move(text); }

  /// \brief AL `ClearLastError()`.
  void ClearLastError() { lastError_.clear(); }

private:
  std::vector<std::string> names_;
  std::string lastError_;
  std::size_t issued_ = 0;
};

}

/// \brief The platform half of a transaction boundary. Not part of the door's vocabulary.
namespace agiru::detail {

/// \brief One transaction boundary, opened on the current session and closed by whichever way the
///        block leaves.
///
/// AL DOES NOT CATCH ERRORS, IT ROLLS THEM BACK, so this is not a try/catch with a different name.
/// Nothing here decides what an error MEANS; it decides what happens to the write set, and the
/// caller reports `false` or captures the text.
class Scope {
public:
  Scope();
  ~Scope();

  Scope(const Scope &) = delete;
  Scope(Scope &&) = delete;
  Scope &operator=(const Scope &) = delete;
  Scope &operator=(Scope &&) = delete;

  /// \brief Keeps everything written inside.
  void Keep();

  /// \brief Discards everything written inside, and remembers why.
  /// \param why The error's text, which AL `GetLastErrorText()` returns afterwards.
  void Discard(std::string_view why);

private:
  std::size_t depth_;
  bool open_ = true;
};

}
