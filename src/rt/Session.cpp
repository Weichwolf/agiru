#include "runtime/Session.h"

#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Events.h"
#include "runtime/Transaction.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Language.h"

#include "SessionState.h"

#include <string>
#include <string_view>

namespace agiru {

namespace {

thread_local Session *g_current = nullptr;

}

Session::Session(const std::string &connectionInfo)
    : connection_(connectionInfo), boundaries_(), previous_(g_current) {
  g_current = this;
  ::agiru::Language::MakeCurrent(language_);
}

Session::Session(const std::string &connectionInfo, const Guid &authenticatedUser)
    : Session(connectionInfo) {
  if (authenticatedUser.IsNull()) { throw SessionError("session user is not active"); }
  platform::User user;
  if (!user.Get(authenticatedUser) || user.UserName.Value().empty() ||
      user.State != platform::UserState::Enabled ||
      (!user.ExpiryDate.IsUndefined() && user.ExpiryDate <= CurrentDateTime())) {
    throw SessionError("session user is not active");
  }
  userSecurityId_ = user.UserSecurityID;
  userId_ = user.UserName.Value();
}

Session::~Session() {
  if (state_ != nullptr) { state_->ReleaseSingles(); }
  g_current = previous_;
  ::agiru::Language::MakeCurrent(previous_ != nullptr ? previous_->language_
                                                      : ::agiru::Language::kEnglishUnitedStates);
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
