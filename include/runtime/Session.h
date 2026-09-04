#pragma once

#include "platform/Tenant.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/Guid.h"

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

  /// \brief AL `UserSecurityId()`.
  ///
  /// \return The security ID of the user this session runs as.
  ///
  /// \note IT IS THE BLANK GUID UNTIL THERE IS AN AUTHENTICATION STORY, and that is a measured
  ///       decision rather than a placeholder: the predecessor returned exactly this constant and
  ///       reached 97.0 % of the UT subset over it (`~/Git/openerp`, `builtins/_system.py:409`). It
  ///       lives on the SESSION rather than in a function, because a user is a property of a
  ///       session and a hardcoded GUID inside a call could never become one.
  [[nodiscard]] const Guid &UserSecurityId() const { return userSecurityId_; }

  /// \brief AL `UserId()`.
  ///
  /// \return The name of the user this session runs as.
  ///
  /// \note IT IS `SYSTEM` UNTIL THERE IS AN AUTHENTICATION STORY, and that is measured rather than
  ///       chosen: the predecessor returns exactly this constant unless a test overrides it, and
  ///       reached 97.0 % of the UT subset over it (`~/Git/openerp`,
  ///       `builtins/_system.py:_al_user_id`). It sits beside `UserSecurityId()` for the reason
  ///       that one gives -- a user is a property of a SESSION, and a constant inside a function
  ///       could never become one.
  [[nodiscard]] std::string_view UserId() const { return userId_; }

  /// \brief AL `WorkDate()` -- the date a session posts under.
  ///
  /// \return The work date; today's date until one is set.
  ///
  /// \note IT IS A PROPERTY OF THE SESSION, which is what AL means by it: `WORKDATE := 010124D`
  ///       changes what THIS session posts under and nothing else. A test library sets it and every
  ///       posting after that reads it, which is why it cannot live in a function.
  [[nodiscard]] Date WorkDate() const;

  /// \brief AL `WorkDate(Date)` -- sets it.
  /// \param date The new work date; the blank date restores today's.
  /// \return The date it now carries.
  Date WorkDate(Date date);

  /// \brief AL `CompanyName()` -- the company this session works in.
  ///
  /// \return The name; empty until one is opened.
  ///
  /// \warning THE COMPANY IS NOT YET A SCHEMA. BC keeps one set of tables per company and the
  ///          CRONUS load carries them under `"CRONUS International Ltd"`; this returns the name a
  ///          session was opened with and nothing reads it for a table yet (board:0004).
  [[nodiscard]] std::string_view CompanyName() const { return company_; }

  /// \brief Names the company this session works in.
  /// \param name The company.
  void CompanyName(std::string_view name) { company_ = name; }

  /// \brief Whether this session runs as the service rather than a self-hosted instance.
  ///
  /// \return False unless the host said otherwise.
  ///
  /// \note IT IS A PROPERTY OF THE SESSION AND NOT OF A CODEUNIT, because the runtime may not know
  ///       an AL object -- `Codeunit "Environment Information"` reads THIS rather than the other
  ///       way round. The test libraries switch it with
  ///       `EnvironmentInfoTestLibrary.SetTestabilitySoftwareAsAService(true)`, which lands here.
  [[nodiscard]] Boolean IsSaaS() const { return tenant_.saas; }

  /// \brief Says whether this session runs as the service.
  /// \param saas True for the service.
  void SetSaaS(Boolean saas) { tenant_.saas = saas; }

  /// \return What the tenant says about its own deployment.
  [[nodiscard]] const TenantSettings &Tenant() const { return tenant_; }

  /// \return The tenant's settings, to be changed by the host or a test library.
  [[nodiscard]] TenantSettings &Tenant() { return tenant_; }

  /// \return The nested transaction boundaries this session is inside.
  ///
  /// \note A SESSION AND ITS BOUNDARIES ARE ONE THING, which is why they live together. A savepoint
  ///       taken on a connection other than the one the statements run on rolls back nothing
  ///       (board:0012), and holding both here is what makes handing the wrong one impossible.
  [[nodiscard]] Boundaries &Transaction() { return boundaries_; }

private:
  Connection connection_;
  Boundaries boundaries_;
  Session *previous_;
  Guid userSecurityId_;
  std::string userId_{"SYSTEM"};
  Date workDate_;
  std::string company_;
  TenantSettings tenant_;
};

}
