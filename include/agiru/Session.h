#pragma once

#include "agiru/Database.h"
#include "agiru/Error.h"

#include <string>

/// \file
/// \brief The ambient session an AL record belongs to.

namespace agiru {

/// \brief An error raised when there is no session to work in.
class SessionError : public Error {
public:
  using Error::Error;
};

/// \brief One AL session: a database connection and the state that belongs to it.
///
/// AL CODE NEVER NAMES A SESSION AND NEVER NAMES A CONNECTION. `Rec.Insert()` takes no argument and
/// no BaseApp line mentions where the row goes. So the session is ambient: the host opens one, and
/// every record operation inside it finds it without being handed it.
///
/// The current session is thread-local, which is what a server with many sessions needs and what
/// the predecessor arrived at the hard way -- openerp rebuilt its whole session model onto
/// `ContextVar` for exactly this reason, after fork-based isolation proved unaffordable.
///
/// \note Constructing a session makes it current and destroying it restores the previous one, so
///       nesting works and nothing has to be unwound by hand.
class Session {
public:
  /// \brief Opens a session on a database.
  /// \param connectionInfo A libpq connection string or URI.
  /// \throws DatabaseError when the connection cannot be established.
  explicit Session(const std::string &connectionInfo);

  ~Session();

  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;
  Session(Session &&) = delete;
  Session &operator=(Session &&) = delete;

  /// \brief The session this thread is working in.
  /// \return The current session.
  /// \throws SessionError when no session is open, which is a defect in the host rather than in AL
  ///         code: an AL statement can only run inside one.
  [[nodiscard]] static Session &Current();

  /// \return True when this thread has a session open.
  [[nodiscard]] static bool HasCurrent();

  /// \return The session's database connection.
  [[nodiscard]] const Connection &Database() const { return connection_; }

private:
  Connection connection_;
  Session *previous_;
};

} // namespace agiru
