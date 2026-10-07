#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/Transaction.h"
#include "runtime/UiHost.h"
#include "runtime/test/Handlers.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Dialog.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"
#include "Check.h"
#include "OwnedDatabase.h"

#include <array>
#include <cstddef>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace {

class RecordingHost final : public agiru::UiHost {
public:
  struct Progress {
    std::string text;
    std::vector<agiru::UiValueBinding> bindings;
    std::vector<agiru::Variant> values;
  };

  void QueueMessage(std::string_view text) override { messages.emplace_back(text); }

  agiru::Boolean Confirm(std::string_view text, agiru::Boolean defaultButton) override {
    question = text;
    presentedDefault = defaultButton;
    if (onQuestion) { onQuestion(); }
    if (!confirmation) { throw agiru::Error("No explicit UI answer", "UiMissingAnswer"); }
    const auto answer = *confirmation;
    confirmation.reset();
    return answer;
  }

  agiru::Integer StrMenu(std::string_view options,
                         agiru::Integer defaultChoice,
                         std::string_view instruction) override {
    menu = options;
    menuDefault = defaultChoice;
    menuInstruction = instruction;
    if (!selection) { throw agiru::Error("No explicit UI selection", "UiMissingAnswer"); }
    const auto answer = *selection;
    selection.reset();
    return answer;
  }

  void OpenProgress(const void *owner,
                    std::string_view text,
                    std::span<const agiru::UiValueBinding> values) override {
    Progress progress{
        .text = std::string(text), .bindings = {values.begin(), values.end()}, .values = {}};
    for (const auto &value : values) { progress.values.push_back(value.read(value.value)); }
    if (!windows.emplace(owner, std::move(progress)).second) {
      throw agiru::Error("Progress already open");
    }
  }

  void
  UpdateProgress(const void *owner, agiru::Integer number, const agiru::Variant &value) override {
    auto &progress = windows.at(owner);
    if (number == 0) {
      for (std::size_t index = 0; index < progress.bindings.size(); ++index) {
        const auto &binding = progress.bindings[index];
        progress.values[index] = binding.read(binding.value);
      }
      return;
    }
    const auto index = static_cast<std::size_t>(number - 1);
    const auto &binding = progress.bindings.at(index);
    progress.values.at(index) = value.IsEmpty() ? binding.read(binding.value) : value;
  }

  void CloseProgress(const void *owner) override {
    if (windows.erase(owner) != 1) { throw agiru::Error("Progress not open"); }
  }

  std::vector<std::string> messages;
  std::string question;
  agiru::Boolean presentedDefault = false;
  std::optional<agiru::Boolean> confirmation;
  std::optional<agiru::Integer> selection;
  std::string menu;
  agiru::Integer menuDefault = 0;
  std::string menuInstruction;
  std::map<const void *, Progress> windows;
  std::function<void()> onQuestion;
};

RecordingHost &Install(agiru::Session &session) {
  auto host = std::make_unique<RecordingHost>();
  auto &installed = *host;
  agiru::InstallUiHost(session, std::move(host));
  return installed;
}

template <typename Call> bool Refused(std::string_view code, Call call) {
  try {
    call();
  } catch (const agiru::Error &error) { return error.Code() == code; }
  return false;
}

void BackgroundAndNestedSessions() {
  agiru::Session parent(AGIRU_TEST_DSN);
  CHECK_TRUE("a background session has no UI capability", !agiru::GuiAllowed());
  CHECK_TRUE("a background session has no installed UI endpoint",
             agiru::CurrentUiHost() == nullptr);
  CHECK_TRUE("a default button cannot answer a background confirmation",
             Refused({}, [] { static_cast<void>(agiru::Confirm("Confirm?", true)); }));
  auto &host = Install(parent);
  CHECK_TRUE("a real installed endpoint enables GUI interaction", agiru::GuiAllowed());
  CHECK_TRUE("the active endpoint belongs to this session", agiru::CurrentUiHost() == &host);
  {
    const agiru::Session child(AGIRU_TEST_DSN);
    CHECK_TRUE("a child session never inherits the parent's GUI endpoint", !agiru::GuiAllowed());
  }
  CHECK_TRUE("restoring the parent restores its own endpoint", agiru::CurrentUiHost() == &host);
  agiru::InstallUiHost(parent, nullptr);
  CHECK_TRUE("detaching an endpoint removes GUI capability", !agiru::GuiAllowed());
  agiru::Dialog window;
  CHECK_TRUE("background progress is not silently swallowed",
             Refused("UiHostUnavailable", [&] { window.Open("Working"); }));
}

