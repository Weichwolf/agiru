#include "runtime/Events.h"

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"

#include "Subscribers.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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

struct Bound {
  const SubscriptionCatalogue *catalogue;
  void *instance;
};

std::vector<Bound> &ManualBindings() {
  thread_local std::vector<Bound> bound;
  return bound;
}

struct Automatic {
  const SubscriptionCatalogue *catalogue;
  void *instance;

  ~Automatic() { catalogue->Free(instance); }
};

std::map<const SubscriptionCatalogue *, std::unique_ptr<Automatic>> &Made() {
  thread_local std::map<const SubscriptionCatalogue *, std::unique_ptr<Automatic>> made;
  return made;
}

void *AutomaticInstance(const SubscriptionCatalogue &catalogue) {
  std::map<const SubscriptionCatalogue *, std::unique_ptr<Automatic>> &made = Made();
  auto found = made.find(&catalogue);
  if (found == made.end()) {
    found =
        made.emplace(&catalogue, std::make_unique<Automatic>(&catalogue, catalogue.Make())).first;
  }
  return found->second->instance;
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
                                             void *(*make)(),
                                             void (*free)(void *))
    : id_(id),
      name_(name),
      subscriptions_(subscriptions),
      manual_(manual),
      make_(make),
      free_(free) {
  Registered().push_back(this);
}

namespace detail {

namespace {

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
      std::vector<void *> instances;
      if (catalogue->Manual()) {
        for (const Bound &held : ManualBindings()) {
          if (held.catalogue == catalogue) { instances.push_back(held.instance); }
        }
      } else {
        instances.push_back(AutomaticInstance(*catalogue));
      }
      if (instances.empty()) { continue; }
      const std::vector<std::size_t> bound = Bind(subscription, *catalogue, args);
      for (void *instance : instances) {
        if (!isolated) {
          subscription.invoke(instance, args, bound);
          continue;
        }
        Scope boundary;
        try {
          subscription.invoke(instance, args, bound);
        } catch (const Error &e) {
          boundary.Discard(e);
          continue;
        } catch (const std::exception &e) {
          boundary.Discard(e.what());
          continue;
        }
        boundary.Keep();
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

void ReleaseAutomaticInstances() {
  Made().clear();
}

bool BindSubscriptions(CodeunitId id, void *instance) {
  const auto found = std::ranges::find_if(
      Catalogues(), [&](const SubscriptionCatalogue *c) { return c->Id().Value() == id.Value(); });
  if (found == Catalogues().end()) { return false; }
  for (const Bound &held : ManualBindings()) {
    if (held.instance == instance) { return false; }
  }
  ManualBindings().push_back(Bound{.catalogue = *found, .instance = instance});
  return true;
}

bool UnbindSubscriptions(CodeunitId id, void *instance) {
  std::vector<Bound> &held = ManualBindings();
  const auto at = std::ranges::find_if(held, [&](const Bound &b) {
    return b.instance == instance && b.catalogue->Id().Value() == id.Value();
  });
  if (at == held.end()) { return false; }
  held.erase(at);
  return true;
}

}

}
