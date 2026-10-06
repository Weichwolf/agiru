#pragma once

#include "platform/Tenant.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Language.h"

#include <memory>
#include <string>

/// \file
/// \brief The ambient session an AL record belongs to.

namespace agiru {

class SessionCommand;

namespace detail {
/// \brief Session-owned runtime storage, defined privately by the runtime.
struct SessionState;
}

/// \brief An error raised for a missing session or refused user identity.
class SessionError : public Error {
public:
  using Error::Error;
};

/// \brief The Windows language id a session runs in until something moves it.
///
/// 1033 is `en-US`, which is the language the BaseApp's own captions are written in and the one
/// every AL test compares its error texts against.
constexpr ::agiru::Integer kEnglishUnitedStates = 1033;

/// \brief Private AL state, current on one worker while executing a command.
/// DSN constructors own a connection and activate immediately for harness compatibility.
/// The Guid constructor stays detached; SessionCommand borrows an exclusive connection.
/// The host must keep a detached session alive until all its commands and pages close,
/// and access its mutable state only while exclusively active or otherwise idle.
class Session {
public:
  /// \brief Opens an unaccounted harness session with SYSTEM/blank user identity.
  /// \param connectionInfo A libpq connection string or URI.
  /// \warning Not a production client sign-in or an authorization grant.
  /// \throws DatabaseError when the connection cannot be established.
  explicit Session(const std::string &connectionInfo);

  /// \brief Opens a session for an already authenticated user from the system User table.
  /// \param connectionInfo A libpq connection string or URI.
  /// \param authenticatedUser Security ID established by the trusted host's authentication.
  /// \throws SessionError for a blank, missing, unnamed, disabled or expired user.
  /// \throws DatabaseError when the connection or User-table read fails.
  /// \note Reads the name and status from PostgreSQL; does not authenticate credentials,
  ///       grant permissions, invoke company sign-in triggers or gate features by license.
  ///       Status is checked at construction; a persistent host must enforce revocation.
  ///       Failure restores the previous thread-local session and language.
  Session(const std::string &connectionInfo, const Guid &authenticatedUser);

  /// \brief Creates detached AL state without opening a database or changing the current session.
  /// \param authenticatedUser Nonblank user GUID established by the trusted host.
  /// \throws SessionError for a blank GUID.
  /// \note Every SessionCommand resolves the account name/status from PostgreSQL again.
  ///       Does not authenticate credentials or grant permissions. No UserId is resolved
  ///       until the first accepted command; pages and SingleInstances survive detachment.
  explicit Session(const Guid &authenticatedUser);

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

  /// \return The session's owned or currently borrowed database connection.
  /// \throws SessionError for an idle detached session.
  [[nodiscard]] const Connection &Database() const;

  /// \brief AL `UserSecurityId()`.
  ///
  /// \return The security ID of the user this session runs as.
  ///
  /// \note The Guid from the system User row, or blank in the unaccounted harness constructor.
  [[nodiscard]] const Guid &UserSecurityId() const { return userSecurityId_; }

  /// \brief AL `GlobalLanguage()` -- the language this session runs in.
  /// \return The Windows language id.
  [[nodiscard]] ::agiru::Integer Language() const { return language_; }

  /// \brief AL `GlobalLanguage(Integer)` -- moves the session to another language.
  /// \param id The Windows language id.
  ///
  /// \note IT IS SESSION STATE AND NOT PROCESS STATE, which is what the name hides:
  ///       `system-globallanguage-method.md` calls it "the current global language setting", and a
  ///       service tier runs ten thousand sessions in one process. A global here would be one
  ///       session's language answering for every other.
  void Language(::agiru::Integer id);

  /// \brief AL `UserId()`.
  ///
  /// \return The name of the user this session runs as.
  ///
  /// \note A snapshot of User."User Name" at the last accepted activation, or SYSTEM in a harness.
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

  /// \brief The platform's sign-in, once the company is named: `Company Triggers.
  ///        OnCompanyOpenCompleted` is raised as the isolated event it is, and what its
  ///        subscribers wrote is kept.
  ///
  /// `devenv-oncompanyopencompleted.md`: the base application subscribes to that platform event
  /// and raises `System Initialization.OnAfterLogin` from it, "both raised during sign-in when
  /// Business Central tries to open the relevant company". A session that never signed in left
  /// every `OnAfterLogin` subscriber dead -- among them the binder that makes `Insert(false)` on a
  /// Customer carry its related record ids (`API Setup UT`, 6 cases; openerp WI-1216/WI-1217).
  ///
  /// \warning THE OBSOLETE `OnCompanyOpen` IS NOT RAISED. It is not isolated -- "a failure in any
  ///          event subscriber will stop the sign-in process" -- and its base-application
  ///          subscriber commits twice on its way through; the predecessor measured the isolated
  ///          event alone as safe and left the other as a question (openerp WI-1218).
  void OpenCompany();

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
  friend struct detail::SessionState;
  friend class SessionCommand;
  void ResolveUser(const Guid &authenticatedUser);
  void Attach(Connection &connection);
  void Detach() noexcept;
  void RestoreCurrent() noexcept;
  mutable std::unique_ptr<detail::SessionState> state_;
  std::unique_ptr<Connection> ownedConnection_;
  const Connection *connection_ = nullptr;
  Boundaries boundaries_;
  Session *previous_ = nullptr;
  Guid userSecurityId_;
  std::string userId_{"SYSTEM"};
  Date workDate_;
  ::agiru::Integer language_ = kEnglishUnitedStates;
  std::string company_;
  TenantSettings tenant_;
};

}
