#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/Transaction.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Time.h"

#include "BuiltinsWritten.h"
#include "Check.h"
#include "OwnedDatabase.h"
#include "SessionState.h"

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

namespace {

constexpr agiru::Integer kSampleBound = 1000000;
constexpr agiru::Integer kClockSeed = 12345;
constexpr agiru::Integer kFirstSeed = 11;
constexpr agiru::Integer kSecondSeed = 99;
constexpr std::size_t kSamples = 8;
using Draws = std::array<agiru::Integer, kSamples>;

Draws Draw() {
  Draws values{};
  for (auto &value : values) { value = agiru::Random(kSampleBound); }
  return values;
}

Draws Seeded(agiru::Integer seed) {
  agiru::Randomize(seed);
  return Draw();
}

template <typename Body> void Worker(Body body) {
  std::exception_ptr failure;
  std::thread worker([&] {
    try {
      body();
    } catch (...) { failure = std::current_exception(); }
  });
  worker.join();
  if (failure) { std::rethrow_exception(failure); }
}

void MissingSession() {
  const auto refuses = [](auto body) {
    try {
      body();
    } catch (const agiru::SessionError &) { return true; }
    return false;
  };
  CHECK_TRUE("ApplicationArea requires active session authority",
             refuses([] { static_cast<void>(agiru::ApplicationArea()); }));
  CHECK_TRUE("Randomize requires active session authority", refuses([] { agiru::Randomize(11); }));
  CHECK_TRUE("Random requires active session authority",
             refuses([] { static_cast<void>(agiru::Random(100)); }));
}

void SeedAndBounds(const std::string &dsn) {
  const agiru::Session session(dsn);
  constexpr std::array seeds{0,
                             1,
                             kFirstSeed,
                             -kFirstSeed,
                             std::numeric_limits<agiru::Integer>::max(),
                             std::numeric_limits<agiru::Integer>::min()};
  for (const auto seed : seeds) {
    const auto reference = Seeded(seed);
    CHECK_TRUE("every explicit signed seed repeats its complete sequence",
               Seeded(seed) == reference);
  }
  CHECK_TRUE("explicit seed zero is not silently replaced with one", Seeded(0) != Seeded(1));
  agiru::Randomize(kFirstSeed);
  CHECK_TRUE("the existing seed-eleven positive draw is preserved", agiru::Random(100) == 48);
  constexpr std::array bounds{0,
                              1,
                              -1,
                              100,
                              -100,
                              std::numeric_limits<agiru::Integer>::max(),
                              -std::numeric_limits<agiru::Integer>::max()};
  for (const auto bound : bounds) {
    const agiru::Integer maximum = bound == 0 ? 1 : (bound < 0 ? -bound : bound);
    agiru::Randomize(kFirstSeed);
    const auto value = agiru::Random(bound);
    CHECK_TRUE("Random accepts representable inclusive positive or negative bounds",
               value >= 1 && value <= maximum);
    agiru::Randomize(kFirstSeed);
    CHECK_TRUE("negative and positive bounds preserve exactly the same draw",
               agiru::Random(maximum) == value);
  }
  bool refused = false;
  try {
    static_cast<void>(agiru::Random(std::numeric_limits<agiru::Integer>::min()));
  } catch (const agiru::Error &error) { refused = error.Code() == "RandomBoundRange"; }
  CHECK_TRUE("unqualified indirect-minimum magnitude refuses rather than claiming a draw", refused);
}

void Nested(const std::string &dsn) {
  const agiru::Session outer(dsn);
  CHECK_TRUE("idle session state does not allocate a random generator",
             agiru::detail::SessionState::Current().random == nullptr);
  CHECK_TEXT("a new session starts with an empty application area", agiru::ApplicationArea(), "");
  agiru::ApplicationArea("#Basic,Ä");
  CHECK_TRUE("application-area-only use leaves random state unallocated",
             agiru::detail::SessionState::Current().random == nullptr);
  const auto reference = Seeded(kFirstSeed);
  const auto *sequence = agiru::detail::SessionState::Current().random.get();
  CHECK_TRUE("the first draw allocates a session-owned generator", sequence != nullptr);
  agiru::Randomize(kFirstSeed);
  CHECK_TRUE("reseed reuses this session's existing allocation",
             agiru::detail::SessionState::Current().random.get() == sequence);
  CHECK_TRUE("the outer session starts the seeded sequence",
             agiru::Random(kSampleBound) == reference[0]);
  {
    const agiru::Session child(dsn);
    CHECK_TEXT("a nested session cannot inherit the worker's application area",
               agiru::ApplicationArea(),
               "");
    agiru::ApplicationArea("#Suite");
    static_cast<void>(Seeded(kSecondSeed));
  }
  CHECK_TRUE("nested session teardown restores the actual outer authority",
             &agiru::Session::Current() == &outer);
  CHECK_TEXT("nested application-area changes do not overwrite the outer session",
             agiru::ApplicationArea(),
             "#Basic,Ä");
  CHECK_TRUE("nested randomization cannot change the outer sequence",
             agiru::Random(kSampleBound) == reference[1]);
  agiru::ApplicationArea("");
  CHECK_TEXT(
      "explicit empty application area clears rather than reads", agiru::ApplicationArea(), "");
}

agiru::Guid Identity() {
  std::array<std::uint8_t, agiru::Guid::kSize> bytes{};
  bytes.back() = 1;
  return agiru::Guid(bytes);
}

void SeedUser(const std::string &dsn) {
  const agiru::Session seed(dsn);
  agiru::CreateTable(seed.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
  agiru::platform::User user;
  user.UserSecurityID = Identity();
  user.UserName = "Session values";
  user.Insert();
  agiru::Session::Current().Transaction().Commit(seed.Database());
}

void Workers(const std::string &dsn) {
  SeedUser(dsn);
  Draws firstReference{};
  Draws secondReference{};
  {
    const agiru::Session reference(dsn);
    firstReference = Seeded(kFirstSeed);
    secondReference = Seeded(kSecondSeed);
  }
  agiru::Session first(Identity());
  agiru::Session second(Identity());

  struct Results {
    bool firstStarted = false;
    bool secondFresh = false;
    bool secondStarted = false;
    bool firstReused = false;
    bool idle = false;
    bool firstMigrated = false;
    bool secondMigrated = false;
  } results;

  Worker([&] {
    agiru::Connection connection(dsn);
    {
      agiru::SessionCommand command(first, connection);
      agiru::ApplicationArea("#Basic,Ä");
      agiru::Randomize(kFirstSeed);
      results.firstStarted = agiru::Random(kSampleBound) == firstReference[0];
      command.Keep();
    }
    {
      agiru::SessionCommand command(second, connection);
      results.secondFresh = agiru::ApplicationArea().empty();
      agiru::ApplicationArea("#Suite");
      agiru::Randomize(kSecondSeed);
      results.secondStarted = agiru::Random(kSampleBound) == secondReference[0];
      command.Keep();
    }
    {
      agiru::SessionCommand command(first, connection);
      results.firstReused = agiru::ApplicationArea() == "#Basic,Ä" &&
                            agiru::Random(kSampleBound) == firstReference[1];
      command.Keep();
    }
    results.idle = !agiru::Session::HasCurrent() && !connection.InTransaction();
  });
  Worker([&] {
    agiru::Connection connection(dsn);
    {
      agiru::SessionCommand command(first, connection);
      results.firstMigrated = agiru::ApplicationArea() == "#Basic,Ä" &&
                              agiru::Random(kSampleBound) == firstReference[2];
      command.Keep();
    }
    {
      agiru::SessionCommand command(second, connection);
      results.secondMigrated =
          agiru::ApplicationArea() == "#Suite" && agiru::Random(kSampleBound) == secondReference[1];
      command.Keep();
    }
  });
  CHECK_TRUE("first worker starts the first logical session's sequence", results.firstStarted);
  CHECK_TRUE("reused worker starts another session with its own empty area", results.secondFresh);
  CHECK_TRUE("reused worker starts the second logical session's sequence", results.secondStarted);
  CHECK_TRUE("worker reuse preserves the first session's area and sequence", results.firstReused);
  CHECK_TRUE("completed commands retain neither worker authority nor SQL transaction",
             results.idle);
  CHECK_TRUE("worker migration preserves the first session's area and sequence",
             results.firstMigrated);
  CHECK_TRUE("worker migration preserves another same-user session independently",
             results.secondMigrated);
}

void Concurrent(const std::string &dsn) {
  constexpr std::size_t kWorkers = 2;
  std::array<Draws, kWorkers> expected{};
  {
    const agiru::Session reference(dsn);
    for (std::size_t at = 0; at < kWorkers; ++at) {
      expected[at] = Seeded(static_cast<agiru::Integer>(at) + kFirstSeed);
    }
  }
  agiru::Session first(Identity());
  agiru::Session second(Identity());
  const std::array sessions{&first, &second};
  std::array<bool, kWorkers> results{};
  std::array<std::exception_ptr, kWorkers> failures{};
  std::array<std::thread, kWorkers> workers;
  std::mutex mutex;
  std::condition_variable ready;
  std::size_t arrived = 0;
  bool aborted = false;
  for (std::size_t at = 0; at < kWorkers; ++at) {
    workers[at] = std::thread([&, at] {
      try {
        agiru::Connection connection(dsn);
        agiru::SessionCommand command(*sessions[at], connection);
        const auto area = "#Worker" + std::to_string(at);
        agiru::ApplicationArea(area);
        agiru::Randomize(static_cast<agiru::Integer>(at) + kFirstSeed);
        const auto firstDraw = agiru::Random(kSampleBound);
        {
          std::unique_lock lock(mutex);
          ++arrived;
          ready.notify_all();
          if (!ready.wait_for(
                  lock, std::chrono::seconds(10), [&] { return arrived == kWorkers || aborted; }) ||
              aborted) {
            throw agiru::Error("concurrent session-value fixture failed or timed out");
          }
        }
        results[at] = agiru::ApplicationArea() == area && firstDraw == expected[at][0] &&
                      agiru::Random(kSampleBound) == expected[at][1];
        command.Keep();
      } catch (...) {
        failures[at] = std::current_exception();
        const std::lock_guard lock(mutex);
        aborted = true;
        ready.notify_all();
      }
    });
  }
  for (auto &worker : workers) { worker.join(); }
  for (std::size_t at = 0; at < kWorkers; ++at) {
    if (failures[at]) { std::rethrow_exception(failures[at]); }
    CHECK_TRUE("concurrent same-user sessions retain independent area and random state",
               results[at]);
  }
}

void Clock(const std::string &dsn, bool fixed) {
  const agiru::Session session(dsn);
  if (fixed) {
    CHECK_TRUE("the authored clock provider is active",
               agiru::CurrentTime().AsMilliseconds() == kClockSeed);
    const auto expected = Seeded(kClockSeed);
    agiru::Randomize();
    CHECK_TRUE("parameterless Randomize uses the current clock seed", Draw() == expected);
    return;
  }
  const auto before = agiru::CurrentTime().AsMilliseconds();
  agiru::Randomize();
  const auto actual = Draw();
  const auto after = agiru::CurrentTime().AsMilliseconds();
  const auto elapsed =
      (after - before + agiru::Time::kMillisecondsPerDay) % agiru::Time::kMillisecondsPerDay;
  constexpr auto kClockBudget = 1000;
  CHECK_TRUE("the native clock observation stays within its bounded test budget",
             elapsed <= kClockBudget);
  if (elapsed > kClockBudget) { return; }
  bool matched = false;
  for (agiru::Integer offset = 0; offset <= elapsed; ++offset) {
    const auto seed = (before + offset) % agiru::Time::kMillisecondsPerDay;
    matched = matched || Seeded(seed) == actual;
  }
  CHECK_TRUE("parameterless Randomize agrees with the observed native clock", matched);
}

}

int main(int argc, char **argv) {
  return gate::Run("SessionValues", [&] {
    const bool fixed = argc == 2 && std::string_view(argv[1]) == "--clock-fixture";
    if (argc != 1 && !fixed) { throw agiru::Error("unknown SessionValues gate argument"); }
    MissingSession();
    const gate::OwnedDatabase database("session_values");
    SeedAndBounds(database.Dsn());
    Nested(database.Dsn());
    Workers(database.Dsn());
    Concurrent(database.Dsn());
    Clock(database.Dsn(), fixed);
  });
}