void MessagesQuestionsAndMenus() {
  agiru::Session session(AGIRU_TEST_DSN);
  auto &host = Install(session);
  agiru::Message("First →");
  agiru::Dialog::Message("Second %1", agiru::Variant{agiru::Integer{2}});
  CHECK_TRUE("messages are queued in AL order", host.messages.size() == 2);
  CHECK_TEXT("Unicode message text is unchanged", host.messages[0], "First →");
  CHECK_TEXT("static Dialog and free Message share substitutions", host.messages[1], "Second 2");
  CHECK_TRUE("a GUI endpoint cannot answer without explicit input", Refused("UiMissingAnswer", [] {
               static_cast<void>(agiru::Confirm("Confirm?", true));
             }));
  host.confirmation = false;
  CHECK_TRUE("an explicit negative answer overrides a positive presentation default",
             !agiru::Dialog::Confirm("Post %1?", true, agiru::Variant{agiru::Integer{1}}));
  CHECK_TEXT("static Dialog and free Confirm share substitutions", host.question, "Post 1?");
  CHECK_TRUE("the positive default reaches presentation without becoming consent",
             host.presentedDefault);
  host.confirmation = true;
  CHECK_TRUE("an explicit positive answer overrides a negative presentation default",
             agiru::Confirm("Confirm?"));
  CHECK_TRUE("omitted confirmation default is No", !host.presentedDefault);
  host.selection = 0;
  CHECK_TRUE("menu cancellation is an explicit zero",
             agiru::StrMenu("Sell,Buy", 2, "Choose →") == 0);
  CHECK_TEXT("menu options remain unchanged", host.menu, "Sell,Buy");
  CHECK_TEXT("menu instruction remains unchanged", host.menuInstruction, "Choose →");
  CHECK_TRUE("the menu default reaches presentation unchanged", host.menuDefault == 2);
  host.selection = 2;
  CHECK_TRUE("static Dialog and free StrMenu share explicit selection",
             agiru::Dialog::StrMenu("Sell,Buy") == 2);
  CHECK_TRUE("omitted static menu default is the first option", host.menuDefault == 1);
}

void LiveProgressBindings() {
  agiru::Session session(AGIRU_TEST_DSN);
  auto &host = Install(session);
  agiru::Dialog window;
  agiru::Integer count = 1;
  agiru::Decimal amount = agiru::Decimal::FromInvariantString("0.1234567890123456789012345678");
  window.Open("Count #1 Amount #2", count, amount);
  CHECK_TEXT("the progress mask reaches the host unchanged",
             host.windows.at(&window).text,
             "Count #1 Amount #2");
  CHECK_TRUE("typed progress bindings retain every variable",
             host.windows.at(&window).values.size() == 2);
  if (host.windows.at(&window).values.size() != 2) { return; }
  CHECK_TEXT("a progress Decimal retains its exact scale and precision",
             host.windows.at(&window).values[1].Get<agiru::Decimal>().ToInvariantString(),
             "0.1234567890123456789012345678");
  count = 2;
  amount = agiru::Decimal{1};
  window.Update();
  CHECK_TRUE("Update without a value rereads the live integer binding",
             host.windows.at(&window).values[0].Get<agiru::Integer>() == 2);
  CHECK_TRUE("Update refreshes every live variable",
             host.windows.at(&window).values[1].Get<agiru::Decimal>() == agiru::Decimal{1});
  window.Update(1, agiru::Variant{agiru::Integer{0}});
  CHECK_TRUE("an explicit zero progress value is not mistaken for omitted input",
             host.windows.at(&window).values[0].Get<agiru::Integer>() == 0);
  CHECK_TRUE("explicit progress updates never mutate the bound AL variable", count == 2);
  window.Update(1);
  CHECK_TRUE("omitted progress replacement rereads its live binding",
             host.windows.at(&window).values[0].Get<agiru::Integer>() == 2);
  window.Close();
  CHECK_TRUE("progress closure reaches the owning endpoint", host.windows.empty());
  CHECK_TRUE("closing an unopened progress dialog raises", Refused({}, [&] { window.Close(); }));
  agiru::Variant dynamic{agiru::Integer{1}};
  window.Open("Dynamic #1", dynamic);
  dynamic = agiru::Integer{2};
  window.Update();
  CHECK_TRUE("the Variant overload retains a live binding",
             host.windows.at(&window).values[0].Get<agiru::Integer>() == 2);
  window.Close();
  window.Open("Working");
  CHECK_TRUE("progress without bindings still reaches the endpoint",
             host.windows.contains(&window));
  window.Close();
}

