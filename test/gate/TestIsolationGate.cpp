#include "meta/Ids.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

using agiru::CodeunitId;
using agiru::Error;
using agiru::Session;
using agiru::TestCatalogue;
using agiru::TestMethod;
using agiru::TransactionModel;

namespace {

// A TEST CODEUNIT THAT DEPENDS ON ITS OWN ORDER, which is what the W1 suite does: one method leaves
// a value behind and a later one reads it without setting it. `ERM General Journal UT` is the case
// the predecessor measured -- `SetJournalSimplePageModePreference(true)` at the end of one method,
// three later methods relying on it.
// IT WRITES TO THE DATABASE AND NOT TO A VARIABLE, because a C++ global is not what a rollback
// acts on -- a case built on one is green whichever isolation the runner uses, which is a gate that
// proves nothing.
std::string g_saw;

const agiru::Connection &Db() {
  return agiru::Session::Current().Database();
}

void Wrote(const char *who) {
  Db().Run(std::string("INSERT INTO isolation_gate (who) VALUES ('") + who + "')");
}

void DefaultLeaves() {
  Wrote("default");
}

void RollbackLeavesNothing() {
  Wrote("rollback");
}

void Fails() {
  Wrote("failed");
  throw Error("this method fails on purpose");
}

void Reads() {
  const agiru::Result rows = Db().Execute("SELECT who FROM isolation_gate ORDER BY who");
  g_saw.clear();
  for (std::size_t row = 0; row < rows.Rows(); ++row) {
    if (!g_saw.empty()) { g_saw += ","; }
    g_saw += rows.Value(row, 0).value_or("");
  }
}

constexpr std::array<TestMethod, 4> kOrdered{{
    {"DefaultLeaves", &DefaultLeaves, {}},
    {"RollbackLeavesNothing", &RollbackLeavesNothing, TransactionModel::AutoRollback},
    {"Fails", &Fails, {}},
    {"Reads", &Reads, {}},
}};

// THE ROLLBACK IS PER CODEUNIT AND NOT PER METHOD. `devenv-testisolation-property.md` gives three
// levels and BC's own CI runner -- codeunit 130450, `Test Runner - Isol. Codeunit` -- declares
// `TestIsolation = Codeunit`. The predecessor measured what running per METHOD costs: the same
// codeunit was 164 of 190 green per method and 179 of 190 per codeunit, and 18 of its 26 red
// messages came from the measurement rather than from the code (openerp WI-1088, WI-963).
//
// AND AN ABSENT `[TransactionModel]` IS NOT `AutoRollback`. The attribute's page says a declared
// `AutoRollback` rolls the method back, and that holds -- but it says nothing about a method that
// declares none, and there the runner's `TestIsolation` is what decides. 4 221 of 4 293 W1 methods
// declare one, so reading the absent case as `AutoRollback` looks harmless and silently discards
// exactly the writes the ordered codeunits depend on.
void WhatOneMethodLeavesTheNextOneSees() {
  Db().Run("DROP TABLE IF EXISTS isolation_gate");
  Db().Run("CREATE TABLE isolation_gate (who text NOT NULL)");
  const TestCatalogue registered{CodeunitId{999999}, "Gate - Ordered UT", nullptr, kOrdered};
  g_saw.clear();
  const agiru::TestRun run = agiru::RunRegisteredTests("Gate - Ordered UT");
  CHECK_TRUE("all four methods ran", run.passed + run.failed == 4);
  CHECK_TRUE("the one that raises is the only red", run.failed == 1);
  CHECK_TRUE("a failing method does not stop the next", run.passed == 3);
  CHECK_TEXT("what a method without the attribute wrote, a later one reads", g_saw, "default");

  // AND THE CODEUNIT'S OWN BOUNDARY TAKES IT ALL BACK, including a row a method committed -- the
  // property's page says so outright.
  const agiru::Result left = Db().Execute("SELECT count(*) FROM isolation_gate");
  CHECK_TRUE("and the codeunit leaves the database where it found it",
             left.Value(0, 0).has_value() && *left.Value(0, 0) == "0");
  Db().Run("DROP TABLE isolation_gate");
}

} // namespace

int main() {
  return gate::Run("TestIsolation", [] {
    try {
      const Session session(AGIRU_TEST_DSN);
      WhatOneMethodLeavesTheNextOneSees();
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}
