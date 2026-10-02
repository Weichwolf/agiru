#include "runtime/Codeunit.h"
#include "platform/Integer.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Events.h"
#include "runtime/Session.h"
#include "runtime/Scopes.h"
#include "runtime/Table.h"
#include "runtime/Transaction.h"
#include "LineNumberBuffer.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <span>
#include <string_view>

namespace {
constexpr auto kDsn = "postgresql://agiru:agiru@localhost:5433/agiru_gate";
int freed = 0;
void *Make() { return new int(0); }
void Free(void *value) { ++freed; delete static_cast<int *>(value); }

void FailSubscriber(void *, const agiru::EventArgs &, std::span<const std::size_t>) {
  throw agiru::Error("review subscriber failure");
}

const std::array<agiru::Subscription, 1> kSubscriptions{{
    {.kind = agiru::EventObject::Codeunit,
     .objectId = 59001,
     .objectName = "Review publisher",
     .event = "Failure",
     .element = "",
     .parameters = {},
     .invoke = FailSubscriber}}};
const agiru::SubscriptionCatalogue kCatalogue{
    agiru::CodeunitId{59002}, "Review subscriber", kSubscriptions, false, false, Make, Free};

int SessionOwnership() {
  agiru::Session parent(kDsn);
  auto *parentValue = static_cast<int *>(agiru::detail::SingleInstanceOf(
      agiru::CodeunitId{59000}, Make, Free));
  *parentValue = 42;
  bool shared = false;
  const int before = freed;
  {
    agiru::Session child(kDsn);
    const auto *childValue = static_cast<int *>(agiru::detail::SingleInstanceOf(
        agiru::CodeunitId{59000}, Make, Free));
    shared = childValue == parentValue;
    std::printf("nested session shares parent instance: %s; child value: %d\n",
                shared ? "yes" : "no", *childValue);
  }
  const bool destroyed = freed != before;
  const auto *restored = static_cast<int *>(agiru::detail::SingleInstanceOf(
      agiru::CodeunitId{59000}, Make, Free));
  std::printf("child destroyed parent instance: %s; restored parent value: %d\n",
              destroyed ? "yes" : "no", *restored);
  return shared || destroyed || *restored != 42 ? 1 : 0;
}

int NavigationAndEvents() {
  agiru::Session session(kDsn);
  session.Database().Run("CREATE TEMP TABLE \"Line Number Buffer\" "
                         "(\"Old Line Number\" integer PRIMARY KEY, \"New Line Number\" integer NOT NULL)");
  agiru::detail::Scope transaction;
  session.Database().Run("INSERT INTO \"Line Number Buffer\" VALUES (1,10),(2,20),(3,30)");
  using Row = agiru::app::tables::LineNumberBuffer;
  int failed = 0;
  {
    agiru::CommitScope outer(agiru::CommitBehavior::Error);
    agiru::CommitScope inner(agiru::CommitBehavior::Ignore);
    bool rejected = false;
    try { agiru::Commit(); } catch (const agiru::Error &) { rejected = true; }
    std::printf("nested Ignore preserves outer Error on Commit: %s\n", rejected ? "yes" : "no");
    failed += !rejected;
  }
  {
    Row row;
    if (!row.FindSet()) { throw agiru::Error("probe setup failed"); }
    const int before = row.OldLineNumber;
    const int steps = row.Next(0);
    const int after = row.OldLineNumber;
    std::printf("SQL Next(0): before=%d after=%d returned=%d\n", before, after, steps);
    failed += before != after || steps != 0;
  }
  {
    agiru::Temporary<Row> row;
    for (int i = 1; i <= 3; ++i) {
      row.OldLineNumber = i;
      row.NewLineNumber = i * 10;
      row.Insert();
    }
    if (!row.FindSet()) { throw agiru::Error("temporary probe setup failed"); }
    const int before = row.OldLineNumber;
    const int steps = row.Next(0);
    const int after = row.OldLineNumber;
    std::printf("temporary Next(0): before=%d after=%d returned=%d\n", before, after, steps);
    failed += before != after || steps != 0;
  }
  {
    Row row;
    row.Ascending(false);
    if (!row.FindFirst()) { throw agiru::Error("descending probe setup failed"); }
    const int first = row.OldLineNumber;
    const int moved = row.Next();
    const int second = row.OldLineNumber;
    std::printf("descending Next after FindFirst: first=%d next=%d returned=%d; expected 3,2,1\n",
                first, second, moved);
    failed += first != 3 || second != 2 || moved != 1;
    const int back = row.Next(-1);
    const int previous = row.OldLineNumber;
    std::printf("descending Next(-1): from=%d to=%d returned=%d; expected 2,3,-1\n",
                second, previous, back);
    failed += previous != 3 || back != -1;
  }
  bool caught = false;
  try {
    agiru::detail::RaiseIsolated(agiru::EventObject::Codeunit, 59001,
                                "Review publisher", "Failure", "", agiru::EventArgs{});
  } catch (const agiru::Error &) { caught = true; }
  std::printf("isolated subscriber error propagates inside write transaction: %s\n",
              caught ? "yes" : "no");
  failed += !caught;
  return failed;
}

int IntegerPopulation() {
  agiru::Session session(kDsn);
  agiru::platform::Integer row;
  row.SetRange(row.FieldNo(row.Number), 1, 3);
  const int small = row.Count();
  row.SetRange(row.FieldNo(row.Number), 1, 1000001);
  const int large = row.Count();
  std::printf("virtual Integer Count: small=%d expected=3; large=%d expected=1000001\n", small, large);
  return small != 3 || large != 1000001;
}
}

int main() {
  try {
    const int failures = SessionOwnership() + NavigationAndEvents() + IntegerPopulation();
    std::printf("review contract failures: %d\n", failures);
    return failures == 0 ? 0 : 1;
  } catch (const agiru::Error &error) {
    std::fprintf(stderr, "probe error: %s\n", error.what());
    return 2;
  }
}