void TestAdapterDoesNotFallBack() {
  agiru::Session session(AGIRU_TEST_DSN);
  auto &host = Install(session);
  agiru::HandlerTable::Install({}, {});
  CHECK_TRUE("the explicit test adapter retains GUI semantics", agiru::GuiAllowed());
  CHECK_TRUE("an undeclared test handler never falls back to a native endpoint",
             Refused({}, [] { agiru::Message("Undeclared"); }));
  CHECK_TRUE("undeclared test interaction never reaches the native queue", host.messages.empty());
  agiru::Dialog window;
  window.Open("Headless test");
  window.Update();
  window.Close();
  CHECK_TRUE("explicit AL test progress remains headless", host.windows.empty());
  CHECK_TRUE("empty test adapter has no missed declarations",
             agiru::HandlerTable::Uninstall().empty());
  CHECK_TRUE("removing a test adapter restores the real native endpoint", agiru::GuiAllowed());
}

void QuestionsPreserveTransactions() {
  const gate::OwnedDatabase database("ui_transaction");
  agiru::Session session(database.Dsn());
  const auto &writer = session.Database();
  const agiru::Connection observer(database.Dsn());
  writer.Run("CREATE TABLE pending_ui (value integer)");
  auto &host = Install(session);
  host.onQuestion = [&] {
    CHECK_TRUE("a question cannot implicitly commit a pending write",
               observer.Execute("SELECT count(*) FROM pending_ui").Value(0, 0) == "0");
  };
  {
    const agiru::detail::Scope boundary;
    writer.Run("INSERT INTO pending_ui VALUES (1)");
    CHECK_TRUE(
        "missing explicit UI input raises inside the active boundary",
        Refused("UiMissingAnswer", [] { static_cast<void>(agiru::Confirm("Post?", true)); }));
    CHECK_TRUE("host replacement is refused inside an AL boundary",
               Refused({}, [&] { agiru::InstallUiHost(session, nullptr); }));
  }
  CHECK_TRUE("unkept question execution rolls back its pending write",
             observer.Execute("SELECT count(*) FROM pending_ui").Value(0, 0) == "0");
  CHECK_TRUE("refused host replacement retains the existing endpoint",
             agiru::CurrentUiHost() == &host);
  host.onQuestion = [&] {
    CHECK_TRUE("a question preserves prior Commit without publishing later writes",
               observer.Execute("SELECT count(*) FROM pending_ui").Value(0, 0) == "1");
  };
  {
    const agiru::detail::Scope boundary;
    writer.Run("INSERT INTO pending_ui VALUES (1)");
    agiru::Commit();
    writer.Run("INSERT INTO pending_ui VALUES (2)");
    CHECK_TRUE("question cancellation still raises after explicit Commit",
               Refused("UiMissingAnswer", [] { static_cast<void>(agiru::Confirm("Post?")); }));
  }
  CHECK_TRUE("question unwind preserves Commit and rolls back only unfinished work",
             observer.Execute("SELECT array_agg(value) FROM pending_ui").Value(0, 0) == "{1}");
}

template <typename Call> void CheckCallback(bool allowed, std::string_view claim, Call call) {
  bool completed = false;
  const bool blocked = Refused("UiWriteTransaction", [&] {
    call();
    completed = true;
  });
  CHECK_TRUE(claim, allowed ? completed && !blocked : !completed && blocked);
}

