#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"
#include "runtime/Transaction.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/PageCore.h"
#include "runtime/test/TestPermissions.h"
#include "type/TransactionModel.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

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
constexpr agiru::Integer kTrapPage = 999998; // [SET] synthetic page owned by this fixture.

struct ForeignFailure {};

void LeaveTrap() {
  agiru::detail::TrapPage(kTrapPage, nullptr, +[](void *, void *, bool) {});
}

const agiru::Connection &Db() {
  return agiru::Session::Current().Database();
}

void Wrote(const char *who) {
  Db().Run(std::string("INSERT INTO isolation_gate (who) VALUES ('") + who + "')");
}

void ForeignException(void *) {
  Wrote("foreign");
  LeaveTrap();
  throw ForeignFailure{};
}

void DefaultLeaves(void *) {
  Wrote("default");
  agiru::Commit();
}

void ImplicitLeaves(void *) {
  Wrote("implicit");
}

void RollbackLeavesNothing(void *) {
  Wrote("rollback");
}

void NestedCommitSurvivesTheInnerError(void *) {
  agiru::AssertError([] {
    Wrote("inner_committed");
    agiru::Commit();
    Wrote("inner_discarded");
    throw Error("discard only writes after the inner Commit");
  });
}

void AutoRollbackRefusesCommit(void *) {
  bool refused = false;
  try {
    agiru::Commit();
  } catch (const Error &) { refused = true; }
  if (!refused) { throw Error("AutoRollback accepted Commit"); }
}

void MissedHandlerWrites(void *) {
  Wrote("missed");
}

void Fails(void *) {
  Wrote("failed");
  throw Error("this method fails on purpose");
}

void Reads(void *) {
  const agiru::Result rows = Db().Execute("SELECT who FROM isolation_gate ORDER BY who");
  g_saw.clear();
  for (std::size_t row = 0; row < rows.Rows(); ++row) {
    if (!g_saw.empty()) { g_saw += ","; }
    g_saw += rows.Value(row, 0).value_or("");
  }
  LeaveTrap();
}

void *MakeNothing() {
  return nullptr;
}

void FreeNothing(void *) {}

