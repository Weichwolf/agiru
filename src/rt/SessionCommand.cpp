#include "runtime/SessionCommand.h"

#include "runtime/Database.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include <cstdio>
#include <exception>

namespace agiru {

SessionCommand::SessionCommand(Session &session,
                               Connection &connection,
                               [[maybe_unused]] Unvalidated unvalidated)
    : session_(&session), connection_(&connection) {
  session.Attach(connection);
}

SessionCommand::SessionCommand(Session &session, Connection &connection)
    : SessionCommand(session, connection, Unvalidated{}) {
  static_cast<void>(session.boundaries_.Open(connection));
  session.ResolveUser(session.userSecurityId_);
}

SessionCommand::~SessionCommand() noexcept {
  if (session_ == nullptr) { return; }
  try {
    if (connection_->IsOpen() && connection_->InTransaction()) { connection_->Run("ROLLBACK"); }
  } catch (const std::exception &error) {
    std::fputs("agiru: session command rollback failed: ", stderr);
    std::fputs(error.what(), stderr);
    std::fputc('\n', stderr);
    connection_->Close();
  }
  if (!connection_->IsOpen()) { connection_->Close(); }
  Detach();
}

void SessionCommand::Detach() noexcept {
  session_->boundaries_.ClearCommand();
  session_->Detach();
  session_ = nullptr;
  connection_ = nullptr;
}

void SessionCommand::Keep() {
  if (session_ == nullptr) { throw SessionError("session command has already ended"); }
  if (&Session::Current() != session_ || session_->boundaries_.Depth() != 1) {
    throw SessionError("session command has an unfinished nested boundary");
  }
  session_->boundaries_.Release(*connection_, 1);
  session_->boundaries_.Commit(*connection_);
  Detach();
}

}
