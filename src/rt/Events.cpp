#include "runtime/Events.h"

#include "meta/Ids.h"
#include "runtime/ErrorValue.h"
#include "runtime/SingleInstance.h"
#include "runtime/Transaction.h"

#include "SessionState.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

std::vector<const SubscriptionCatalogue *> &Registered() {
  static std::vector<const SubscriptionCatalogue *> catalogues;
  return catalogues;
}

bool SameName(std::string_view a, std::string_view b) {
  return a.size() == b.size() && std::ranges::equal(a, b, [](unsigned char x, unsigned char y) {
           return std::tolower(x) == std::tolower(y);
         });
}

struct Invocation {
  void *instance;
  std::uint64_t generation;
};

using OwnedSubscriber = std::unique_ptr<void, void (*)(void *)>;

OwnedSubscriber MakeSubscriber(const SubscriptionCatalogue &catalogue) {
  if (catalogue.Maker() == nullptr || catalogue.Freer() == nullptr) {
    throw Error("event subscriber: invalid factory");
  }
  OwnedSubscriber owned(catalogue.Make(), catalogue.Freer());
  if (owned == nullptr) { throw Error("event subscriber: the factory returned no instance"); }
  return owned;
}

bool Listens(const Subscription &subscription,
             EventObject kind,
             std::int32_t objectId,
             std::string_view objectName,
             std::string_view event,
             std::string_view element) {
  if (subscription.kind != kind) { return false; }
  const bool sameObject = subscription.objectId != 0 && objectId != 0
                              ? subscription.objectId == objectId
                              : SameName(subscription.objectName, objectName);
  return sameObject && SameName(subscription.event, event) &&
         SameName(subscription.element, element);
}

std::vector<std::size_t> Bind(const Subscription &subscription,
                              const SubscriptionCatalogue &catalogue,
                              const EventArgs &args) {
  std::vector<std::size_t> bound;
  bound.reserve(subscription.parameters.size());
  for (const std::string_view name : subscription.parameters) {
    const auto at = std::ranges::find_if(
        args.names, [&](std::string_view published) { return SameName(published, name); });
    if (at == args.names.end() && SameName(name, "Sender") && args.sender != nullptr) {
      bound.push_back(kSenderSlot);
      continue;
    }
    if (at == args.names.end()) {
      throw Error("the subscriber " + std::string(catalogue.Name()) + " names a parameter " +
                  std::string(name) + " that the event " + std::string(subscription.event) +
                  " does not publish");
    }
    bound.push_back(static_cast<std::size_t>(at - args.names.begin()));
  }
  return bound;
}

const std::vector<const SubscriptionCatalogue *> &Catalogues() {
  static std::once_flag once;
  std::call_once(once, [] {
    std::ranges::sort(Registered(),
                      [](const SubscriptionCatalogue *a, const SubscriptionCatalogue *b) {
                        return a->Id().Value() < b->Id().Value();
                      });
  });
  return Registered();
}

}

SubscriptionCatalogue::SubscriptionCatalogue(CodeunitId id,
                                             std::string_view name,
                                             std::span<const Subscription> subscriptions,
                                             bool manual,
                                             bool singleInstance,
                                             void *(*make)(),
                                             void (*free)(void *))
    : id_(id),
      name_(name),
      subscriptions_(subscriptions),
      manual_(manual),
      singleInstance_(singleInstance),
      make_(make),
      free_(free) {
  Registered().push_back(this);
}

