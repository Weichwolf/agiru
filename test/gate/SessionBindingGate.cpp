#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/ErrorValue.h"
#include "runtime/Events.h"
#include "runtime/Session.h"
#include "runtime/Subscriptions.h"
#include "type/Integer.h"

#include "Check.h"

#include <array>
#include <functional>
#include <string_view>
#include <thread>

namespace {
class Listener;
}

template <> struct agiru::CodeunitTraits<Listener> {
  static constexpr agiru::CodeunitId kId{50124};
  static constexpr std::string_view kName = "Session Binding Listener";
};

namespace {

constexpr agiru::CodeunitId kManual{50124};
constexpr agiru::CodeunitId kAutomatic{50126};
constexpr std::array<std::string_view, 1> kNames{"Seen"};

class Listener : public agiru::Codeunit<Listener> {
public:
  void Hear(agiru::Integer &seen) {
    ++calls;
    ++seen;
    if (action) { action(); }
  }

  int calls = 0;
  std::function<void()> action;
};

constexpr std::array<agiru::Subscription, 1> kSubscriptions{
    {{.kind = agiru::EventObject::Codeunit,
      .objectId = 50125,
      .objectName = "Binding Publisher",
      .event = "Observe",
      .element = "",
      .parameters = kNames,
      .invoke = &agiru::detail::InvokeSubscriber<Listener, &Listener::Hear>}}};
const agiru::SubscriptionCatalogue kManualCatalogue{
    kManual,
    agiru::CodeunitTraits<Listener>::kName,
    kSubscriptions,
    true,
    false,
    []() -> void * { return new Listener; },
    [](void *instance) { delete static_cast<Listener *>(instance); }};
const agiru::SubscriptionCatalogue kAutomaticCatalogue{
    kAutomatic,
    "Automatic Binding Listener",
    {},
    false,
    false,
    []() -> void * { return new Listener; },
    [](void *instance) { delete static_cast<Listener *>(instance); }};

agiru::Integer Raise() {
  agiru::Integer seen = 0;
  agiru::detail::RaiseEvent(
      agiru::EventObject::Codeunit, 50125, "Binding Publisher", "Observe", kNames, seen);
  return seen;
}

void NestedSessions() {
  const agiru::Session parent(AGIRU_TEST_DSN);
  Listener outer;
  Listener inner;
  CHECK_TRUE("parent binds its own instance", agiru::detail::BindSubscriptions(kManual, &outer));
  CHECK_TRUE("duplicate binding is refused", !agiru::detail::BindSubscriptions(kManual, &outer));
  {
    const agiru::Session child(AGIRU_TEST_DSN);
    CHECK_TRUE("child cannot see parent subscriptions", Raise() == 0);
    CHECK_TRUE("child cannot unbind a parent subscription",
               !agiru::detail::UnbindSubscriptions(kManual, &outer));
    CHECK_TRUE("child binds its own instance", agiru::detail::BindSubscriptions(kManual, &inner));
    const int before = outer.calls;
    CHECK_TRUE("child dispatch reaches only its own instance", Raise() == 1);
    CHECK_TRUE("child dispatch never changes parent state", outer.calls == before);
  }
  const int before = inner.calls;
  CHECK_TRUE("parent binding survives child close", Raise() == 1);
  CHECK_TRUE("child binding is discarded on close", inner.calls == before);
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &outer));
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &inner));
}

void SequentialSessions() {
  Listener held;
  {
    const agiru::Session first(AGIRU_TEST_DSN);
    static_cast<void>(agiru::detail::BindSubscriptions(kManual, &held));
  }
  const agiru::Session reused(AGIRU_TEST_DSN);
  CHECK_TRUE("a reused worker inherits no subscriptions", Raise() == 0);
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &held));
}

void DestructionHookReachesAncestors() {
  const agiru::Session parent(AGIRU_TEST_DSN);
  Listener held;
  static_cast<void>(agiru::detail::BindSubscriptions(kManual, &held));
  {
    const agiru::Session child(AGIRU_TEST_DSN);
    agiru::detail::ReleaseSubscriptions(kManual, &held);
  }
  CHECK_TRUE("destruction under a child removes the ancestor binding", Raise() == 0);
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &held));
}

void AReboundInstanceIsNotAnOldSnapshotEntry() {
  const agiru::Session session(AGIRU_TEST_DSN);
  Listener first;
  Listener second;
  first.action = [&second] {
    static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &second));
    static_cast<void>(agiru::detail::BindSubscriptions(kManual, &second));
  };
  static_cast<void>(agiru::detail::BindSubscriptions(kManual, &first));
  static_cast<void>(agiru::detail::BindSubscriptions(kManual, &second));
  CHECK_TRUE("rebound instances are not invoked from the old snapshot", Raise() == 1);
  CHECK_TRUE("the old snapshot never calls the new registration", second.calls == 0);
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &first));
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &second));
}

void AutomaticBindingRefuses() {
  const agiru::Session session(AGIRU_TEST_DSN);
  Listener held;
  bool bindRefused = false;
  bool unbindRefused = false;
  try {
    static_cast<void>(agiru::detail::BindSubscriptions(kAutomatic, &held));
  } catch (const agiru::Error &) { bindRefused = true; }
  try {
    static_cast<void>(agiru::detail::UnbindSubscriptions(kAutomatic, &held));
  } catch (const agiru::Error &) { unbindRefused = true; }
  CHECK_TRUE("automatic subscribers cannot be manually bound", bindRefused);
  CHECK_TRUE("automatic subscribers cannot be manually unbound", unbindRefused);
}

void AWorkerCannotSeeAnotherSessionsBindings() {
  const agiru::Session parent(AGIRU_TEST_DSN);
  Listener held;
  static_cast<void>(agiru::detail::BindSubscriptions(kManual, &held));
  agiru::Integer observed = -1;
  std::thread worker([&observed] {
    const agiru::Session other(AGIRU_TEST_DSN);
    observed = Raise();
  });
  worker.join();
  CHECK_TRUE("another worker has no caller subscriptions", observed == 0);
  CHECK_TRUE("the parent still has its own binding", Raise() == 1);
  static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &held));
}

void SessionlessBindingRefuses() {
  Listener held;
  bool refused = false;
  try {
    static_cast<void>(agiru::detail::BindSubscriptions(kManual, &held));
  } catch (const agiru::SessionError &) { refused = true; }
  if (!refused) { static_cast<void>(agiru::detail::UnbindSubscriptions(kManual, &held)); }
  CHECK_TRUE("manual binding requires a session", refused);
}

}

int main() {
  return gate::Run("SessionBinding", [] {
    NestedSessions();
    SequentialSessions();
    DestructionHookReachesAncestors();
    AReboundInstanceIsNotAnOldSnapshotEntry();
    AutomaticBindingRefuses();
    AWorkerCannotSeeAnotherSessionsBindings();
    SessionlessBindingRefuses();
  });
}
