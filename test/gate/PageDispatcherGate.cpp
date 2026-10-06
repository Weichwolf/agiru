#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Page.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageSession.h"
#include "runtime/test/TestPage.h"
#include "type/Boolean.h"
#include "type/StringValue.h"

#include "Check.h"

#include <array>
#include <string>
#include <string_view>
#include <type_traits>

namespace {

class CommandPage : public agiru::Page<CommandPage> {
public:
  void Set(std::string_view text) { text_ = text; }

  [[nodiscard]] std::string Read() const { return text_; }

  void Hit() { ++hits_; }

  [[nodiscard]] std::string Hits() const { return std::to_string(hits_); }

  agiru::Boolean Allowed() { return text_ == "enabled"; }

  agiru::Text<0> Fail() { throw agiru::Error("original AL failure", "OriginalCode"); }

private:
  std::string text_ = "before";
  int hits_ = 0;
};

struct Controls {};

constexpr std::array kDisabledFields{agiru::ControlDef{
    .kind = agiru::ControlKind::Field, .name = "DisabledField", .enabled = "true"}};
constexpr std::array kReadOnlyFields{agiru::ControlDef{
    .kind = agiru::ControlKind::Field, .name = "ReadOnlyField", .editable = "true"}};
constexpr std::array kHiddenFields{
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "HiddenField", .visible = "true"}};
constexpr std::array kLayout{
    agiru::ControlDef{
        .kind = agiru::ControlKind::Field, .name = "Value", .caption = "Display value"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Hits", .editable = "false"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Fail"},
    agiru::ControlDef{.kind = agiru::ControlKind::Group,
                      .name = "DisabledGroup",
                      .enabled = "false",
                      .children = kDisabledFields},
    agiru::ControlDef{.kind = agiru::ControlKind::Group,
                      .name = "ReadOnlyGroup",
                      .editable = "false",
                      .children = kReadOnlyFields},
    agiru::ControlDef{.kind = agiru::ControlKind::Group,
                      .name = "HiddenGroup",
                      .visible = "false",
                      .children = kHiddenFields}};
constexpr std::array kDisabledActions{agiru::ControlDef{
    .kind = agiru::ControlKind::Action, .name = "DisabledAction", .enabled = "true"}};
constexpr std::array kActions{
    agiru::ControlDef{.kind = agiru::ControlKind::Action, .name = "Run"},
    agiru::ControlDef{.kind = agiru::ControlKind::Action, .name = "Conditional"},
    agiru::ControlDef{.kind = agiru::ControlKind::ActionRef, .name = "Promoted"},
    agiru::ControlDef{.kind = agiru::ControlKind::Group,
                      .name = "DisabledActions",
                      .enabled = "false",
                      .children = kDisabledActions}};
constexpr agiru::PageDef kPage{
    .id = agiru::PageId{50131}, .name = "Command Fixture", .layout = kLayout, .actions = kActions};
constexpr auto kSet = +[](CommandPage &page, std::string_view text) { page.Set(text); };
constexpr auto kRead = +[](const CommandPage &page) { return page.Read(); };
constexpr std::array kBindings{
    agiru::ControlTrigger<CommandPage>{.control = "Value", .set = kSet, .text = kRead},
    agiru::ControlTrigger<CommandPage>{.control = "DisabledField", .set = kSet, .text = kRead},
    agiru::ControlTrigger<CommandPage>{.control = "ReadOnlyField", .set = kSet, .text = kRead},
    agiru::ControlTrigger<CommandPage>{.control = "HiddenField", .set = kSet, .text = kRead},
    agiru::ControlTrigger<CommandPage>{
        .control = "Hits", .text = +[](const CommandPage &page) { return page.Hits(); }},
    agiru::ControlTrigger<CommandPage>{.control = "Fail", .sourceText = &CommandPage::Fail},
    agiru::ControlTrigger<CommandPage>{.control = "Run", .action = &CommandPage::Hit},
    agiru::ControlTrigger<CommandPage>{
        .control = "Conditional", .action = &CommandPage::Hit, .enabled = &CommandPage::Allowed},
    agiru::ControlTrigger<CommandPage>{.control = "DisabledAction", .action = &CommandPage::Hit}};

