#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Events.h"
#include "runtime/Session.h"
#include "type/Integer.h"

#include "Check.h"

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace {

constexpr std::int32_t kPublisher = 50121;
constexpr std::array<std::string_view, 1> kSeenNames{"Seen"};
constexpr std::array<std::string_view, 2> kRecursiveNames{"Depth", "Seen"};

void Raise(std::string_view event, agiru::Integer &seen) {
  agiru::detail::RaiseEvent(
      agiru::EventObject::Codeunit, kPublisher, "Lifetime Publisher", event, kSeenNames, seen);
}

struct Subscriber {
  Subscriber() { ++made; }

  Subscriber(const Subscriber &) = delete;
  Subscriber &operator=(const Subscriber &) = delete;
  Subscriber(Subscriber &&) = delete;
  Subscriber &operator=(Subscriber &&) = delete;

  ~Subscriber() { ++destroyed; }

  void First(agiru::Integer &seen) { seen = ++number; }

  void Second(agiru::Integer &seen) { seen = seen * 10 + ++number; }

  void Fail(agiru::Integer &seen) {
    seen = ++number;
    throw agiru::Error("lifetime fixture failure");
  }

  void Recurse(agiru::Integer &depth, agiru::Integer &seen) {
    seen = seen * 10 + ++number;
    if (depth == 0) {
      depth = 1;
      agiru::detail::RaiseEvent(agiru::EventObject::Codeunit,
                                kPublisher,
                                "Lifetime Publisher",
                                "Recursive",
                                kRecursiveNames,
                                depth,
                                seen);
    }
  }

  agiru::Integer number = 0;
  static inline int made = 0;
  static inline int destroyed = 0;
};

template <auto Method>
constexpr agiru::Subscription Subscription(std::string_view event,
                                           std::span<const std::string_view> parameters) {
  return {.kind = agiru::EventObject::Codeunit,
          .objectId = kPublisher,
          .objectName = "Lifetime Publisher",
          .event = event,
          .element = "",
          .parameters = parameters,
          .invoke = &agiru::detail::InvokeSubscriber<Subscriber, Method>};
}

constexpr std::array kOrdinarySubscriptions{
    Subscription<&Subscriber::First>("Fresh", kSeenNames),
    Subscription<&Subscriber::Second>("Fresh", kSeenNames),
    Subscription<&Subscriber::Fail>("Failure", kSeenNames),
    Subscription<&Subscriber::Recurse>("Recursive", kRecursiveNames)};
constexpr std::array kSingleSubscriptions{Subscription<&Subscriber::First>("Single", kSeenNames),
                                          Subscription<&Subscriber::Second>("Single", kSeenNames)};

void *Make() {
  return new Subscriber;
}

void Free(void *instance) {
  delete static_cast<Subscriber *>(instance);
}

const agiru::SubscriptionCatalogue kOrdinary{agiru::CodeunitId{50122},
                                             "Ordinary Lifetime",
                                             kOrdinarySubscriptions,
                                             false,
                                             false,
                                             Make,
                                             Free};
const agiru::SubscriptionCatalogue kSingle{
    agiru::CodeunitId{50123}, "Single Lifetime", kSingleSubscriptions, false, true, Make, Free};

void FreshForEveryMethodAndRaise() {
  const int made = Subscriber::made;
  const int destroyed = Subscriber::destroyed;
  agiru::Integer seen = 0;
  Raise("Fresh", seen);
  CHECK_TRUE("two methods receive separate fresh instances", seen == 11);
  CHECK_TRUE("two methods construct two instances", Subscriber::made == made + 2);
  CHECK_TRUE("both instances are disposed before raise returns",
             Subscriber::destroyed == destroyed + 2);
  Raise("Fresh", seen);
  CHECK_TRUE("a repeated raise has no prior subscriber globals", seen == 11);
  CHECK_TRUE("a repeated raise constructs both instances anew", Subscriber::made == made + 4);
  CHECK_TRUE("a repeated raise immediately disposes both instances",
             Subscriber::destroyed == destroyed + 4);
}

void RecursiveRaisesHaveIndependentGlobals() {
  const int made = Subscriber::made;
  const int destroyed = Subscriber::destroyed;
  agiru::Integer seen = 0;
  agiru::Integer depth = 0;
  agiru::detail::RaiseEvent(agiru::EventObject::Codeunit,
                            kPublisher,
                            "Lifetime Publisher",
                            "Recursive",
                            kRecursiveNames,
                            depth,
                            seen);
  CHECK_TRUE("a recursive raise starts with independent globals", seen == 11);
  CHECK_TRUE("recursive invocations each construct an instance", Subscriber::made == made + 2);
  CHECK_TRUE("recursive invocations each dispose their instance",
             Subscriber::destroyed == destroyed + 2);
}

void ErrorDisposesTheInvocation() {
  const int made = Subscriber::made;
  const int destroyed = Subscriber::destroyed;
  agiru::Integer seen = 0;
  bool refused = false;
  try {
    Raise("Failure", seen);
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("ordinary subscriber errors propagate", refused);
  CHECK_TRUE("var output survives the subscriber error", seen == 1);
  CHECK_TRUE("a failing invocation constructs once", Subscriber::made == made + 1);
  CHECK_TRUE("a failing invocation is disposed before the caller catches",
             Subscriber::destroyed == destroyed + 1);
}

void SingleInstanceRemainsSessionOwned() {
  const int made = Subscriber::made;
  const int destroyed = Subscriber::destroyed;
  agiru::Integer seen = 0;
  {
    const agiru::Session parent(AGIRU_TEST_DSN);
    Raise("Single", seen);
    CHECK_TRUE("single instance methods share globals", seen == 12);
    CHECK_TRUE("single instance is made once", Subscriber::made == made + 1);
    CHECK_TRUE("single instance survives dispatch", Subscriber::destroyed == destroyed);
    {
      const agiru::Session child(AGIRU_TEST_DSN);
      Raise("Single", seen);
      CHECK_TRUE("a child session has its own automatic single instance", seen == 12);
    }
    CHECK_TRUE("child close disposes only its single instance",
               Subscriber::destroyed == destroyed + 1);
    Raise("Single", seen);
    CHECK_TRUE("parent globals survive child close", seen == 34);
  }
  CHECK_TRUE("parent close disposes its single instance", Subscriber::destroyed == destroyed + 2);
}

void IsolatedFailureDisposesTheInvocation() {
  const agiru::Session session(AGIRU_TEST_DSN);
  const int destroyed = Subscriber::destroyed;
  agiru::Integer seen = 0;
  agiru::detail::RaiseIsolatedEventFrom(nullptr,
                                        agiru::EventObject::Codeunit,
                                        kPublisher,
                                        "Lifetime Publisher",
                                        "Failure",
                                        "",
                                        kSeenNames,
                                        seen);
  CHECK_TRUE("isolated failure retains var output", seen == 1);
  CHECK_TRUE("isolated failure disposes its invocation before returning",
             Subscriber::destroyed == destroyed + 1);
}

}

int main() {
  return gate::Run("SubscriberLifetime", [] {
    FreshForEveryMethodAndRaise();
    RecursiveRaisesHaveIndependentGlobals();
    ErrorDisposesTheInvocation();
    SingleInstanceRemainsSessionOwned();
    IsolatedFailureDisposesTheInvocation();
  });
}