constexpr std::array<std::string_view, 1> kUnusedHandler{"NeverCalled"};
constexpr std::array<TestMethod, 7> kOrdered{{
    {.name = "DefaultLeaves",
     .invoke = &DefaultLeaves,
     .model = {},
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "RollbackLeavesNothing",
     .invoke = &RollbackLeavesNothing,
     .model = TransactionModel::AutoRollback,
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "NestedCommitSurvivesTheInnerError",
     .invoke = &NestedCommitSurvivesTheInnerError,
     .model = {},
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "AutoRollbackRefusesCommit",
     .invoke = &AutoRollbackRefusesCommit,
     .model = TransactionModel::AutoRollback,
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "MissedHandlerWrites",
     .invoke = &MissedHandlerWrites,
     .model = {},
     .handlers = kUnusedHandler,
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "Fails",
     .invoke = &Fails,
     .model = {},
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "Reads",
     .invoke = &Reads,
     .model = {},
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
}};

constexpr std::array<TestMethod, 3> kPolicyMethods{{
    {.name = "DefaultLeaves",
     .invoke = &DefaultLeaves,
     .model = {},
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "ImplicitLeaves",
     .invoke = &ImplicitLeaves,
     .model = TransactionModel::AutoCommit,
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
    {.name = "Reads",
     .invoke = &Reads,
     .model = {},
     .handlers = {},
     .permissions = agiru::TestPermissions::Restrictive},
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
  static constexpr agiru::CodeunitDef orderedDefinition{
      .id = CodeunitId{999999}, .name = "Gate - Ordered UT", .subtype = agiru::Subtype::Test};
  static constexpr agiru::CodeunitDef foreignDefinition{
      .id = CodeunitId{999998}, .name = "Gate - Foreign UT", .subtype = agiru::Subtype::Test};
  static constexpr agiru::CodeunitDef policyDefinition{
      .id = CodeunitId{999997}, .name = "Gate - Policy UT", .subtype = agiru::Subtype::Test};
  const TestCatalogue registered{orderedDefinition, &MakeNothing, &FreeNothing, nullptr, kOrdered};
  constexpr std::array<TestMethod, 1> foreignMethods{
      {{.name = "ForeignException",
        .invoke = &ForeignException,
        .model = {},
        .handlers = {},
        .permissions = agiru::TestPermissions::Restrictive}}};
  const TestCatalogue foreign{
      foreignDefinition, &MakeNothing, &FreeNothing, nullptr, foreignMethods};
  const TestCatalogue policy{policyDefinition, &MakeNothing, &FreeNothing, nullptr, kPolicyMethods};
  g_saw.clear();
  const agiru::TestRun run = agiru::RunRegisteredTests("Gate - Ordered UT");
  CHECK_TRUE("all seven methods ran", run.passed + run.failed == 7);
  CHECK_TRUE("the throwing method and unused handler both fail", run.failed == 2);
  CHECK_TRUE("a failed method does not stop the next", run.passed == 5);
  CHECK_TEXT("Commit survives an inner error while failed writes do not",
             g_saw,
             "default,inner_committed");
  CHECK_TRUE("no page trap survives the last successful method",
             !agiru::detail::TrapPending(kTrapPage));
  agiru::detail::ClearTraps();
  bool propagated = false;
  try {
    static_cast<void>(agiru::RunRegisteredTests("Gate - Foreign UT"));
  } catch (const ForeignFailure &) { propagated = true; }
  CHECK_TRUE("foreign exceptions remain visible", propagated);
  CHECK_TRUE("foreign exceptions detach handlers", !agiru::HandlerTable::Installed());
  CHECK_TRUE("foreign exceptions release page traps", !agiru::detail::TrapPending(kTrapPage));
  CHECK_TRUE("foreign exceptions release the test instance",
             agiru::CurrentTestInstance() == nullptr);
  static_cast<void>(agiru::HandlerTable::Uninstall());
  agiru::detail::ClearTraps();

  // AND THE CODEUNIT'S OWN BOUNDARY TAKES IT ALL BACK, including a row a method committed -- the
  // property's page says so outright.
  const agiru::Result left = Db().Execute("SELECT count(*) FROM isolation_gate");
  const std::optional<std::string_view> counted = left.Value(0, 0);
  CHECK_TRUE("and the codeunit leaves the database where it found it",
             counted.has_value() && *counted == "0");
  g_saw.clear();
  const agiru::TestRun functionRun =
      agiru::RunRegisteredTests("Gate - Policy UT", agiru::TestIsolation::Function);
  CHECK_TRUE("Function isolation runs all methods", functionRun.passed == 3);
  CHECK_TEXT(
      "Function isolation rolls back even an explicit Commit before the next method", g_saw, "");
  const agiru::Result functionLeft = Db().Execute("SELECT count(*) FROM isolation_gate");
  CHECK_TEXT("Function isolation leaves no committed row",
             std::string(functionLeft.Value(0, 0).value_or("")),
             "0");
  g_saw.clear();
  const agiru::TestRun disabledRun =
      agiru::RunRegisteredTests("Gate - Policy UT", agiru::TestIsolation::Disabled);
  CHECK_TRUE("Disabled isolation runs all methods", disabledRun.passed == 3);
  CHECK_TEXT("Disabled isolation lets the next method read both writes", g_saw, "default,implicit");
  const agiru::Result disabledLeft = Db().Execute("SELECT count(*) FROM isolation_gate");
  CHECK_TEXT("Disabled isolation keeps the committed row",
             std::string(disabledLeft.Value(0, 0).value_or("")),
             "2");
  const agiru::Connection observer(AGIRU_TEST_DSN);
  const agiru::Result visible = observer.Execute("SELECT count(*) FROM isolation_gate");
  CHECK_TEXT("a second connection sees Disabled's implicit method commit",
             std::string(visible.Value(0, 0).value_or("")),
             "2");
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
