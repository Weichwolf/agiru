#include "runtime/SingleInstance.h"

#include "meta/Ids.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/UiHost.h"

#include "SessionState.h"

#include <memory>
#include <utility>
#include <vector>

namespace agiru::detail {

SessionState::SessionState() = default;
SessionState::~SessionState() = default;

SessionState &SessionState::Current() {
  return For(Session::Current());
}

SessionState &SessionState::For(Session &session) {
  if (session.state_ == nullptr) { session.state_ = std::make_unique<SessionState>(); }
  return *session.state_;
}

SessionState *SessionState::Peek() {
  return Session::HasCurrent() ? Session::Current().state_.get() : nullptr;
}

void SessionState::ReleaseBindings(CodeunitId id, void *instance) noexcept {
  if (!Session::HasCurrent()) { return; }
  for (Session *session = &Session::Current(); session != nullptr; session = session->previous_) {
    if (session->state_ == nullptr) { continue; }
    std::erase_if(session->state_->bindings, [=](const Binding &held) noexcept {
      return held.id == id && held.instance == instance;
    });
  }
}

void *SingleInstanceOf(CodeunitId id, void *(*make)(), void (*free)(void *)) {
  auto &singles = SessionState::Current().singles;
  auto found = singles.find(id);
  if (found == singles.end()) {
    if (make == nullptr || free == nullptr) { throw Error("SingleInstance: invalid factory"); }
    SessionState::OwnedInstance owned(make(), free);
    if (owned == nullptr) { throw Error("SingleInstance: the factory returned no instance"); }
    found = singles.emplace(id, std::move(owned)).first;
  }
  return found->second.get();
}

void SessionState::ReleaseSingles() {
  decltype(singles) released;
  released.swap(singles);
}

void ReleaseSingleInstances() {
  if (SessionState *state = SessionState::Peek(); state != nullptr) { state->ReleaseSingles(); }
}

}
