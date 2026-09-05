#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Events.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include "Check.h"

#include <array>
#include <string>
#include <string_view>

using agiru::Boolean;
using agiru::Codeunit;
using agiru::CodeunitId;
using agiru::CodeunitTraits;
using agiru::Error;
using agiru::EventObject;
using agiru::Integer;
using agiru::Subscription;
using agiru::SubscriptionCatalogue;
using agiru::Text;

namespace {
class Publisher_Codeunit;
class Listener_Codeunit;
class Misnamed_Codeunit;
} // namespace

template <> struct agiru::CodeunitTraits<Publisher_Codeunit> {
  static constexpr CodeunitId kId{50100};
  static constexpr std::string_view kName{"Event Gate Publisher"};
};

template <> struct agiru::CodeunitTraits<Listener_Codeunit> {
  static constexpr CodeunitId kId{50101};
  static constexpr std::string_view kName{"Event Gate Listener"};
};

template <> struct agiru::CodeunitTraits<Misnamed_Codeunit> {
  static constexpr CodeunitId kId{50102};
  static constexpr std::string_view kName{"Event Gate Misnamed"};
};

namespace {

/// AL: `[IntegrationEvent(false, false)] procedure OnBeforePost(var Amount: Integer; Note: Text;
/// var IsHandled: Boolean)`, as the generator emits it.
class Publisher_Codeunit : public Codeunit<Publisher_Codeunit> {
public:
  void OnBeforePost(Integer &Amount, Text<0> Note, Boolean &IsHandled) {
    static constexpr std::array<std::string_view, 3> kNames{"Amount", "Note", "IsHandled"};
    agiru::detail::RaiseEvent(EventObject::Codeunit,
                              CodeunitTraits<Publisher_Codeunit>::kId.Value(),
                              CodeunitTraits<Publisher_Codeunit>::kName,
                              "OnBeforePost",
                              kNames,
                              Amount,
                              Note,
                              IsHandled);
  }
};

/// AL: a `Manual` codeunit subscribing with a SUBSET of the parameters, in another order.
class Listener_Codeunit : public Codeunit<Listener_Codeunit> {
public:
  void Doubles(Boolean &IsHandled, Integer &Amount) {
    Amount = Amount * 2;
    IsHandled = true;
    ++calls;
  }

  int calls = 0;
};

class Misnamed_Codeunit : public Codeunit<Misnamed_Codeunit> {
public:
  void Wrong(Integer &Total) { Total = 0; }
};

} // namespace

namespace {

constexpr std::array<std::string_view, 2> kDoublesNames{"IsHandled", "Amount"};
constexpr std::array<Subscription, 1> kListenerSubscriptions{{
    {EventObject::Codeunit, 0, "Event Gate Publisher", "OnBeforePost", "", kDoublesNames,
     &agiru::detail::InvokeSubscriber<Listener_Codeunit, &Listener_Codeunit::Doubles>},
}};
const SubscriptionCatalogue kListenerCatalogue{
    CodeunitTraits<Listener_Codeunit>::kId,
    CodeunitTraits<Listener_Codeunit>::kName,
    kListenerSubscriptions,
    true,
    []() -> void * { return new Listener_Codeunit(); },
    [](void *instance) { delete static_cast<Listener_Codeunit *>(instance); }};

constexpr std::array<std::string_view, 1> kWrongNames{"Total"};
constexpr std::array<Subscription, 1> kMisnamedSubscriptions{{
    {EventObject::Codeunit, 0, "Event Gate Publisher", "OnBeforePost", "", kWrongNames,
     &agiru::detail::InvokeSubscriber<Misnamed_Codeunit, &Misnamed_Codeunit::Wrong>},
}};
const SubscriptionCatalogue kMisnamedCatalogue{
    CodeunitTraits<Misnamed_Codeunit>::kId,
    CodeunitTraits<Misnamed_Codeunit>::kName,
    kMisnamedSubscriptions,
    true,
    []() -> void * { return new Misnamed_Codeunit(); },
    [](void *instance) { delete static_cast<Misnamed_Codeunit *>(instance); }};

void AManualSubscriberHearsOnlyWhileBound() {
  Publisher_Codeunit publisher;
  Listener_Codeunit listener;
  Integer amount = 5;
  Boolean handled = false;
  publisher.OnBeforePost(amount, Text<0>("note"), handled);
  CHECK_TRUE("unbound, a Manual subscriber hears nothing", listener.calls == 0 && amount == 5);
  CHECK_TRUE("BindSubscription binds", agiru::BindSubscription(listener));
  CHECK_TRUE("and binding twice says so", !agiru::BindSubscription(listener));
  publisher.OnBeforePost(amount, Text<0>("note"), handled);
  CHECK_TRUE("bound, the subscriber runs once", listener.calls == 1);
  CHECK_TRUE("a var parameter is the caller's variable", amount == 10 && handled);
  CHECK_TRUE("UnbindSubscription unbinds", agiru::UnbindSubscription(listener));
  publisher.OnBeforePost(amount, Text<0>("note"), handled);
  CHECK_TRUE("and unbound it hears nothing again", listener.calls == 1 && amount == 10);
  CHECK_TRUE("unbinding twice says so", !agiru::UnbindSubscription(listener));
}

void ASubscriberNamingAnUnpublishedParameterIsRefused() {
  Publisher_Codeunit publisher;
  Misnamed_Codeunit wrong;
  static_cast<void>(agiru::BindSubscription(wrong));
  Integer amount = 1;
  Boolean handled = false;
  std::string said;
  try {
    publisher.OnBeforePost(amount, Text<0>(""), handled);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("the raise names the subscriber and the parameter",
             said.contains("Event Gate Misnamed") && said.contains("Total"));
  static_cast<void>(agiru::UnbindSubscription(wrong));
}

} // namespace

int main() {
  return gate::Run("Event", [] {
    AManualSubscriberHearsOnlyWhileBound();
    ASubscriberNamingAnUnpublishedParameterIsRefused();
  });
}
