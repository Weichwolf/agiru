#include "meta/Ids.h"
#include "runtime/Session.h"
#include "runtime/SingleInstance.h"
#include "type/Integer.h"

#include "Check.h"

#include <atomic>
#include <thread>

namespace {

constexpr agiru::CodeunitId kFixtureId{50119};
constexpr agiru::Integer kParentValue = 42;
constexpr agiru::Integer kChildValue = 17;

struct Value {
  Value() = default;
  Value(const Value &) = delete;
  Value &operator=(const Value &) = delete;
  Value(Value &&) = delete;
  Value &operator=(Value &&) = delete;
  agiru::Integer number = 0;

  ~Value() { destroyed.fetch_add(1); }

  static std::atomic<int> destroyed;
};

std::atomic<int> Value::destroyed{};

Value &CurrentValue() {
  return *static_cast<Value *>(agiru::detail::SingleInstanceOf(
      kFixtureId,
      []() -> void * { return new Value; },
      [](void *held) { delete static_cast<Value *>(held); }));
}

void NestedSessionsKeepTheirOwnInstances() {
  const int before = Value::destroyed.load();
  {
    const agiru::Session parent(AGIRU_TEST_DSN);
    Value &original = CurrentValue();
    original.number = kParentValue;
    CHECK_TRUE("lookup in one session returns one instance", &CurrentValue() == &original);
    {
      const agiru::Session child(AGIRU_TEST_DSN);
      Value &nested = CurrentValue();
      CHECK_TRUE("a nested session never borrows the parent's instance", &nested != &original);
      CHECK_TRUE("a nested session starts with its own default state", nested.number == 0);
      nested.number = kChildValue;
      CHECK_TRUE("the parent state survives child writes", original.number == kParentValue);
    }
    CHECK_TRUE("child close frees only the child instance", Value::destroyed.load() == before + 1);
    CHECK_TRUE("parent lookup after child close preserves state",
               CurrentValue().number == kParentValue);
  }
  CHECK_TRUE("parent close frees its own instance once", Value::destroyed.load() == before + 2);
  {
    const agiru::Session reusedWorker(AGIRU_TEST_DSN);
    CHECK_TRUE("a later session on the same worker starts clean", CurrentValue().number == 0);
  }
  CHECK_TRUE("a later session releases only its own instance",
             Value::destroyed.load() == before + 3);
}

void SeparateWorkersNeverShareInstances() {
  const agiru::Session parent(AGIRU_TEST_DSN);
  CurrentValue().number = kParentValue;
  int observed = -1;
  std::thread worker([&observed] {
    const agiru::Session other(AGIRU_TEST_DSN);
    observed = CurrentValue().number;
    CurrentValue().number = kChildValue;
  });
  worker.join();
  CHECK_TRUE("a separate worker starts clean", observed == 0);
  CHECK_TRUE("a separate worker cannot change the caller's value",
             CurrentValue().number == kParentValue);
}

void NoSessionRefusesLookup() {
  bool refused = false;
  try {
    static_cast<void>(CurrentValue());
  } catch (const agiru::SessionError &) { refused = true; }
  if (!refused) { agiru::detail::ReleaseSingleInstances(); }
  CHECK_TRUE("a single-instance lookup requires an active session", refused);
}

}

int main() {
  return gate::Run("SessionInstance", [] {
    NestedSessionsKeepTheirOwnInstances();
    SeparateWorkersNeverShareInstances();
    NoSessionRefusesLookup();
  });
}
