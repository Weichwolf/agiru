#include "runtime/Session.h"

#include "runtime/Database.h"

#include <string>

namespace agiru {

namespace {

thread_local Session *g_current = nullptr;

} // namespace

Session::Session(const std::string &connectionInfo)
    : connection_(connectionInfo), boundaries_(), previous_(g_current) {
  g_current = this;
}

Session::~Session() {
  g_current = previous_;
}

Session &Session::Current() {
  if (g_current == nullptr) { throw SessionError("no session is open on this thread"); }
  return *g_current;
}

bool Session::HasCurrent() {
  return g_current != nullptr;
}

} // namespace agiru
