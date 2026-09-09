#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Events.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include "BuiltinsWritten.h"
#include "Check.h"
#include "LineNumberBuffer.h"

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
class Watcher_Codeunit;
class Told_Codeunit;
class Unsent_Codeunit;
} // namespace

template <> struct agiru::CodeunitTraits<Publisher_Codeunit> {
  static constexpr CodeunitId kId{50100};
  static constexpr std::string_view kName{"Event Gate Publisher"};
};

template <> struct agiru::CodeunitTraits<Listener_Codeunit> {
  static constexpr CodeunitId kId{50101};
  static constexpr std::string_view kName{"Event Gate Listener"};
};

template <> struct agiru::CodeunitTraits<Watcher_Codeunit> {
  static constexpr CodeunitId kId{50103};
  static constexpr std::string_view kName{"Event Gate Watcher"};
};

template <> struct agiru::CodeunitTraits<Told_Codeunit> {
  static constexpr CodeunitId kId{50104};
  static constexpr std::string_view kName{"Event Gate Told"};
};

template <> struct agiru::CodeunitTraits<Unsent_Codeunit> {
  static constexpr CodeunitId kId{50105};
  static constexpr std::string_view kName{"Event Gate Unsent"};
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

/// AL: `[EventSubscriber(ObjectType::Table, Database::"Line Number Buffer", 'OnBeforeInsertEvent',
/// '', false, false)] local procedure Seen(var Rec: Record "Line Number Buffer"; RunTrigger:
/// Boolean)`.
class Watcher_Codeunit : public Codeunit<Watcher_Codeunit> {
public:
  void Seen(agiru::app::tables::LineNumberBuffer &Rec, Boolean RunTrigger) {
    Rec.NewLineNumber = Rec.OldLineNumber + 1;
    sawRunTrigger = RunTrigger;
    ++seen;
  }

  int seen = 0;
  Boolean sawRunTrigger = false;
};

// A SUBSCRIBER MAY NAME ITS FIRST PARAMETER `Sender` AND RECEIVE THE RAISING OBJECT, which the
// publisher never lists: AL's own rule for table and codeunit events, and the BaseApp's
// `RecordRestrictionMgt.CustomerCheckSalesPostRestrictions(var Sender: Record "Sales Header")`.
class Told_Codeunit : public Codeunit<Told_Codeunit> {
public:
  void Heard(agiru::app::tables::LineNumberBuffer &Sender, Boolean RunTrigger) {
    Sender.NewLineNumber = Sender.OldLineNumber + 10;
    static_cast<void>(RunTrigger);
    ++told;
  }

  int told = 0;
};

class Unsent_Codeunit : public Codeunit<Unsent_Codeunit> {
public:
  void Wants(Publisher_Codeunit &Sender, Integer &Amount) {
    static_cast<void>(Sender);
    Amount = 0;
  }
};

class Misnamed_Codeunit : public Codeunit<Misnamed_Codeunit> {
public:
  void Wrong(Integer &Total) { Total = 0; }
};

} // namespace

