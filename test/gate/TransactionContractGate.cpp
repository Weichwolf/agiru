#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Events.h"
#include "runtime/Report.h"
#include "runtime/Scopes.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/Transaction.h"
#include "runtime/XmlPort.h"
#include "type/CommitBehavior.h"
#include "type/ErrorBehavior.h"
#include "type/ErrorInfo.h"
#include "type/IsolatedStorage.h"
#include "type/StringValue.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "ResourceCost.h"
#include "options/Types.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace {
class Routine;
class TransactionReport;
class TransactionXml;
}

template <> struct agiru::CodeunitTraits<Routine> {
  static constexpr agiru::CodeunitId kId{50301};
  static constexpr std::string_view kName = "Transaction contract";
};

template <> struct agiru::ReportTraits<TransactionReport> {
  static constexpr agiru::ReportId kId{50302};
  static constexpr std::string_view kName = "Transaction report";
};

template <> struct agiru::XmlPortTraits<TransactionXml> {
  static constexpr agiru::XmlPortId kId{50305};
  static constexpr std::string_view kName = "Transaction XMLport";
  static constexpr agiru::XmlPortDef kPort{
      .id = kId, .name = kName, .useRequestPage = false, .rootName = "Root"};
};

namespace {
using Row = agiru::Projects::Resources::Pricing::ResourceCost_Table;

Row Make(std::string_view code) {
  Row row;
  row.Type = agiru::options::OptionResourceGroupResourceAll::Resource;
  row.Code = code;
  return row;
}

bool Exists(const agiru::Connection &connection, std::string_view code) {
  const std::array<std::optional<std::string>, 1> binds{std::string(code)};
  return connection.Execute(R"(SELECT 1 FROM "Resource Cost" WHERE "Code" = $1)", binds).Rows() ==
         1;
}

class Routine : public agiru::Codeunit<Routine> {
public:
  Row Rec;
  std::function<void()> body;

  void OnRun() const { body(); }
};

class TransactionReport : public agiru::Report<TransactionReport> {
public:
  std::function<void()> walk;
  std::function<void()> post;
  bool posted = false;

  bool AdoptView_([[maybe_unused]] const agiru::TableDef *table,
                  [[maybe_unused]] const void *record) const {
    return false;
  }

  void Walk_() const { walk(); }

  void OnPostReport() {
    posted = true;
    post();
  }
};

class TransactionXml : public agiru::XmlPort<TransactionXml> {
public:
  std::function<void()> process;
  std::function<void()> post;
  bool posted = false;

  bool AdoptView_([[maybe_unused]] const agiru::TableDef *table,
                  [[maybe_unused]] const void *record) const {
    return false;
  }

  void Export_() const { process(); }

  void Import_() const { process(); }

