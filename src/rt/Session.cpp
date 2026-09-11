#include "runtime/Session.h"

#include "runtime/Database.h"
#include "runtime/Events.h"
#include "runtime/Transaction.h"
#include "type/Date.h"

#include "BuiltinsWritten.h"
#include "Subscribers.h"

#include <string>
#include <string_view>

namespace agiru {

namespace {

thread_local Session *g_current = nullptr;

}

Session::Session(const std::string &connectionInfo)
    : connection_(connectionInfo), boundaries_(), previous_(g_current) {
  g_current = this;
}

Session::~Session() {
  detail::ReleaseAutomaticInstances();
  g_current = previous_;
}

Session &Session::Current() {
  if (g_current == nullptr) { throw SessionError("no session is open on this thread"); }
  return *g_current;
}

bool Session::HasCurrent() {
  return g_current != nullptr;
}

Date Session::WorkDate() const {
  return workDate_.IsUndefined() ? Today() : workDate_;
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
