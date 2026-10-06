#include "runtime/Session.h"

#include "meta/TableDef.h"
#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Events.h"
#include "runtime/Table.h"
#include "runtime/TablePermissions.h"
#include "runtime/Transaction.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Language.h"

#include "Rows.h"
#include "SessionState.h"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {

namespace {

thread_local Session *g_current = nullptr;

}

Session::Session(const std::string &connectionInfo)
    : ownedConnection_(std::make_unique<Connection>(connectionInfo)),
      connection_(ownedConnection_.get()),
      previous_(g_current) {
  g_current = this;
  ::agiru::Language::MakeCurrent(language_);
}

Session::Session(const std::string &connectionInfo, const Guid &authenticatedUser)
    : Session(connectionInfo) {
  ResolveUser(authenticatedUser);
}

Session::Session(const Guid &authenticatedUser)
    : state_(std::make_unique<detail::SessionState>()),
      userSecurityId_(authenticatedUser),
      userId_() {
  if (authenticatedUser.IsNull()) { throw SessionError("session user is not active"); }
}

void Session::ResolveUser(const Guid &authenticatedUser) {
  if (authenticatedUser.IsNull()) { throw SessionError("session user is not active"); }
  platform::User user;
  const auto &table = platform::kUserTable;
  const std::array<std::optional<std::string>, 1> key{authenticatedUser.ToStorageText()};
  const auto row = GetRow(Database(), table, key);
  if (!row) { throw SessionError("session user is not active"); }
  std::size_t column = 0;
  for (const auto &field : table.fields) {
    if (Stored(field)) { detail::SetFieldText(&user, field, Required((*row)[column++], field)); }
  }
  if (user.UserName.Value().empty() || user.State != platform::UserState::Enabled ||
      (!user.ExpiryDate.IsUndefined() && user.ExpiryDate <= CurrentDateTime())) {
    throw SessionError("session user is not active");
  }
  userSecurityId_ = user.UserSecurityID;
  userId_ = user.UserName.Value();
}

void Session::TablePermissions(std::shared_ptr<const TablePermissionAuthority> authority) {
  if (!authority || boundaries_.Depth() != 0 ||
      (connection_ != nullptr && connection_->InTransaction())) {
    throw SessionError("table authority requires a provider and an idle session");
  }
  tableAuthority_ = std::move(authority);
}

bool Session::AllowsTable(const TableDef &table, TableOperation operation) const {
  if (!HasCurrent() || &Current() != this) {
    throw SessionError("table permissions require the active session");
  }
  if (tableAuthority_) { return tableAuthority_->Allows(table, operation); }
  if (userSecurityId_.IsNull()) { return true; }
  throw Error("Authenticated session has no table permission authority", "PermissionUnavailable");
}

Session::~Session() {
  if (state_ != nullptr) { state_->ReleaseSingles(); }
  if (ownedConnection_ != nullptr) { RestoreCurrent(); }
}

void Session::RestoreCurrent() noexcept {
  g_current = previous_;
  ::agiru::Language::MakeCurrent(previous_ != nullptr ? previous_->language_
                                                      : ::agiru::Language::kEnglishUnitedStates);
}

const Connection &Session::Database() const {
  if (connection_ == nullptr) { throw SessionError("session has no active database lease"); }
  return *connection_;
}

void Session::Language(::agiru::Integer id) {
  language_ = id;
  if (g_current == this) { ::agiru::Language::MakeCurrent(id); }
}

void Session::Attach(Connection &connection) {
  if (ownedConnection_ != nullptr) { throw SessionError("an owned session is already active"); }
  if (!connection.IsOpen()) { throw DatabaseError("session command requires an open connection"); }
  if (state_->commandActive.test_and_set()) {
    throw SessionError("session command is already active");
  }
  if (boundaries_.Depth() != 0 || connection.InTransaction()) {
    state_->commandActive.clear();
    throw SessionError("session command requires an idle transaction");
  }
  connection_ = &connection;
  previous_ = g_current;
  g_current = this;
  ::agiru::Language::MakeCurrent(language_);
}

void Session::Detach() noexcept {
  RestoreCurrent();
  connection_ = nullptr;
  previous_ = nullptr;
  state_->commandActive.clear();
}

Session &Session::Current() {
  if (g_current == nullptr) { throw SessionError("no session is open on this thread"); }
  return *g_current;
}

bool Session::HasCurrent() {
  return g_current != nullptr;
}

Date Session::WorkDate() const {
  return workDate_.IsUndefined() ? CurrentDateTime().Date() : workDate_;
}

void Session::OpenCompany() {
  static constexpr std::string_view kCompanyTriggers = "Company Triggers";
  static constexpr std::string_view kOpened = "OnCompanyOpenCompleted";
  detail::Scope boundary;
  detail::RaiseIsolated(EventObject::Codeunit, 0, kCompanyTriggers, kOpened, "", EventArgs{});
  boundary.Keep();
}

Date Session::WorkDate(Date date) {
  workDate_ = date;
  return WorkDate();
}

}