class Authorization final : public agiru::PageAuthorization {
public:
  void Require(agiru::PageId page, const agiru::PageControlCommand &command) override {
    static_cast<void>(command);
    CHECK_TRUE("authorization receives owning page identity", page == kPage.id);
    ++calls_;
    if (!allowed_) { throw agiru::Error("permission revoked", "PermissionDenied"); }
  }

  void Revoke() { allowed_ = false; }

  [[nodiscard]] int Calls() const { return calls_; }

private:
  bool allowed_ = true;
  int calls_ = 0;
};

}

template <> struct agiru::PageTraits<CommandPage> {
  static constexpr agiru::PageId kId = ::kPage.id;
  static constexpr std::string_view kName = ::kPage.name;
  static constexpr const agiru::PageDef &kPage = ::kPage;
  static constexpr auto kControlTriggers = kBindings;
  template <typename, typename, template <typename> class> using Controls = ::Controls;
};

static_assert(agiru::PageTraits<CommandPage>::kId == kPage.id);
static_assert(agiru::PageTraits<CommandPage>::kName == kPage.name);

namespace {

using Operation = agiru::PageControlOperation;

template <typename T>
concept HasTestTrap = requires(T &page) { page.Trap(); };

static_assert(!std::is_copy_constructible_v<agiru::PageSession<CommandPage>>);
static_assert(!std::is_move_constructible_v<agiru::PageSession<CommandPage>>);
static_assert(!HasTestTrap<agiru::PageSession<CommandPage>>);
static_assert(HasTestTrap<agiru::TestPage<CommandPage>>);

void Refuses(agiru::PageDispatcher &dispatcher,
             Operation operation,
             std::string_view control,
             std::string_view code = "PageCommand",
             std::string_view message = {}) {
  bool refused = false;
  try {
    static_cast<void>(dispatcher.Execute(
        {.operation = operation, .control = control, .text = "unexpected write"}));
  } catch (const agiru::Error &error) {
    refused = error.Code() == code && (message.empty() || error.what() == message);
  }
  CHECK_TRUE("invalid commands refuse with the expected diagnostic", refused);
}

void DispatchUsesExistingBindings() {
  agiru::TestPage<CommandPage> page;
  page.OpenEdit();
  Authorization authority;
  agiru::PageDispatcher dispatcher(kPage, page, authority);
  const std::string exact = "Grüezi <script>\n79228162514264337593543950335";
  static_cast<void>(
      dispatcher.Execute({.operation = Operation::Set, .control = "Value", .text = exact}));
  CHECK_TEXT("dispatcher delegates the unchanged input to the existing binding",
             page.ControlText("Value"),
             exact);
  const auto result = dispatcher.Execute({.operation = Operation::Read, .control = "Value"});
  CHECK_TEXT("read text preserves all data without HTML or numeric conversion", result.text, exact);
  CHECK_TRUE("a plain text source does not fabricate an option ordinal", result.ordinal.empty());
  static_cast<void>(dispatcher.Execute({.operation = Operation::Action, .control = "Run"}));
  CHECK_TEXT("action executes once on the same page instance", page.ControlText("Hits"), "1");
  CHECK_TRUE("every successful command reauthorizes", authority.Calls() == 3);
  Refuses(dispatcher, Operation::Read, "Fail", "OriginalCode", "original AL failure");
  Refuses(dispatcher, Operation::Filter, "Value", "", "The TestPage is not open.");
  page.Close();
}

void CurrentStateAndIdentityAreAuthoritative() {
  agiru::TestPage<CommandPage> page;
  page.OpenEdit();
  Authorization authority;
  agiru::PageDispatcher dispatcher(kPage, page, authority);
  Refuses(dispatcher, Operation::Action, "Conditional");
  Refuses(dispatcher, Operation::Action, "DisabledAction");
  Refuses(dispatcher, Operation::Set, "DisabledField");
  Refuses(dispatcher, Operation::Set, "ReadOnlyField");
  Refuses(dispatcher, Operation::Read, "HiddenField");
  Refuses(dispatcher, Operation::Action, "Missing");
  Refuses(dispatcher, Operation::Set, "value");
  Refuses(dispatcher, Operation::Set, "Display value");
  Refuses(dispatcher, Operation::Action, "Value");
  Refuses(dispatcher, Operation::Set, "Run");
  Refuses(dispatcher, Operation::Action, "DisabledActions");
  Refuses(dispatcher, Operation::Action, "Promoted");
  Refuses(dispatcher, Operation::Unknown, "Value");
  CHECK_TEXT("refused writes never reach setters", page.ControlText("Value"), "before");
  CHECK_TEXT("refused actions never reach triggers", page.ControlText("Hits"), "0");
  CHECK_TEXT("disabled display fields can still be read",
             dispatcher.Execute({.operation = Operation::Read, .control = "DisabledField"}).text,
             "before");
  static_cast<void>(
      dispatcher.Execute({.operation = Operation::Set, .control = "Value", .text = "enabled"}));
  static_cast<void>(dispatcher.Execute({.operation = Operation::Action, .control = "Conditional"}));
  CHECK_TEXT("computed state is reevaluated after input", page.ControlText("Hits"), "1");
  authority.Revoke();
  for (const Operation operation : {Operation::Read, Operation::Set, Operation::Filter}) {
    Refuses(dispatcher, operation, "Value", "PermissionDenied");
  }
  Refuses(dispatcher, Operation::Action, "Run", "PermissionDenied");
  CHECK_TEXT("revoked permissions prevent another action", page.ControlText("Hits"), "1");
  CHECK_TEXT("revoked permissions prevent another write", page.ControlText("Value"), "enabled");
  page.Close();
  page.OpenView();
  Refuses(dispatcher, Operation::Set, "Value", "PermissionDenied");
  Authorization viewingAuthority;
  agiru::PageDispatcher viewing(kPage, page, viewingAuthority);
  Refuses(viewing, Operation::Set, "Value");
  Refuses(viewing, Operation::Lookup, "Value");
  Refuses(viewing, Operation::AssistEdit, "Value");
  CHECK_TEXT(
      "view-mode refusal does not edit the reopened page", page.ControlText("Value"), "before");
  page.Close();
}

void ProductionSessionsUseTheSharedKernel() {
  agiru::PageSession<CommandPage> first;
  agiru::PageSession<CommandPage> second;
  CHECK_TRUE("production session starts closed", !first.IsOpen());
  bool refused = false;
  try {
    first.SetControlText("Value", "before open");
  } catch (const agiru::Error &error) { refused = error.Code() == "PageNotOpen"; }
  CHECK_TRUE("production lifecycle has its own not-open diagnostic", refused);
  first.OpenEdit();
  second.OpenEdit();
  Authorization firstAuthority;
  Authorization secondAuthority;
  agiru::PageDispatcher dispatcher(kPage, first, firstAuthority);
  agiru::PageDispatcher other(kPage, second, secondAuthority);
  static_cast<void>(
      dispatcher.Execute({.operation = Operation::Set, .control = "Value", .text = "enabled"}));
  static_cast<void>(dispatcher.Execute({.operation = Operation::Action, .control = "Conditional"}));
  CHECK_TEXT("production dispatcher reaches the same typed trigger path",
             dispatcher.Execute({.operation = Operation::Read, .control = "Hits"}).text,
             "1");
  Refuses(other, Operation::Action, "Conditional");
  CHECK_TEXT("production page instances do not share mutable state",
             other.Execute({.operation = Operation::Read, .control = "Value"}).text,
             "before");
  Refuses(dispatcher, Operation::Read, "Fail", "OriginalCode", "original AL failure");
  refused = false;
  try {
    first.OpenNew();
  } catch (const agiru::Error &error) { refused = error.Code() == "PageAlreadyOpen"; }
  CHECK_TRUE("production double-open is an explicit refusal", refused);
  first.Close();
  CHECK_TRUE("production close releases the owned page", !first.IsOpen());
  first.OpenView();
  Refuses(dispatcher, Operation::Set, "Value");
  CHECK_TEXT("production reopen resets page-local state", first.ControlText("Value"), "before");
  first.Close();
  second.Close();
}

}

int main() {
  return gate::Run("PageDispatcher", [] {
    DispatchUsesExistingBindings();
    CurrentStateAndIdentityAreAuthoritative();
    ProductionSessionsUseTheSharedKernel();
  });
}