  void OnPostXmlPort() {
    posted = true;
    post();
  }
};

template <typename Body> bool Raises(Body body, std::string_view code = {}) {
  try {
    body();
  } catch (const agiru::Error &error) { return code.empty() || error.Code() == code; }
  return false;
}

void EvaluatedRuns(const std::string &dsn) {
  const agiru::Session session(dsn);
  const agiru::Connection observer(dsn);
  Routine unit;
  CHECK_TEXT("the gate executes the declared codeunit", Routine::Name(), "Transaction contract");
  unit.body = [] { Make("VALUE").Insert(); };
  {
    const agiru::CommitScope restriction(agiru::CommitBehavior::Error);
    CHECK_TRUE("evaluated success bypasses explicit CommitBehavior", unit.Ok_Run());
  }
  CHECK_TRUE("evaluated success is independently durable", Exists(observer, "VALUE"));
  unit.body = [] {
    Make("FAILED").Insert();
    throw agiru::Error("callee failure", "RunFailure");
  };
  CHECK_TRUE("evaluated failure returns false", !unit.Ok_Run());
  CHECK_TRUE("failed evaluated writes never reach the observer", !Exists(observer, "FAILED"));
  CHECK_TRUE("failed evaluated writes are removed from the writer",
             !Exists(session.Database(), "FAILED"));
  CHECK_TEXT("evaluated failure retains its exact diagnostic",
             agiru::GetLastErrorText(),
             "callee failure");
  CHECK_TRUE("a failed evaluated run restores the read phase", !session.Transaction().IsWriting());
  Make("PENDING").Insert();
  bool invoked = false;
  unit.body = [&] { invoked = true; };
  CHECK_TRUE("a pending write refuses before executing evaluated Run",
             Raises([&] { static_cast<void>(unit.Ok_Run()); }, "CodeunitRunTransaction") &&
                 !invoked);
  CHECK_TRUE("refusal does not secretly commit the caller", !Exists(observer, "PENDING"));
  agiru::Commit();
  CHECK_TRUE("explicit commit permits a subsequent evaluated run", unit.Ok_Run());
}

void StatementAndCommit(const std::string &dsn) {
  const agiru::Session session(dsn);
  const agiru::Connection observer(dsn);
  Routine unit;
  unit.body = [] { Make("STATEMENT").Insert(); };
  {
    const agiru::detail::Scope caller;
    unit.Run();
    CHECK_TRUE("statement success does not commit its caller", !Exists(observer, "STATEMENT"));
  }
  CHECK_TRUE("later caller failure discards statement writes", !Exists(observer, "STATEMENT"));
  unit.body = [] {
    Make("PRECOMMIT").Insert();
    agiru::Commit();
    Make("POSTCOMMIT").Insert();
    throw agiru::Error("after durable commit");
  };
  CHECK_TRUE("a statement failure propagates", Raises([&] { unit.Run(); }));
  CHECK_TRUE("callee explicit commit survives its later error", Exists(observer, "PRECOMMIT"));
  CHECK_TRUE("work after the explicit commit rolls back", !Exists(observer, "POSTCOMMIT"));
  CHECK_TRUE("post-Commit failure discards the writer's pending row",
             !Exists(session.Database(), "POSTCOMMIT"));
  unit.body = [] {
    Make("VALUECOMMIT").Insert();
    agiru::Commit();
    Make("VALUELATER").Insert();
    throw agiru::Error("evaluated error after commit");
  };
  CHECK_TRUE("evaluated failure after a Commit still returns false", !unit.Ok_Run());
  CHECK_TRUE("failed evaluated execution preserves its prior durable Commit",
             Exists(observer, "VALUECOMMIT"));
  CHECK_TRUE("failed evaluated execution discards only its later work",
             !Exists(session.Database(), "VALUELATER"));
}

void TryPolicy(const std::string &dsn, bool disabled) {
  const agiru::Session session(dsn, {.disableWriteInsideTryFunctions = disabled});
  const agiru::Connection observer(dsn);
  const std::string code = disabled ? "TRYDENIED" : "TRYALLOWED";
  {
    const agiru::detail::Scope caller;
    CHECK_TRUE("a consumed try catches the failure", !agiru::Tried([&] {
                 Make(code).Insert();
                 throw agiru::Error("after try write");
               }));
    CHECK_TRUE("configured try policy controls actual database writes",
               Exists(session.Database(), code) != disabled);
    CHECK_TRUE("a consumed try does not implicitly commit", !Exists(observer, code));
    agiru::Temporary<Row> temporary;
    temporary.Type = agiru::options::OptionResourceGroupResourceAll::Resource;
    temporary.Code = "TEMPTRY";
    CHECK_TRUE("temporary writes remain allowed in either policy",
               agiru::Tried([&] { temporary.Insert(); }));
    CHECK_TRUE("temporary rows are present", temporary.Count() == 1);
  }
  CHECK_TRUE("the caller boundary discards allowed failed-try writes", !Exists(observer, code));
  CHECK_TRUE("try catch scopes do not leak", !session.Transaction().IsTrying());
  Make(disabled ? "ORDINARYDENIED" : "ORDINARYALLOWED").Insert();
  agiru::Commit();
  CHECK_TRUE("ordinary calls are writable under either policy",
             Exists(observer, disabled ? "ORDINARYDENIED" : "ORDINARYALLOWED"));
  if (!disabled) {
    agiru::detail::Scope caller;
    static_cast<void>(agiru::Tried([] {
      Make("TRYKEPT").Insert();
      throw agiru::Error("caught");
    }));
    caller.Keep();
    agiru::Commit();
    CHECK_TRUE("a caught try error does not roll back writes at successful completion",
               Exists(observer, "TRYKEPT"));
  }
}

void CollectedErrors(const std::string &dsn) {
  const agiru::Session session(dsn);
  const agiru::Connection observer(dsn);
  {
    const agiru::ErrorScope errors(agiru::ErrorBehavior::Collect);
    Routine unit;
    unit.body = [] {
      Make("COLLECTED").Insert();
      agiru::RaiseOrCollect(agiru::ErrorInfo::Create("collectible", true));
    };
    CHECK_TRUE("only collectible errors allow evaluated Run to complete", unit.Ok_Run());
    CHECK_TRUE("collected-error evaluated writes roll back immediately",
               !Exists(session.Database(), "COLLECTED"));
    CHECK_TRUE("rollback preserves the collection for custom UI",
               agiru::ErrorScope::Collected().size() == 1);
    agiru::ErrorScope::Clear();
    CHECK_TRUE("ordinary text errors still stop collection scopes",
               Raises([] { agiru::RaiseOrCollect("plain"); }));
    CHECK_TRUE("noncollectible ErrorInfo still stops", Raises([] {
                 agiru::RaiseOrCollect(agiru::ErrorInfo::Create("noncollectible", false));
               }));
    Make("CLEARED").Insert();
    agiru::RaiseOrCollect(agiru::ErrorInfo::Create("handled", true));
    agiru::ErrorScope::Clear();
  }
  agiru::Commit();
  CHECK_TRUE("clearing collected errors does not discard ordinary writes",
             Exists(observer, "CLEARED"));
  CHECK_TRUE("unhandled collected errors fail their caller", Raises([] {
               const agiru::detail::Scope caller;
               const agiru::ErrorScope errors(agiru::ErrorBehavior::Collect);
               Make("UNHANDLED").Insert();
               agiru::RaiseOrCollect(agiru::ErrorInfo::Create("unhandled", true));
             }));
  CHECK_TRUE("unhandled collection failure rolls back its boundary",
             !Exists(session.Database(), "UNHANDLED"));
}

void MutationPolicy(const std::string &dsn, bool disabled) {
  const agiru::Session session(dsn, {.disableWriteInsideTryFunctions = disabled});
  const std::string key = disabled ? "MUTATIONDENIED" : "MUTATIONALLOWED";
  Make(key).Insert();
  agiru::Commit();
  const std::array<std::function<void(Row &)>, 6> mutations{
      [](Row &row) {
        Make("MUTATIONINSERT").Insert();
        static_cast<void>(row);
      },
      [](Row &row) {
        row.UnitCost = 1;
        row.Modify();
      },
      [](Row &row) { row.Delete(); },
      [](Row &row) { row.Rename(row.Type, "MUTATIONRENAMED", row.WorkTypeCode); },
      [](Row &row) {
        row.SetRange(row.Code, row.Code);
        row.DeleteAll();
      },
      [](Row &row) {
        row.SetRange(row.Code, row.Code);
        row.ModifyAll(row.UnitCost, 1);
      }};
  for (const auto &mutation : mutations) {
    const agiru::detail::Scope caller;
    Row row = Make(key);
    row.Get(row.Type, row.Code, row.WorkTypeCode);
    CHECK_TRUE("every record mutation is caught by the consumed try", !agiru::Tried([&] {
                 mutation(row);
                 throw agiru::Error("after mutation");
               }));
    CHECK_TEXT("all six mutation paths enforce the selected policy",
               agiru::GetLastErrorText(),
               disabled
                   ? "Database writes inside a TryFunction are disabled by runtime configuration"
                   : "after mutation");
  }
  CHECK_TRUE("mutation caller rollbacks preserve the original row",
             Exists(session.Database(), key));
  CHECK_TRUE("mutation caller rollbacks remove inserted rows",
             !Exists(session.Database(), "MUTATIONINSERT"));
  CHECK_TRUE("mutation caller rollbacks remove renamed rows",
             !Exists(session.Database(), "MUTATIONRENAMED"));
  CHECK_TRUE("IsolatedStorage writes obey the same runtime try policy", !agiru::Tried([&] {
               agiru::IsolatedStorage::Set(agiru::Text<0>{key}, "value");
               throw agiru::Error("after isolated storage");
             }));
  CHECK_TEXT("IsolatedStorage cannot bypass the configured try-write refusal",
             agiru::GetLastErrorText(),
             disabled ? "Database writes inside a TryFunction are disabled by runtime configuration"
                      : "after isolated storage");
  if (!disabled) {
    agiru::Text<0> value;
    CHECK_TRUE("allowed IsolatedStorage writes survive the caught error",
               agiru::IsolatedStorage::Get(agiru::Text<0>{key}, value) && value == "value");
  }
}

constexpr std::array<std::string_view, 1> kNames{"Counter"};

void FailingSubscriber([[maybe_unused]] void *instance,
                       const agiru::EventArgs &args,
                       std::span<const std::size_t> bound) {
  ++*static_cast<int *>(args.values[bound[0]]);
  Make("EVENTFAILED").Insert();
  throw agiru::Error("subscriber failure");
}

void SuccessfulSubscriber([[maybe_unused]] void *instance,
                          const agiru::EventArgs &args,
                          std::span<const std::size_t> bound) {
  ++*static_cast<int *>(args.values[bound[0]]);
  Make("EVENTKEPT").Insert();
}

constexpr std::int32_t kEventPublisher = 50303;
constexpr std::array<agiru::Subscription, 2> kSubscriptions{{{.kind = agiru::EventObject::Codeunit,
                                                              .objectId = kEventPublisher,
                                                              .objectName = "Rollback event",
                                                              .event = "Isolated",
                                                              .element = "",
                                                              .parameters = kNames,
                                                              .invoke = FailingSubscriber},
                                                             {.kind = agiru::EventObject::Codeunit,
                                                              .objectId = kEventPublisher,
                                                              .objectName = "Rollback event",
                                                              .event = "Isolated",
                                                              .element = "",
                                                              .parameters = kNames,
                                                              .invoke = SuccessfulSubscriber}}};
const agiru::SubscriptionCatalogue kCatalogue{
    agiru::CodeunitId{50304},
    "Rollback subscribers",
    kSubscriptions,
    false,
    false,
    []() -> void * { return new int{}; },
    [](void *instance) { delete static_cast<int *>(instance); }};

void IsolatedEvents(const std::string &dsn) {
  const agiru::Session session(dsn);
  const agiru::Connection observer(dsn);
  int counter = 0;
  const std::array<void *, 1> values{&counter};
  const auto raise = [&] {
    agiru::detail::RaiseIsolated(agiru::EventObject::Codeunit,
                                 kEventPublisher,
                                 "Rollback event",
                                 "Isolated",
                                 "",
                                 {.names = kNames, .values = values});
  };
  {
    const agiru::detail::Scope caller;
    raise();
    CHECK_TRUE("failed isolated subscribers do not stop later subscribers", counter == 2);
    CHECK_TRUE("failed isolated subscriber writes roll back",
               !Exists(session.Database(), "EVENTFAILED"));
    CHECK_TRUE("successful isolated subscriber writes commit independently",
               Exists(observer, "EVENTKEPT"));
  }
  CHECK_TRUE("successful isolated writes survive later caller rollback",
             Exists(observer, "EVENTKEPT"));
  counter = 0;
  CHECK_TRUE("pending writes make isolated dispatch propagate the subscriber error",
             Raises([&] {
               const agiru::detail::Scope caller;
               Make("EVENTCALLER").Insert();
               raise();
             }) &&
                 counter == 1);
  CHECK_TRUE("nonisolated fallback rolls back the entire caller",
             !Exists(session.Database(), "EVENTCALLER"));
  CHECK_TRUE("nonisolated fallback discards subscriber writes",
             !Exists(session.Database(), "EVENTFAILED"));
}

void Reports(const std::string &dsn) {
  const agiru::Session session(dsn);
  TransactionReport report;
  report.post = [] {};
  report.walk = [&] {
    Make("REPORTQUIT").Insert();
    report.Quit();
  };
  report.Execute();
  CHECK_TRUE("report Quit skips OnPostReport", !report.posted);
  CHECK_TRUE("report Quit rolls back dataset writes", !Exists(session.Database(), "REPORTQUIT"));
  report.walk = [] { Make("REPORTPOST").Insert(); };
  report.post = [&] { report.Quit(); };
  report.Execute();
  CHECK_TRUE("OnPostReport can Quit and discard the report",
             report.posted && !Exists(session.Database(), "REPORTPOST"));
  report.post = [] { throw agiru::Error("report post error"); };
  CHECK_TRUE("OnPostReport errors propagate", Raises([&] { report.Execute(); }));
  CHECK_TRUE("OnPostReport errors discard all report writes",
             !Exists(session.Database(), "REPORTPOST"));
  report.walk = [&] {
    Make("REPORTCOMMIT").Insert();
    agiru::Commit();
    Make("REPORTLATER").Insert();
    report.Quit();
  };
  report.Execute();
  const agiru::Connection observer(dsn);
  CHECK_TRUE("Quit does not undo a report's explicit durable Commit",
             Exists(observer, "REPORTCOMMIT"));
  CHECK_TRUE("Quit discards only post-Commit work", !Exists(session.Database(), "REPORTLATER"));
}

void XmlPorts(const std::string &dsn) {
  const agiru::Session session(dsn);
  TransactionXml port;
  port.post = [] {};
  port.process = [&] {
    Make("XMLQUIT").Insert();
    port.Quit();
  };
  static_cast<void>(port.Export());
  CHECK_TRUE("XMLport Quit skips OnPostXMLport", !port.posted);
  CHECK_TRUE("XMLport Quit discards writes", !Exists(session.Database(), "XMLQUIT"));
  port.process = [] { Make("XMLPOST").Insert(); };
  port.post = [&] { port.Quit(); };
  static_cast<void>(port.Export());
  CHECK_TRUE("OnPostXMLport can Quit and roll back",
             port.posted && !Exists(session.Database(), "XMLPOST"));
  port.post = [] { throw agiru::Error("XMLport post failure"); };
  CHECK_TRUE("XMLport errors propagate", Raises([&] { static_cast<void>(port.Export()); }));
  CHECK_TRUE("XMLport errors discard earlier writes", !Exists(session.Database(), "XMLPOST"));
  port.process = [&] {
    Make("XMLCOMMIT").Insert();
    agiru::Commit();
    Make("XMLLATER").Insert();
    port.Quit();
  };
  static_cast<void>(port.Export());
  const agiru::Connection observer(dsn);
  CHECK_TRUE("XMLport Quit retains an explicit durable Commit", Exists(observer, "XMLCOMMIT"));
  CHECK_TRUE("XMLport Quit discards post-Commit work", !Exists(session.Database(), "XMLLATER"));
}

void NestedSessions(const std::string &dsn) {
  const agiru::Session first(dsn);
  const agiru::CommitScope restricted(agiru::CommitBehavior::Error);
  const agiru::ErrorScope errors(agiru::ErrorBehavior::Collect);
  agiru::RaiseOrCollect(agiru::ErrorInfo::Create("first session", true));
  {
    const agiru::Session second(dsn);
    CHECK_TRUE("another session on the same worker inherits no CommitBehavior",
               !agiru::CommitScope::Standing());
    CHECK_TRUE("another session inherits no collection scope", !agiru::ErrorScope::Collecting());
    CHECK_TRUE("another session inherits no collected errors",
               agiru::ErrorScope::Collected().empty());
  }
  CHECK_TRUE("restoring the parent restores its restriction",
             agiru::CommitScope::Standing() == agiru::CommitBehavior::Error);
  CHECK_TRUE("restoring the parent retains only its own errors",
             agiru::ErrorScope::Collected().size() == 1);
  agiru::ErrorScope::Clear();
}
}

int main() {
  return gate::Run("TransactionContract", [] {
    const gate::OwnedDatabase database("transactions");
    {
      const agiru::Connection connection(database.Dsn());
      agiru::CreateTable(connection, agiru::TableTraits<Row>::kTable);
    }
    EvaluatedRuns(database.Dsn());
    StatementAndCommit(database.Dsn());
    TryPolicy(database.Dsn(), false);
    TryPolicy(database.Dsn(), true);
    MutationPolicy(database.Dsn(), false);
    MutationPolicy(database.Dsn(), true);
    CollectedErrors(database.Dsn());
    IsolatedEvents(database.Dsn());
    Reports(database.Dsn());
    XmlPorts(database.Dsn());
    NestedSessions(database.Dsn());
  });
}