void CheckBlockingCallbacks(RecordingHost &host, bool allowed) {
  host.question.clear();
  host.menu.clear();
  host.confirmation = false;
  host.selection = 0;
  CheckCallback(allowed, "confirmation enforces the configured callback policy", [] {
    CHECK_TRUE("permitted confirmation returns an explicit answer, not its default",
               !agiru::Confirm("Post?", true));
  });
  CheckCallback(allowed, "menu enforces the configured callback policy", [] {
    CHECK_TRUE("permitted menu returns explicit cancellation, not its default",
               agiru::StrMenu("Post,Preview", 2, "Choose") == 0);
  });
  CHECK_TRUE("disabled callback policy refuses before presenting a native question",
             host.question.empty() == !allowed && host.menu.empty() == !allowed);
}

void ConfiguredCallbackPolicy(bool allowed) {
  const gate::OwnedDatabase database(allowed ? "ui_callbacks_allowed" : "ui_callbacks_denied");
  agiru::Session session(database.Dsn(),
                         {.allowSessionCallSuspendWhenWriteTransactionStarted = allowed});
  const auto &writer = session.Database();
  const agiru::Connection observer(database.Dsn());
  writer.Run("CREATE TABLE callbacks (value integer)");
  auto &host = Install(session);
  CHECK_TRUE("callback policy does not remove the session's actual GUI endpoint",
             agiru::GuiAllowed());
  CheckBlockingCallbacks(host, true);
  {
    const agiru::detail::Scope boundary;
    static_cast<void>(writer.Execute("SELECT count(*) FROM callbacks"));
    CheckBlockingCallbacks(host, true);
    agiru::detail::RequireWrite();
    writer.Run("INSERT INTO callbacks VALUES (1)");
    const auto depth = session.Transaction().Depth();
    const auto epoch = session.Transaction().CursorEpoch();
    CheckBlockingCallbacks(host, allowed);
    CHECK_TRUE("callback checks preserve the write phase and rollback boundary",
               session.Transaction().IsWriting() && session.Transaction().Depth() == depth &&
                   session.Transaction().CursorEpoch() == epoch);
    CHECK_TRUE("callback refusal neither commits nor rolls back caller writes",
               writer.Execute("SELECT count(*) FROM callbacks").Value(0, 0) == "1" &&
                   observer.Execute("SELECT count(*) FROM callbacks").Value(0, 0) == "0");
    agiru::Message("Queued while writing");
    agiru::Dialog progress;
    progress.Open("Writing");
    progress.Update();
    progress.Close();
    CHECK_TRUE("nonblocking messages and progress remain allowed during writes",
               host.messages.size() == 1 && host.windows.empty());
    {
      const agiru::detail::Scope nested;
      CheckBlockingCallbacks(host, allowed);
    }
    CheckBlockingCallbacks(host, allowed);
    agiru::Commit();
    CHECK_TRUE("explicit Commit ends the write phase without changing immutable callback policy",
               !session.Transaction().IsWriting() &&
                   session.Options().allowSessionCallSuspendWhenWriteTransactionStarted == allowed);
    CheckBlockingCallbacks(host, true);
    agiru::detail::RequireWrite();
    writer.Run("INSERT INTO callbacks VALUES (2)");
    CheckBlockingCallbacks(host, allowed);
  }
  CHECK_TRUE("callback boundary unwind retains prior Commit and rolls back only later work",
             observer.Execute("SELECT array_agg(value) FROM callbacks").Value(0, 0) == "{1}");
  CheckBlockingCallbacks(host, true);
}

