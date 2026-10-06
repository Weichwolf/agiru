#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/codeunit/ForLoopConsumer.h"

#include <array>
#include <stdexcept>
#include <string_view>

namespace {
using Consumer = agiru::Fixture::ForLoopConsumer_Codeunit;

void IntegerBounds() {
  Consumer consumer;
  CHECK_TEXT("the bound dequeues the count, not the following typed lot values",
             consumer.QueuedValues(),
             "LOT1LOT2");
  CHECK_TRUE("the loop consumes exactly the declared typed queue values",
             consumer.RemainingQueue() == 0);
  for (const bool descending : std::array{false, true}) {
    agiru::Integer trace = 0;
    agiru::Integer calls = 0;
    CHECK_TRUE("integer bounds preserve the inclusive iteration values",
               consumer.OrderedBounds(trace, calls, descending) == 6);
    CHECK_TRUE("integer bounds execute exactly once each", calls == 2);
    CHECK_TRUE("start executes before the captured end", trace == (descending ? 31 : 13));
    CHECK_TRUE("body mutation does not change the captured integer bound",
               consumer.MutableBound(descending) == 6);
    trace = 0;
    calls = 0;
    CHECK_TRUE("empty ranges never execute the body",
               consumer.EmptyBounds(trace, calls, descending) == 0);
    CHECK_TRUE("empty ranges still evaluate both bounds once", calls == 2);
    CHECK_TRUE("empty ranges retain bound effects in source order",
               trace == (descending ? 13 : 31));
  }
}

void BooleanAndOwnedCounters() {
  Consumer consumer;
  for (const bool descending : std::array{false, true}) {
    agiru::Integer calls = 0;
    CHECK_TRUE("Boolean loops retain false/true values and captured bounds",
               consumer.BooleanBounds(calls, descending) == (descending ? 21 : 12));
    CHECK_TRUE("Boolean end expressions execute exactly once", calls == 1);
  }
  CHECK_TRUE("global and nested counters retain their actual declared values",
             consumer.GlobalAndNested() == 54);
  agiru::Integer counter = 0;
  CHECK_TRUE("var counters are visible during every iteration", consumer.VarCounter(counter) == 6);
  CHECK_TRUE("continue increments and break exits only its loop",
             consumer.ControlTransfers() == 20);
  CHECK_TRUE("BigInteger bounds retain values beyond binary-double precision",
             consumer.ExactBigInteger() == 18014398509481987LL);
  CHECK_TRUE("Option bounds retain their captured ordinal and declared counter",
             consumer.OptionBounds() == 123);
  CHECK_TRUE("generated bound and Boolean step names cannot shadow AL variables",
             consumer.TemporaryNames() == 11);
}

void FailedBounds() {
  Consumer consumer;
  for (const bool failFirst : std::array{false, true}) {
    agiru::Integer trace = 0;
    agiru::Integer calls = 0;
    bool raised = false;
    try {
      consumer.FailedBounds(trace, calls, failFirst);
    } catch (const agiru::Error &error) {
      raised = std::string_view(error.what()) == "Loop bound failed";
    }
    CHECK_TRUE("a failed bound propagates before body execution", raised);
    CHECK_TRUE("a failed start prevents end evaluation", calls == (failFirst ? 1 : 2));
    CHECK_TRUE("failed bounds retain only their preceding effects", trace == (failFirst ? 7 : 17));
  }
}
}

int main(int argc, char **argv) {
  return gate::Run("Generated For Loops", [argc, argv] {
    if (argc != 2) { throw std::runtime_error("expected the dedicated gate database DSN"); }
    const agiru::Session session(argv[1]);
    IntegerBounds();
    BooleanAndOwnedCounters();
    FailedBounds();
  });
}
