#pragma once

/// \file
/// \brief One exclusive command over persistent AL state and a borrowed SQL connection.

namespace agiru {

class Connection;
class Session;

/// \brief Activates a detached session on this worker and owns its root transaction boundary.
/// The host supplies an exclusively borrowed, idle connection and already authenticated
/// session, both alive until the command ends. No thread or connection is reserved while
/// the session is idle. Authorization, credentials, receipts and pool admission stay with
/// the host. The lease provider must reset nontransactional PostgreSQL settings before reuse.
class SessionCommand {
public:
  /// \brief Rechecks the User account and activates its persistent AL state on this worker.
  /// \param session Detached session; simultaneous or nested activation of it refuses.
  /// \param connection Exclusively borrowed, healthy connection with no existing transaction.
  /// \throws Error for an inactive account, active session, dirty connection or SQL failure.
  SessionCommand(Session &session, Connection &connection);

  /// \brief Rolls back unfinished work, invalidates cursors and restores the previous context.
  /// Explicit production Commit remains durable. Failed SQL cleanup closes the connection
  /// and emits a diagnostic; it cannot return a contaminated lease to a pool.
  ~SessionCommand() noexcept;

  SessionCommand(const SessionCommand &) = delete;
  SessionCommand &operator=(const SessionCommand &) = delete;
  SessionCommand(SessionCommand &&) = delete;
  SessionCommand &operator=(SessionCommand &&) = delete;

  /// \brief Ends a successful command, committing its updates and detaching immediately.
  /// \throws Error for live nested boundaries, inconsistency or commit failure.
  /// Failed Keep still unwinds through rollback; do not report it as a successful command.
  /// Calling Keep after successful completion refuses; execution must not continue afterward.
  void Keep();

private:
  struct Unvalidated {};

  SessionCommand(Session &session,
                 Connection &connection,
                 [[maybe_unused]] Unvalidated unvalidated);
  void Detach() noexcept;

  Session *session_;
  Connection *connection_;
};

}