namespace {

constexpr std::array<std::string_view, 2> kDoublesNames{"IsHandled", "Amount"};
constexpr std::array<Subscription, 1> kListenerSubscriptions{{
    {.kind = EventObject::Codeunit,
     .objectId = 0,
     .objectName = "Event Gate Publisher",
     .event = "OnBeforePost",
     .element = "",
     .parameters = kDoublesNames,
     .invoke = &agiru::detail::InvokeSubscriber<Listener_Codeunit, &Listener_Codeunit::Doubles>},
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
    {.kind = EventObject::Codeunit,
     .objectId = 0,
     .objectName = "Event Gate Publisher",
     .event = "OnBeforePost",
     .element = "",
     .parameters = kWrongNames,
     .invoke = &agiru::detail::InvokeSubscriber<Misnamed_Codeunit, &Misnamed_Codeunit::Wrong>},
}};
const SubscriptionCatalogue kMisnamedCatalogue{
    CodeunitTraits<Misnamed_Codeunit>::kId,
    CodeunitTraits<Misnamed_Codeunit>::kName,
    kMisnamedSubscriptions,
    true,
    []() -> void * { return new Misnamed_Codeunit(); },
    [](void *instance) { delete static_cast<Misnamed_Codeunit *>(instance); }};

constexpr std::array<std::string_view, 2> kHeardNames{"Sender", "RunTrigger"};
constexpr std::array<Subscription, 1> kToldSubscriptions{{
    {.kind = EventObject::Table,
     .objectId = 0,
     .objectName = "Line Number Buffer",
     .event = "OnBeforeInsertEvent",
     .element = "",
     .parameters = kHeardNames,
     .invoke = &agiru::detail::InvokeSubscriber<Told_Codeunit, &Told_Codeunit::Heard>},
}};
const SubscriptionCatalogue kToldCatalogue{
    CodeunitTraits<Told_Codeunit>::kId,
    CodeunitTraits<Told_Codeunit>::kName,
    kToldSubscriptions,
    true,
    []() -> void * { return new Told_Codeunit(); },
    [](void *instance) { delete static_cast<Told_Codeunit *>(instance); }};
constexpr std::array<std::string_view, 2> kWantsNames{"Sender", "Amount"};
constexpr std::array<Subscription, 1> kUnsentSubscriptions{{
    {.kind = EventObject::Codeunit,
     .objectId = 0,
     .objectName = "Event Gate Publisher",
     .event = "OnBeforePost",
     .element = "",
     .parameters = kWantsNames,
     .invoke = &agiru::detail::InvokeSubscriber<Unsent_Codeunit, &Unsent_Codeunit::Wants>},
}};
const SubscriptionCatalogue kUnsentCatalogue{
    CodeunitTraits<Unsent_Codeunit>::kId,
    CodeunitTraits<Unsent_Codeunit>::kName,
    kUnsentSubscriptions,
    true,
    []() -> void * { return new Unsent_Codeunit(); },
    [](void *instance) { delete static_cast<Unsent_Codeunit *>(instance); }};
constexpr std::array<std::string_view, 2> kSeenNames{"Rec", "RunTrigger"};
constexpr std::array<Subscription, 1> kWatcherSubscriptions{{
    {.kind = EventObject::Table,
     .objectId = 0,
     .objectName = "Line Number Buffer",
     .event = "OnBeforeInsertEvent",
     .element = "",
     .parameters = kSeenNames,
     .invoke = &agiru::detail::InvokeSubscriber<Watcher_Codeunit, &Watcher_Codeunit::Seen>},
}};
const SubscriptionCatalogue kWatcherCatalogue{
    CodeunitTraits<Watcher_Codeunit>::kId,
    CodeunitTraits<Watcher_Codeunit>::kName,
    kWatcherSubscriptions,
    true,
    []() -> void * { return new Watcher_Codeunit(); },
    [](void *instance) { delete static_cast<Watcher_Codeunit *>(instance); }};

/// The platform raises `OnBeforeInsertEvent` from `Insert`, with `Rec` the record itself, so a
/// subscriber writing a field writes what gets inserted (`devenv-event-types.md:109`).
void ASubscriberNamedSenderReceivesTheRaisingRecord() {
  Told_Codeunit told;
  static_cast<void>(agiru::BindSubscription(told));
  agiru::Temporary<agiru::app::tables::LineNumberBuffer> buffer;
  constexpr agiru::Integer kOldLine = 3;
  buffer.OldLineNumber = kOldLine;
  buffer.Insert(true);
  CHECK_TRUE("the subscriber was told once", told.told == 1);
  CHECK_TRUE("and what it wrote through Sender is the inserted row",
             buffer.Get(kOldLine) && buffer.NewLineNumber == kOldLine + 10);
  static_cast<void>(agiru::UnbindSubscription(told));

  // THE NEGATIVE CONTROL: a raise that carries no sender still refuses a `Sender` it cannot fill,
  // so the slot is filled only from a raise that names its object and never from thin air.
  Unsent_Codeunit unsent;
  static_cast<void>(agiru::BindSubscription(unsent));
  Publisher_Codeunit publisher;
  Integer amount = 1;
  Boolean handled = false;
  std::string said;
  try {
    publisher.OnBeforePost(amount, Text<0>{"x"}, handled);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a Sender on a raise without one refuses",
             said.find("names a parameter Sender") != std::string::npos);
  static_cast<void>(agiru::UnbindSubscription(unsent));
}

void ATableTriggerEventReachesASubscriber() {
  Watcher_Codeunit watcher;
  static_cast<void>(agiru::BindSubscription(watcher));
  agiru::Temporary<agiru::app::tables::LineNumberBuffer> buffer;
  constexpr agiru::Integer kOldLine = 7;
  constexpr agiru::Integer kNewLine = 8;
  buffer.OldLineNumber = kOldLine;
  buffer.Insert(true);
  CHECK_TRUE("Insert raised OnBeforeInsertEvent once", watcher.seen == 1);
  CHECK_TRUE("with RunTrigger as given", watcher.sawRunTrigger);
  CHECK_TRUE("and the subscriber's write to Rec is what was inserted",
             buffer.Get(kOldLine) && buffer.NewLineNumber == kNewLine);
  static_cast<void>(agiru::UnbindSubscription(watcher));
}

void AManualSubscriberHearsOnlyWhileBound() {
  Publisher_Codeunit publisher;
  Listener_Codeunit listener;
  constexpr Integer kAmount = 5;
  Integer amount = kAmount;
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
    ATableTriggerEventReachesASubscriber();
    ASubscriberNamedSenderReceivesTheRaisingRecord();
  });
}