void TestHandlersObeyCallbackPolicy(bool allowed) {
  const gate::OwnedDatabase database(allowed ? "ui_test_allowed" : "ui_test_denied");
  const agiru::Session session(database.Dsn(),
                               {.allowSessionCallSuspendWhenWriteTransactionStarted = allowed});
  constexpr std::array handlers{
      agiru::TestHandler{
          .name = "Question",
          .kind = agiru::HandlerKind::Confirm,
          .object = 0,
          .invoke =
              +[](std::string_view, void *reply) { *static_cast<agiru::Boolean *>(reply) = false; },
          .optional = false},
      agiru::TestHandler{.name = "Menu",
                         .kind = agiru::HandlerKind::StrMenu,
                         .object = 0,
                         .invoke =
                             +[](std::string_view, void *reply) {
                               static_cast<agiru::StrMenuAnswer *>(reply)->choice = 0;
                             },
                         .optional = false}};
  constexpr std::array<std::string_view, 2> declared{"Question", "Menu"};
  agiru::HandlerTable::Install(handlers, declared);
  try {
    const agiru::detail::Scope boundary;
    agiru::detail::RequireWrite();
    CheckCallback(allowed, "AL test confirmation obeys the configured callback policy", [] {
      CHECK_TRUE("permitted AL confirmation handler retains its explicit answer",
                 !agiru::Confirm("Test?", true));
    });
    CheckCallback(allowed, "AL test menu obeys the configured callback policy", [] {
      CHECK_TRUE("permitted AL menu handler retains its explicit answer",
                 agiru::StrMenu("Post,Preview", 2) == 0);
    });
  } catch (...) {
    agiru::HandlerTable::Reset();
    throw;
  }
  const auto missed = agiru::HandlerTable::Uninstall();
  CHECK_TRUE(
      "disabled policy refuses before invoking or marking AL test callbacks",
      (allowed ? missed.empty() : missed == std::vector<std::string_view>{"Question", "Menu"}));
}

void DetachedSessionsKeepTheirEndpoint() {
  const gate::OwnedDatabase database("ui_migration");
  const agiru::Guid identity = agiru::Guid::Create();
  {
    const agiru::Session seed(database.Dsn());
    agiru::CreateTable(seed.Database(), agiru::platform::kUserTable);
    agiru::platform::User user;
    user.UserSecurityID = identity;
    user.UserName = "UI gate user";
    user.Insert();
    agiru::Commit();
  }
  agiru::Session session(identity, {.allowSessionCallSuspendWhenWriteTransactionStarted = false});
  auto &host = Install(session);
  CHECK_TRUE("an idle authenticated context cannot become the ambient UI host",
             agiru::CurrentUiHost() == nullptr);
  for (agiru::Integer command = 0; command < 2; ++command) {
    std::exception_ptr failure;
    std::thread worker([&] {
      try {
        agiru::Connection connection(database.Dsn());
        agiru::SessionCommand execution(session, connection);
        CHECK_TRUE("worker activation finds this session's retained UI endpoint",
                   agiru::CurrentUiHost() == &host && agiru::GuiAllowed());
        agiru::detail::RequireWrite();
        CHECK_TRUE("callback policy belongs to the session across worker migration",
                   Refused("UiWriteTransaction",
                           [] { static_cast<void>(agiru::Confirm("Worker question?", true)); }));
        agiru::Message("Command %1", command);
        CHECK_TRUE("an active command refuses UI host replacement",
                   Refused({}, [&] { agiru::InstallUiHost(session, nullptr); }));
        execution.Keep();
        CHECK_TRUE("a reused worker retains no ambient UI endpoint",
                   agiru::CurrentUiHost() == nullptr);
      } catch (...) { failure = std::current_exception(); }
    });
    worker.join();
    if (failure) { std::rethrow_exception(failure); }
  }
  CHECK_TRUE("the retained endpoint survives commands on different workers",
             host.messages.size() == 2);
  CHECK_TEXT("first command queues to its owning endpoint", host.messages[0], "Command 0");
  CHECK_TEXT("second command queues to the same session endpoint", host.messages[1], "Command 1");
}

}

int main() {
  return gate::Run("UiHost", [] {
    BackgroundAndNestedSessions();
    MessagesQuestionsAndMenus();
    LiveProgressBindings();
    TestAdapterDoesNotFallBack();
    QuestionsPreserveTransactions();
    ConfiguredCallbackPolicy(true);
    ConfiguredCallbackPolicy(false);
    TestHandlersObeyCallbackPolicy(true);
    TestHandlersObeyCallbackPolicy(false);
    DetachedSessionsKeepTheirEndpoint();
  });
}