namespace detail {

namespace {

std::vector<Invocation> BoundInstances(const SubscriptionCatalogue &catalogue) {
  std::vector<Invocation> instances;
  const detail::SessionState *state = detail::SessionState::Peek();
  if (state == nullptr) { return instances; }
  for (const detail::SessionState::Binding &held : state->bindings) {
    if (held.catalogue == &catalogue) { instances.push_back({held.instance, held.generation}); }
  }
  return instances;
}

bool StillBound(const detail::SessionState *owner, std::uint64_t generation) {
  if (generation == 0) { return true; }
  const detail::SessionState *state = detail::SessionState::Peek();
  return state != nullptr && state == owner &&
         std::ranges::any_of(state->bindings,
                             [=](const auto &held) { return held.generation == generation; });
}

void Invoke(const Subscription &subscription,
            void *instance,
            const EventArgs &args,
            std::span<const std::size_t> bound,
            bool isolated) {
  if (!isolated) {
    subscription.invoke(instance, args, bound);
    return;
  }
  Scope boundary;
  try {
    subscription.invoke(instance, args, bound);
  } catch (const Error &e) {
    boundary.Discard(e);
    return;
  } catch (const std::exception &e) {
    boundary.Discard(e.what());
    return;
  }
  boundary.Keep();
}

void Dispatch(EventObject kind,
              std::int32_t objectId,
              std::string_view objectName,
              std::string_view event,
              std::string_view element,
              const EventArgs &args,
              bool isolated) {
  for (const SubscriptionCatalogue *catalogue : Catalogues()) {
    for (const Subscription &subscription : catalogue->Subscriptions()) {
      if (!Listens(subscription, kind, objectId, objectName, event, element)) { continue; }
      const SessionState *owner = SessionState::Peek();
      std::vector<Invocation> instances;
      OwnedSubscriber automatic(nullptr, catalogue->Freer());
      if (catalogue->Manual()) {
        instances = BoundInstances(*catalogue);
      } else if (catalogue->SingleInstance()) {
        instances.push_back(
            {SingleInstanceOf(catalogue->Id(), catalogue->Maker(), catalogue->Freer()), 0});
      } else {
        automatic = MakeSubscriber(*catalogue);
        instances.push_back({automatic.get(), 0});
      }
      if (instances.empty()) { continue; }
      const std::vector<std::size_t> bound = Bind(subscription, *catalogue, args);
      for (const Invocation &held : instances) {
        if (!StillBound(owner, held.generation)) { continue; }
        Invoke(subscription, held.instance, args, bound, isolated);
      }
    }
  }
}

}

void Raise(EventObject kind,
           std::int32_t objectId,
           std::string_view objectName,
           std::string_view event,
           std::string_view element,
           const EventArgs &args) {
  Dispatch(kind, objectId, objectName, event, element, args, false);
}

void RaiseIsolated(EventObject kind,
                   std::int32_t objectId,
                   std::string_view objectName,
                   std::string_view event,
                   std::string_view element,
                   const EventArgs &args) {
  Dispatch(kind, objectId, objectName, event, element, args, true);
}

bool BindSubscriptions(CodeunitId id, void *instance) {
  SessionState &state = SessionState::Current();
  const auto found = std::ranges::find_if(
      Catalogues(), [&](const SubscriptionCatalogue *c) { return c->Id().Value() == id.Value(); });
  if (found == Catalogues().end()) { return false; }
  if (!(*found)->Manual()) {
    throw Error("BindSubscription requires EventSubscriberInstance Manual");
  }
  if (instance == nullptr) { throw Error("BindSubscription requires a live instance"); }
  for (const SessionState::Binding &held : state.bindings) {
    if (held.instance == instance) { return false; }
  }
  if (state.lastBinding == std::numeric_limits<std::uint64_t>::max()) {
    throw Error("BindSubscription exhausted this session's binding identities");
  }
  state.bindings.push_back({id, *found, instance, ++state.lastBinding});
  return true;
}

bool UnbindSubscriptions(CodeunitId id, void *instance) {
  SessionState &state = SessionState::Current();
  const auto catalogue =
      std::ranges::find_if(Catalogues(), [=](const auto *entry) { return entry->Id() == id; });
  if (catalogue != Catalogues().end() && !(*catalogue)->Manual()) {
    throw Error("UnbindSubscription requires EventSubscriberInstance Manual");
  }
  auto &held = state.bindings;
  const auto at = std::ranges::find_if(
      held, [&](const SessionState::Binding &b) { return b.instance == instance && b.id == id; });
  if (at == held.end()) { return false; }
  held.erase(at);
  return true;
}

void ReleaseSubscriptions(CodeunitId id, void *instance) noexcept {
  SessionState::ReleaseBindings(id, instance);
}

}

}
