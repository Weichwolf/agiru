#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Page.h"
#include "runtime/PageCore.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageHtml.h"
#include "runtime/PageInstance.h"
#include "runtime/PageSession.h"
#include "runtime/PageValue.h"
#include "runtime/PageWindow.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TablePermissions.h"
#include "runtime/Transaction.h"
#include "runtime/UiHost.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/TestPage.h"
#include "runtime/test/TestRequestPage.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Variant.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "fixture/codeunit/NavigationPropertyConsumer.h"
#include "fixture/page/NavigationBlockedList.h"
#include "fixture/page/NavigationCard.h"
#include "fixture/page/NavigationCreated.h"
#include "fixture/page/NavigationCustom.h"
#include "fixture/page/NavigationDelayed.h"
#include "fixture/page/NavigationFindOnly.h"
#include "fixture/page/NavigationList.h"
#include "fixture/page/NavigationModal.h"
#include "fixture/page/NavigationOverride.h"
#include "fixture/page/NavigationProviderWindow.h"
#include "fixture/page/NavigationWindow.h"
#include "fixture/page/NavigationWindowError.h"
#include "fixture/report/NavigationReport.h"
#include "fixture/table/NavigationRow.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using Row = agiru::Fixture::NavigationRow_Table;
using Card = agiru::Fixture::NavigationCard_Page;
using List = agiru::Fixture::NavigationList_Page;
using Modal = agiru::Fixture::NavigationModal_Page;
constexpr agiru::Integer kFirstValue = 11;
constexpr agiru::Integer kSecondValue = 22;
constexpr agiru::Integer kOverrideIncrement = 100;
constexpr agiru::Integer kCopiedValue = 55;
constexpr agiru::Integer kRequestLimit = 7;
constexpr std::size_t kListRows = 40;
constexpr agiru::Integer kCallerModalMarker = 41;

class HtmlAuthorization final : public agiru::PageAuthorization {
public:
  void Require(agiru::PageId page, const agiru::PageControlCommand &command) override {
    if (page != agiru::PageTraits<Card>::kId ||
        (command.operation != agiru::PageControlOperation::ReadValue &&
         command.operation != agiru::PageControlOperation::Inspect)) {
      throw agiru::Error("unexpected HTML command", "FixturePermission");
    }
    ++calls;
  }

  int calls = 0;
};

class CommandAuthorization final : public agiru::PageAuthorization {
public:
  void Require(agiru::PageId page, const agiru::PageControlCommand &command) override {
    if (agiru::Session::Current().UserSecurityId().ToStorageText() !=
            "00000000-0000-0000-0000-000000000001" ||
        page != agiru::PageTraits<Card>::kId ||
        (command.operation != agiru::PageControlOperation::ReadValue &&
         command.operation != agiru::PageControlOperation::Inspect &&
         command.operation != agiru::PageControlOperation::Set)) {
      throw agiru::Error("unexpected authenticated fixture command", "FixturePermission");
    }
  }
};

class NavigationTableAuthority final : public agiru::TablePermissionAuthority {
public:
  bool Allows(const agiru::TableDef &table,
              [[maybe_unused]] agiru::TableOperation operation) const override {
    return table.id == agiru::TableTraits<Row>::kTable.id;
  }
};

void Prepare() {
  agiru::DropTable(agiru::Session::Current().Database(), agiru::TableTraits<Row>::kTable);
  agiru::CreateTable(agiru::Session::Current().Database(), agiru::TableTraits<Row>::kTable);
  Row row;
  row.DeleteAll();
  row.ID = 1;
  row.Value = kFirstValue;
  row.Insert();
  row.ID = 2;
  row.Value = kSecondValue;
  row.Insert();
}

class ModalHost final : public agiru::UiHost {
public:
  void QueueMessage([[maybe_unused]] std::string_view text) override {
    throw agiru::Error("Unexpected message");
  }

  agiru::Boolean Confirm([[maybe_unused]] std::string_view text,
                         [[maybe_unused]] agiru::Boolean defaultButton) override {
    throw agiru::Error("Unexpected confirmation");
  }

  agiru::Integer StrMenu([[maybe_unused]] std::string_view options,
                         [[maybe_unused]] agiru::Integer defaultChoice,
                         [[maybe_unused]] std::string_view instruction) override {
    throw agiru::Error("Unexpected menu");
  }

  agiru::Action RunModal(agiru::PageInstance &page) override {
    ++calls;
    return run ? run(page) : UiHost::RunModal(page);
  }

  void OpenProgress([[maybe_unused]] const void *owner,
                    [[maybe_unused]] std::string_view text,
                    [[maybe_unused]] std::span<const agiru::UiValueBinding> values) override {
    throw agiru::Error("Unexpected progress");
  }

  void UpdateProgress([[maybe_unused]] const void *owner,
                      [[maybe_unused]] agiru::Integer number,
                      [[maybe_unused]] const agiru::Variant &value) override {
    throw agiru::Error("Unexpected progress");
  }

  void CloseProgress([[maybe_unused]] const void *owner) override {
    throw agiru::Error("Unexpected progress");
  }

  std::function<agiru::Action(agiru::PageInstance &)> run;
  int calls = 0;
};

template <typename Call> bool ModalRefused(std::string_view code, Call call) {
  try {
    call();
  } catch (const agiru::Error &error) { return error.Code() == code; }
  return false;
}

void BorrowedModalCloseRetries(ModalHost &host) {
  Modal page;
  page.SetMarker(kCallerModalMarker);
  page.RequireCloseRetries();
  page.LookupMode(true);
  Row source;
  source.SetRange(source.ID, 2);
  page.SetTableView(source);
  host.run = [](agiru::PageInstance &adapter) {
    CHECK_TRUE("the modal host receives a closed prepared adapter", !adapter.IsOpen());
    adapter.Open(agiru::PageOpenMode::Edit);
    CHECK_TEXT("modal opening preserves caller variables",
               adapter.Controls().ControlText("OwnerMarker"),
               "42");
    const auto marker = adapter.Controls().Control_Value("OwnerMarker");
    CHECK_TRUE("original page variables have declared exact scalar bindings",
               marker.type == "Integer" && marker.value == "42");
    CHECK_TEXT("modal opening preserves caller filters", adapter.Controls().ControlText("ID"), "2");
    CHECK_TRUE(
        "false query-close leaves the modal open",
        ModalRefused({}, [&] { static_cast<void>(adapter.CloseModal(agiru::Action::OK)); }) &&
            adapter.IsOpen());
    CHECK_TEXT("the first explicit close attempt ran AL",
               adapter.Controls().ControlText("CloseAttempts"),
               "1");
    CHECK_TRUE(
        "an error on a later query-close leaves the modal open",
        ModalRefused({}, [&] { static_cast<void>(adapter.CloseModal(agiru::Action::OK)); }) &&
            adapter.IsOpen());
    CHECK_TEXT("query-close runs again after a veto",
               adapter.Controls().ControlText("CloseAttempts"),
               "2");
    CHECK_TEXT("close triggers have not run after refusals",
               adapter.Controls().ControlText("ClosedCount"),
               "0");
    CHECK_TRUE("invalid modal actions cannot implicitly close",
               ModalRefused("UiModalAction",
                            [&] { static_cast<void>(adapter.CloseModal(agiru::Action::None)); }) &&
                   adapter.IsOpen());
    const auto answer = adapter.CloseModal(agiru::Action::OK);
    CHECK_TRUE("explicit LookupMode normalizes the actual choice",
               answer == agiru::Action::LookupOK);
    CHECK_TRUE("the successful third attempt releases only the adapter", !adapter.IsOpen());
    return answer;
  };
  CHECK_TRUE("AL receives the original modal's explicit result",
             page.RunModal() == agiru::Action::LookupOK);
  Row selected;
  page.GetRecord(selected);
  CHECK_TRUE("GetRecord remains usable on the original AL variable", selected.ID == 2);
  CHECK_TRUE("opening did not lose the caller's filtered view",
             page.Rec.GetFilter(page.Rec.ID).Value() == "2");
  auto inspect = agiru::MakeInstalledPage(agiru::PageTraits<Modal>::kId);
  inspect->PrepareBorrowed(&page, agiru::PageTraits<Modal>::kId);
  inspect->Open(agiru::PageOpenMode::View);
  CHECK_TEXT("reopening retains the original object's variable",
             inspect->Controls().ControlText("OwnerMarker"),
             "43");
  CHECK_TEXT("both refusals were retried on the original object",
             inspect->Controls().ControlText("CloseAttempts"),
             "3");
  CHECK_TEXT("OnClosePage ran exactly once", inspect->Controls().ControlText("ClosedCount"), "1");
  static_cast<void>(inspect->CloseModal(agiru::Action::Cancel));
  CHECK_TRUE("each reopening runs its own successful close exactly once",
             page.GetCloseCount() == 2);
}

void BorrowedModalChoices(ModalHost &host) {
  host.run = [](agiru::PageInstance &adapter) {
    adapter.Open(agiru::PageOpenMode::View);
    CHECK_TRUE("explicit modal selection moves through the SQL cursor",
               adapter.Move(agiru::PagePosition::Last));
    return adapter.CloseModal(agiru::Action::Cancel);
  };
  Modal page;
  CHECK_TRUE("a card cancellation is not consent", page.RunModal() == agiru::Action::Cancel);
  page.LookupMode(true);
  CHECK_TRUE("lookup cancellation is not lookup acceptance",
             page.RunModal() == agiru::Action::LookupCancel);
  Row source;
  source.Get(1);
  CHECK_TRUE("numbered modal cancellation uses the existing AL action normalization",
             agiru::Page<>::RunModal(agiru::PageTraits<Modal>::kId.Value(), source) ==
                 agiru::Action::LookupCancel);
  CHECK_TRUE("numbered modal writes its selected record back to the AL argument", source.ID == 2);
  auto adapter = agiru::MakeInstalledPage(agiru::PageTraits<Modal>::kId);
  CHECK_TRUE("borrowed factory checks declaration identity before casting",
             ModalRefused("UiModalIdentity",
                          [&] { adapter->PrepareBorrowed(&page, agiru::PageTraits<Card>::kId); }));
  CHECK_TRUE("borrowed factory refuses a null original object",
             ModalRefused("UiModalIdentity", [&] {
               adapter->PrepareBorrowed(nullptr, agiru::PageTraits<Modal>::kId);
             }));
  adapter->PrepareBorrowed(&page, agiru::PageTraits<Modal>::kId);
  CHECK_TRUE("a prepared adapter cannot silently replace its AL object",
             ModalRefused("PageAlreadyOpen",
                          [&] { adapter->PrepareBorrowed(&page, agiru::PageTraits<Modal>::kId); }));
  host.run = {};
  CHECK_TRUE("an installed host without modal transport refuses instead of selecting",
             ModalRefused("UiModalUnsupported", [&] { static_cast<void>(page.RunModal()); }));
  const int before = host.calls;
  agiru::HandlerTable::Install({}, {});
  try {
    CHECK_TRUE("undeclared AL modal handlers never fall back to a native host",
               ModalRefused({}, [&] { static_cast<void>(page.RunModal()); }) &&
                   host.calls == before);
  } catch (...) {
    agiru::HandlerTable::Reset();
    throw;
  }
  CHECK_TRUE("undeclared modal handlers do not invent used declarations",
             agiru::HandlerTable::Uninstall().empty());
}

void ModalTestHandlerPrecedence(ModalHost &host);

void BorrowedModalInputRefusals(ModalHost &host) {
  Modal page;
  page.SetMarker(kCallerModalMarker);
  host.run = [](agiru::PageInstance &adapter) {
    adapter.Open(agiru::PageOpenMode::Edit);
    auto &controls = adapter.Controls();
    using Input = std::pair<std::string_view, std::string_view>;
    constexpr std::array inputs{Input{"OwnerMarker", "invalid-integer"},
                                Input{"OwnerMarker", "2147483648"},
                                Input{"ExactAmount", "invalid-decimal"},
                                Input{"ExactInteger", "9223372036854775808"},
                                Input{"ExactInteger", "-9223372036854775809"},
                                Input{"ArrayValue", "1.5"},
                                Input{"Choice", "absent-member"},
                                Input{"Choice", "2147483648"},
                                Input{"Choice", "1-2"}};
    for (const auto input : inputs) {
      const std::string before = controls.ControlText(input.first);
      CHECK_TRUE("invalid modal variables refuse before their AL validation trigger",
                 ModalRefused("TestValidation", [&controls, input] {
                   controls.SetControlText(input.first, input.second);
                 }));
      CHECK_TEXT("failed modal conversion preserves the original AL value",
                 controls.ControlText(input.first),
                 before);
      CHECK_TEXT("failed conversion never runs the AL validation trigger",
                 controls.ControlText("ValidationCount"),
                 "0");
    }
    controls.SetControlText("OwnerMarker", "123");
    CHECK_TEXT("explicit valid correction updates the original variable",
               controls.ControlText("OwnerMarker"),
               "123");
    CHECK_TEXT("valid correction runs its AL validation exactly once",
               controls.ControlText("ValidationCount"),
               "1");
    return adapter.CloseModal(agiru::Action::Cancel);
  };
  CHECK_TRUE("invalid inputs do not close the original modal",
             page.RunModal() == agiru::Action::Cancel);
  CHECK_TRUE("the caller retains the corrected AL variable", page.GetMarker() == 123);
}

void BorrowedModalLifecycle(const std::string &dsn) {
  agiru::Session session(dsn);
  auto host = std::make_unique<ModalHost>();
  auto &installed = *host;
  agiru::InstallUiHost(session, std::move(host));
  const agiru::detail::Scope isolation;
  Prepare();
  BorrowedModalCloseRetries(installed);
  BorrowedModalChoices(installed);
  BorrowedModalInputRefusals(installed);
  ModalTestHandlerPrecedence(installed);
}

void ModalTestHandlerPrecedence(ModalHost &host) {
  constexpr std::array handlers{agiru::TestHandler{.name = "Modal",
                                                   .kind = agiru::HandlerKind::ModalPage,
                                                   .object = agiru::PageTraits<Modal>::kId.Value(),
                                                   .invoke =
                                                       +[](std::string_view, void *object) {
                                                         static_cast<agiru::Page<Modal> &>(
                                                             *static_cast<Modal *>(object))
                                                             .CloseWith(agiru::Action::Cancel);
                                                       },
                                                   .optional = false}};
  constexpr std::array<std::string_view, 1> declared{"Modal"};
  const int before = host.calls;
  Modal page;
  agiru::HandlerTable::Install(handlers, declared);
  try {
    CHECK_TRUE("explicit AL modal handlers retain precedence over the native host",
               page.RunModal() == agiru::Action::Cancel && host.calls == before);
  } catch (...) {
    agiru::HandlerTable::Reset();
    throw;
  }
  CHECK_TRUE("the explicit modal handler is counted as used",
             agiru::HandlerTable::Uninstall().empty());
}

void ModalCallbacksRefuseBeforeOpening(const std::string &dsn) {
  agiru::Session session(dsn, {.allowSessionCallSuspendWhenWriteTransactionStarted = false});
  auto host = std::make_unique<ModalHost>();
  const auto &installed = *host;
  agiru::InstallUiHost(session, std::move(host));
  const agiru::detail::Scope boundary;
  agiru::detail::RequireWrite();
  const auto depth = session.Transaction().Depth();
  const auto epoch = session.Transaction().CursorEpoch();
  Modal page;
  page.SetMarker(kCallerModalMarker);
  CHECK_TRUE("disabled modal callbacks refuse before native opening",
             ModalRefused("UiWriteTransaction", [&] { static_cast<void>(page.RunModal()); }) &&
                 installed.calls == 0 && page.GetMarker() == kCallerModalMarker);
  CHECK_TRUE("disabled numbered callbacks refuse before constructing a native modal",
             ModalRefused("UiWriteTransaction",
                          [&] {
                            static_cast<void>(
                                agiru::Page<>::RunModal(agiru::PageTraits<Modal>::kId.Value()));
                          }) &&
                 installed.calls == 0);
  CHECK_TRUE("modal refusal preserves the caller's write phase and boundary",
             session.Transaction().IsWriting() && session.Transaction().Depth() == depth &&
                 session.Transaction().CursorEpoch() == epoch);
}

void PageRunArgumentsBorrowTheLastUsableRecord() {
  Row row;
  agiru::detail::PageRunRecord source;
  agiru::detail::TakePageRunRecord(source, row);
  CHECK_TRUE("typed page arguments borrow their writable source",
             source.record == &row && source.table == &agiru::TableTraits<Row>::kTable &&
                 source.writable);
  agiru::detail::TakePageRunRecord(source, std::as_const(row));
  CHECK_TRUE("const typed page arguments cannot write back",
             source.record == &row && !source.writable);
  agiru::RecordRef reference;
  reference.GetTable(row);
  agiru::detail::TakePageRunRecord(source, reference);
  CHECK_TRUE("open record references retain their owned record and table",
             source.record == reference.RecordPointer() &&
                 source.table == reference.TableDefinition() && source.writable);
  agiru::detail::TakePageRunRecord(source, std::as_const(reference));
  CHECK_TRUE("const record references cannot write back", !source.writable);
  agiru::Variant held(reference);
  agiru::detail::TakePageRunRecord(source, held);
  CHECK_TRUE("a Variant record reference retains the existing writable alias contract",
             source.record == reference.RecordPointer() && source.writable);
  agiru::RecordRef closed;
  agiru::detail::TakePageRunRecord(source, closed);
  CHECK_TRUE("a closed later argument preserves the last usable record",
             source.record == reference.RecordPointer());
  agiru::Variant scalar(kFirstValue);
  agiru::detail::TakePageRunRecord(source, scalar);
  CHECK_TRUE("a scalar Variant does not replace the record selection",
             source.record == reference.RecordPointer());
  agiru::Variant owned(row);
  agiru::detail::TakePageRunRecord(source, owned);
  const auto *record = owned.HeldRecord();
  CHECK_TRUE("a record Variant lends its own image rather than the original record",
             record != nullptr && source.record == record->RecordPointer() &&
                 source.record != &row && source.writable &&
                 source.table == &agiru::TableTraits<Row>::kTable);
}

void InstalledPageLifecycle() {
  auto list = agiru::MakeInstalledPage(agiru::PageTraits<List>::kId);
  auto card = agiru::MakeInstalledPage(agiru::PageTraits<Card>::kId);
  CHECK_TRUE("generated catalogue links a closed production factory",
             !list->IsOpen() && !card->IsOpen());
  list->Open(agiru::PageOpenMode::View);
  CHECK_TRUE("type-erased first uses the existing cursor", list->Move(agiru::PagePosition::First));
  CHECK_TEXT("first positions on the declared key order", list->Controls().ControlText("ID"), "1");
  CHECK_TRUE("type-erased next uses the existing cursor", list->Move(agiru::PagePosition::Next));
  CHECK_TEXT("next reaches the next SQL row", list->Controls().ControlText("ID"), "2");
  const agiru::RecordId selected = list->CurrentRecord();
  CHECK_TRUE("selected row retains its declared table identity",
             selected.TableNo() == agiru::TableTraits<Row>::kTable.id.Value());
  CHECK_TRUE("type-erased previous uses the existing cursor",
             list->Move(agiru::PagePosition::Previous));
  CHECK_TEXT("previous reaches the preceding SQL row", list->Controls().ControlText("ID"), "1");
  CHECK_TRUE("type-erased last uses the existing cursor", list->Move(agiru::PagePosition::Last));
  CHECK_TRUE("exact record identity repositions the production page", list->SelectRecord(selected));
  card->Open(agiru::PageOpenMode::Edit);
  CHECK_TRUE("list identity selects the same row in a production card",
             card->SelectRecord(selected));
  CHECK_TEXT("production selection runs the generated after-get trigger",
             card->Controls().ControlText("LoadedValue"),
             "22");
  CHECK_TEXT("production opening still exposes editing mode to AL",
             card->Controls().ControlText("OpeningMode"),
             "Yes");
  CHECK_TRUE("the selected identity remains exact after navigation",
             card->CurrentRecord() == selected);
  HtmlAuthorization authority;
  const auto html = agiru::RenderPageHtml(
      card->Declaration(),
      card->Controls(),
      authority,
      {.pageHandle = "card_1", .revision = "1", .commandPrefix = "render_1", .csrf = "fixture"});
  CHECK_TRUE("generated factory supplies exact record values to semantic HTML",
             html.html.contains("data-type=\"Integer\" data-value=\"22\""));
  CHECK_TRUE("semantic HTML qualifies bound variables without bypassing authority",
             html.unsupported == 0 && authority.calls == 17 &&
                 card->Controls().Control_Value("LoadedValue").value == "22" &&
                 card->Controls().Control_Value("OpeningMode").value == "true");
  CHECK_TRUE("rendering a generated page preserves its selected record",
             card->CurrentRecord() == selected);
  bool refused = false;
  try {
    static_cast<void>(list->Move(agiru::PagePosition::Unknown));
  } catch (const agiru::Error &error) { refused = error.Code() == "PagePosition"; }
  CHECK_TRUE("unknown movement refuses instead of changing selection",
             refused && list->CurrentRecord() == selected);
  list->Close();
  card->Close();
}

class WindowReceiver final : public agiru::PageWindowReceiver {
public:
  void Row(const agiru::RecordId &identity, agiru::PageCore &controls) override {
    identities.push_back(identity);
    ids.push_back(controls.ControlText("ID"));
    values.push_back(controls.Control_Value("Value").value);
    if (calculated) {
      loaded.push_back(controls.ControlText("Loaded"));
      originals.push_back(controls.ControlText("Original"));
      CHECK_TEXT("row triggers do not make every loaded row current",
                 controls.ControlText("CurrentCount"),
                 previousCurrentCount);
    }
  }

  void Current(const agiru::RecordId &identity, agiru::PageCore &controls) override {
    current = identity;
    if (!calculated) { return; }
    trace = controls.ControlText("Trace");
    readCount = controls.ControlText("ReadCount");
    currentCount = controls.ControlText("CurrentCount");
    currentValue = controls.Control_Value("Value").value;
    selectedOriginal = controls.ControlText("SelectedOriginal");
  }

  bool calculated = true;
  std::string previousCurrentCount = "0";
  std::vector<agiru::RecordId> identities;
  std::vector<std::string> ids;
  std::vector<std::string> values;
  std::vector<std::string> loaded;
  std::vector<std::string> originals;
  agiru::RecordId current;
  std::string trace;
  std::string readCount;
  std::string currentCount;
  std::string currentValue;
  std::string selectedOriginal;
};

void GeneratedListWindows() {
  using Window = agiru::Fixture::NavigationWindow_Page;
  auto block = agiru::MakeInstalledPage(agiru::PageTraits<Window>::kId);
  WindowReceiver pair;
  static_cast<void>(block->OpenWindow(agiru::PageOpenMode::View, kListRows, pair));
  CHECK_TEXT("selected row restores its original image after other rows were loaded",
             pair.selectedOriginal,
             "11");
  CHECK_TEXT("the selected trigger sees all completed row triggers", pair.trace, "OAAC");
  block->Close();
  auto list = agiru::MakeInstalledPage(agiru::PageTraits<Window>::kId);
  WindowReceiver first;
  const auto initial = list->OpenWindow(agiru::PageOpenMode::View, 1, first);
  CHECK_TRUE("generated list reads only the trusted bound and one unpresented probe",
             initial.rows == 1 && initial.rowsRead == 2 && initial.more && first.ids.size() == 1);
  CHECK_TEXT("opening runs each loaded trigger before the selected trigger", first.trace, "OAC");
  CHECK_TEXT("probe rows run no AL after-get trigger", first.readCount, "1");
  CHECK_TEXT("calculated controls are captured at their own row", first.loaded.front(), "22");
  CHECK_TEXT("loaded rows capture their original stored image before AL changes",
             first.originals.front(),
             "11");
  CHECK_TEXT(
      "current row retains AL buffer changes without rereading SQL", first.currentValue, "22");
  CHECK_TRUE("first SQL row remains selected after rendering",
             first.current == list->CurrentRecord());
  WindowReceiver next;
  next.previousCurrentCount = "1";
  const auto following = list->ReadWindow(agiru::PageWindowPosition::Next, 1, next);
  CHECK_TRUE("next continues at the retained SQL boundary",
             following.rows == 1 && following.rowsRead == 1 && !following.more);
  CHECK_TEXT("next row follows declared SQL order", next.ids.front(), "2");
  CHECK_TEXT("block movement runs exactly one selected trigger", next.trace, "OACAC");
  WindowReceiver all;
  all.previousCurrentCount = "2";
  const auto repeated = list->ReadWindow(agiru::PageWindowPosition::First, kListRows, all);
  CHECK_TRUE("forty-row block presents the complete small filtered source",
             repeated.rows == 2 && repeated.rowsRead == 2 && !repeated.more);
  CHECK_TRUE("all row callbacks precede current state and retain row-specific calculations",
             (all.values == std::vector<std::string>{"22", "44"} && all.values == all.loaded));
  CHECK_TRUE("each loaded row retains its own original image rather than the prior row",
             (all.originals == std::vector<std::string>{"11", "22"}));
  CHECK_TEXT("unchanged selection does not rerun its current trigger", all.trace, "OACACAA");
  CHECK_TRUE("reading a block keeps the selected row rather than the last visited row",
             all.current == next.current && list->CurrentRecord() == next.current);
  CHECK_TEXT("selected post-trigger record fields survive the block", all.currentValue, "44");
  WindowReceiver last;
  last.previousCurrentCount = "2";
  const auto ending = list->ReadWindow(agiru::PageWindowPosition::Last, 1, last);
  CHECK_TRUE("last window reports reverse continuation", ending.rows == 1 && ending.more);
  WindowReceiver previous;
  previous.previousCurrentCount = "2";
  const auto preceding = list->ReadWindow(agiru::PageWindowPosition::Previous, 1, previous);
  CHECK_TRUE("previous uses the first retained SQL boundary",
             preceding.rows == 1 && !preceding.more);
  CHECK_TEXT("previous returns declared forward-order data", previous.ids.front(), "1");
  CHECK_TEXT("selected-row callback follows all row triggers", previous.trace, "OACACAAAAC");
  WindowReceiver exhausted;
  const auto empty = list->ReadWindow(agiru::PageWindowPosition::Previous, kListRows, exhausted);
  CHECK_TRUE("exhausted continuation retains the previous block and selected row",
             empty.rows == 0 && exhausted.ids.empty() && exhausted.current == previous.current &&
                 list->CurrentRecord() == previous.current);
  CHECK_TEXT(
      "exhausted continuation runs no row/current triggers", exhausted.trace, previous.trace);
  list->Close();
  auto card = agiru::MakeInstalledPage(agiru::PageTraits<Card>::kId);
  bool refused = false;
  try {
    static_cast<void>(card->OpenWindow(agiru::PageOpenMode::View, kListRows, first));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageWindowProvider"; }
  CHECK_TRUE("unsupported page windows refuse before opening", refused && !card->IsOpen());
  auto invalid = agiru::MakeInstalledPage(agiru::PageTraits<Window>::kId);
  refused = false;
  try {
    static_cast<void>(invalid->OpenWindow(agiru::PageOpenMode::View, 0, first));
  } catch (const agiru::Error &error) { refused = error.Code() == "RecordWindowLimit"; }
  CHECK_TRUE("invalid trusted limits refuse before AL page initialization",
             refused && !invalid->IsOpen());
}

void CustomSourceNavigation() {
  using Custom = agiru::Fixture::NavigationCustom_Page;
  for (const bool temporary : {false, true}) {
    Custom original{};
    original.Configure(temporary, false, false, false);
    agiru::PageSession<Custom> page;
    page.Prepare_Borrowed(original);
    page.OpenView();
    CHECK_TEXT("custom opening invokes find instead of the default SQL provider",
               page.ControlText("ID"),
               temporary ? "7" : "2");
    CHECK_TEXT(
        "custom opening retains the exact find fallback order", page.ControlText("FindCalls"), "2");
    CHECK_TEXT("custom returned row captures its own xRec",
               page.ControlText("Original"),
               temporary ? "70" : "22");
    CHECK_TEXT("custom opening executes loaded-row calculations",
               page.ControlText("Loaded"),
               temporary ? "140" : "44");
    CHECK_TRUE("custom first is an explicit provider navigation", page.First());
    CHECK_TEXT(
        "custom first receives the AL first-record selector", page.ControlText("LastWhich"), "-");
    CHECK_TEXT(
        "custom first reruns the current-record trigger", page.ControlText("CurrentCount"), "2");
    CHECK_TRUE("custom next uses its declared result without SQL fallback",
               static_cast<bool>(page.Next()) == temporary);
    CHECK_TEXT("custom next receives the signed AL step", page.ControlText("LastSteps"), "1");
    if (temporary) {
      CHECK_TEXT("custom next returns a row absent from SQL", page.ControlText("ID"), "9");
      CHECK_TEXT(
          "custom next captures the returned row's xRec", page.ControlText("Original"), "90");
      CHECK_TRUE("custom previous uses the original provider", page.Previous());
      CHECK_TEXT(
          "custom previous receives a negative AL step", page.ControlText("LastSteps"), "-1");
      CHECK_TEXT("custom previous restores its exact temporary row", page.ControlText("ID"), "7");
    }
    CHECK_TRUE("custom last invokes its declared find", page.Last());
    CHECK_TEXT(
        "custom last receives the AL last-record selector", page.ControlText("LastWhich"), "+");
    CHECK_TEXT("custom last displays the provider's last row",
               page.ControlText("ID"),
               temporary ? "9" : "2");
    page.Close();
  }
  Custom refused{};
  refused.Configure(false, true, true, false);
  agiru::PageSession<Custom> empty;
  empty.Prepare_Borrowed(refused);
  empty.OpenView();
  CHECK_TRUE("custom find refusal never falls through to existing SQL rows", !empty.First());
  CHECK_TEXT(
      "custom find refusal runs no after-current trigger", empty.ControlText("CurrentCount"), "0");
  empty.Close();
  Custom stopped{};
  stopped.Configure(true, false, true, false);
  agiru::PageSession<Custom> stopping;
  stopping.Prepare_Borrowed(stopped);
  stopping.OpenView();
  CHECK_TRUE("custom zero next result does not adopt another source", !stopping.Next());
  CHECK_TEXT("custom zero next result retains the selected row", stopping.ControlText("ID"), "7");
  CHECK_TEXT("custom zero next result raises no current-row trigger",
             stopping.ControlText("CurrentCount"),
             "1");
  stopping.Close();
  agiru::PageSession<agiru::Fixture::NavigationFindOnly_Page> findOnly;
  findOnly.OpenView();
  CHECK_TRUE("a find-only provider retains default Next navigation", findOnly.Next());
  CHECK_TEXT("default Next reaches the next SQL row without a custom next trigger",
             findOnly.ControlText("ID"),
             "2");
  findOnly.Close();
}

void CustomNavigationErrorsCloseAndRollback() {
  using Custom = agiru::Fixture::NavigationCustom_Page;
  Custom original{};
  original.Configure(false, false, false, true);
  agiru::PageSession<Custom> page;
  page.Prepare_Borrowed(original);
  page.OpenView();
  bool refused = false;
  {
    const agiru::detail::Scope boundary;
    try {
      static_cast<void>(page.Next());
    } catch (const agiru::Error &error) {
      refused = std::string_view(error.what()) == "custom next failed";
    }
    CHECK_TRUE("a custom next error closes the page and retains its diagnostic",
               refused && !page.IsOpen());
  }
  const auto stored = agiru::Session::Current().Database().Execute(
      R"(SELECT "Value" FROM "Navigation Row" WHERE "ID"=2)");
  CHECK_TEXT("a failed custom navigation rolls back its SQL modification",
             stored.Value(0, 0).value_or(""),
             "22");
}

void CopiedBuffersRetainSeekAnchorsWithoutSharingRows() {
  constexpr agiru::Integer kTemporaryFirstValue = 101;
  constexpr agiru::Integer kTemporaryLastValue = 303;
  Row source;
  CHECK_TRUE("the ordinary copy source has an observed SQL row", source.Get(1));
  agiru::Temporary<Row> temporary;
  temporary.ID = 1;
  temporary.Value = kTemporaryFirstValue;
  temporary.Insert();
  temporary.ID = 3;
  temporary.Value = kTemporaryLastValue;
  temporary.Insert();
  temporary.Copy(source);
  CHECK_TRUE("ordinary-to-temporary Copy retains the destination's independent rows",
             temporary.Count() == 2 && temporary.Value == source.Value);
  CHECK_TRUE("Next after cross-provider Copy seeks from the copied buffer",
             temporary.Next() == 1 && temporary.ID == 3 && temporary.Value == kTemporaryLastValue);
  Row copied;
  copied.Copy(temporary);
  CHECK_TRUE("temporary-to-ordinary Copy does not turn SQL storage temporary",
             !copied.IsTemporary());
  CHECK_TRUE("a cross-provider SQL seek uses the copied key without sharing the temporary store",
             copied.Next(-1) == -1 && copied.ID == 2 && copied.Value == 22);
  CHECK_TRUE("copied cursor invalidation does not move or replace its source",
             temporary.ID == 3 && temporary.Value == kTemporaryLastValue && source.ID == 1 &&
                 source.Value == kFirstValue);
}

void CustomStoredWindows() {
  using Provider = agiru::Fixture::NavigationProviderWindow_Page;
  Provider original{};
  original.Configure(false, 0, true);
  agiru::PageWindowSession<Provider> page;
  page.Prepare_Borrowed(original);
  WindowReceiver first;
  const auto opened = page.Open_Window(agiru::PageOpenMode::View, 1, first);
  CHECK_TRUE("custom SQL windows retain the bound and one undisplayed source probe",
             opened.rows == 1 && opened.rowsRead == 2 && opened.more && first.ids.size() == 1);
  CHECK_TEXT(
      "custom SQL windows capture xRec before row buffer mutation", first.originals.at(0), "11");
  CHECK_TEXT("custom SQL windows present calculated fields", first.values.at(0), "22");
  CHECK_TEXT("custom SQL windows keep row triggers distinct from current", first.trace, "OAC");
  CHECK_TEXT(
      "custom SQL probe invokes the declared step only once", page.ControlText("NextCalls"), "1");
  CHECK_TEXT("custom SQL windows do not invoke after-get for the probe", first.readCount, "1");
  CHECK_TRUE("custom window identity is independent of mutated display keys",
             first.current == first.identities.at(0) &&
                 first.current == page.Window_Current_Record());
  WindowReceiver next;
  next.previousCurrentCount = "1";
  const auto continued = page.Read_Window(agiru::PageWindowPosition::Next, 1, next);
  CHECK_TRUE("custom windows continue from immutable source keys instead of mutated buffers",
             continued.rows == 1 && continued.rowsRead == 1 && !continued.more &&
                 next.ids.size() == 1);
  CHECK_TEXT(
      "custom forward continuation presents the second provider row", next.ids.at(0), "1002");
  CHECK_TEXT(
      "custom continuation has the second row's own before-image", next.originals.at(0), "22");
  WindowReceiver all;
  all.previousCurrentCount = "2";
  static_cast<void>(page.Read_Window(agiru::PageWindowPosition::First, kListRows, all));
  CHECK_TRUE("custom block reloading retains selection and ordered per-row calculations",
             all.current == next.current && (all.values == std::vector<std::string>{"22", "44"}));
  CHECK_TEXT(
      "custom selected image survives loading other source rows", all.selectedOriginal, "22");
  CHECK_TEXT("custom unchanged selection does not rerun after-current", all.currentCount, "2");
  CHECK_TRUE("custom retained selection uses its original identity",
             page.Select_Window_Record(first.identities.at(0)));
  CHECK_TEXT("custom selecting a mutated row recaptures its actual source image",
             page.ControlText("SelectedOriginal"),
             "11");
  Row missing;
  missing.ID = 3;
  bool refused = false;
  try {
    static_cast<void>(page.Select_Window_Record(missing.RecordId()));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageWindowSelection"; }
  CHECK_TRUE("custom selecting a row outside the retained block refuses without navigation",
             refused && page.Window_Current_Record() == first.current);
  page.Close();
}

void CustomTemporaryWindows() {
  using Provider = agiru::Fixture::NavigationProviderWindow_Page;
  constexpr agiru::Integer kPopulation = 5;
  Provider original{};
  original.Configure(true, kPopulation, true);
  agiru::PageWindowSession<Provider> page;
  page.Prepare_Borrowed(original);
  WindowReceiver first;
  const auto opened = page.Open_Window(agiru::PageOpenMode::View, 2, first);
  CHECK_TRUE("temporary custom windows bound visits without a whole-result materialization",
             opened.rows == 2 && opened.rowsRead == 3 && opened.more && first.ids.size() == 2);
  CHECK_TRUE("temporary custom row projections keep exact independent before-images",
             (first.originals == std::vector<std::string>{"10", "20"} &&
              first.values == std::vector<std::string>{"20", "40"}));
  CHECK_TEXT("temporary windows complete all row triggers before current", first.trace, "OAAC");
  WindowReceiver next;
  next.previousCurrentCount = "1";
  const auto continued = page.Read_Window(agiru::PageWindowPosition::Next, 2, next);
  CHECK_TRUE("temporary continuation uses the declared provider and excludes the boundary",
             continued.rows == 2 && continued.rowsRead == 3 && continued.more &&
                 (next.ids == std::vector<std::string>{"1003", "1004"}));
  WindowReceiver last;
  last.previousCurrentCount = "2";
  const auto ending = page.Read_Window(agiru::PageWindowPosition::Last, 2, last);
  CHECK_TRUE("custom last reverses visits into provider forward order, not client key sorting",
             ending.rows == 2 && ending.rowsRead == 3 && ending.more &&
                 (last.ids == std::vector<std::string>{"1004", "1005"}));
  CHECK_TEXT("custom reverse loading captures original images in presentation order",
             last.selectedOriginal,
             "40");
  CHECK_TRUE("temporary retained row absent from SQL can be selected through its provider",
             page.Select_Window_Record(last.identities.at(1)));
  CHECK_TEXT(
      "temporary selection does not substitute or reread SQL", page.ControlText("Value"), "100");
  WindowReceiver previous;
  previous.previousCurrentCount = "4";
  const auto preceding = page.Read_Window(agiru::PageWindowPosition::Previous, 2, previous);
  CHECK_TRUE("custom previous starts before the raw first boundary with no duplicate or skip",
             preceding.rows == 2 && preceding.more &&
                 (previous.ids == std::vector<std::string>{"1002", "1003"}));
  WindowReceiver beginning;
  beginning.previousCurrentCount = "5";
  const auto start = page.Read_Window(agiru::PageWindowPosition::Previous, 2, beginning);
  CHECK_TRUE("custom reverse exhaustion presents the final partial block",
             start.rows == 1 && start.rowsRead == 1 && !start.more &&
                 beginning.ids.at(0) == "1001");
  WindowReceiver exhausted;
  const auto empty = page.Read_Window(agiru::PageWindowPosition::Previous, 2, exhausted);
  CHECK_TRUE("exhausted custom continuation retains the block and selected source identity",
             empty.rows == 0 && empty.rowsRead == 0 && exhausted.ids.empty() &&
                 exhausted.current == beginning.current &&
                 page.Window_Current_Record() == beginning.current);
  CHECK_TEXT(
      "custom exhaustion preserves the selected post-trigger buffer", exhausted.currentValue, "20");
  CHECK_TEXT("custom exhaustion runs no extra after-get/current triggers",
             exhausted.trace,
             beginning.trace);
  CHECK_TEXT(
      "custom exhaustion preserves the selected before-image", exhausted.selectedOriginal, "10");
  page.Close();
}

void CustomWindowRefusals() {
  using Provider = agiru::Fixture::NavigationProviderWindow_Page;
  Provider original{};
  original.Configure(false, 0, false);
  original.ConfigureFailure(true, false, false, false);
  agiru::PageWindowSession<Provider> empty;
  empty.Prepare_Borrowed(original);
  WindowReceiver none;
  const auto missing = empty.Open_Window(agiru::PageOpenMode::View, kListRows, none);
  CHECK_TRUE("custom window false find never falls through to existing SQL rows",
             missing.rows == 0 && missing.rowsRead == 0 && !missing.more && none.ids.empty());
  CHECK_TEXT("custom window false find runs no current-row trigger", none.currentCount, "0");
  empty.Close();
  original.ConfigureFailure(false, true, false, false);
  agiru::PageWindowSession<Provider> stopped;
  stopped.Prepare_Borrowed(original);
  WindowReceiver one;
  const auto ended = stopped.Open_Window(agiru::PageOpenMode::View, kListRows, one);
  CHECK_TRUE("custom window zero Next ends its source without default SQL fallback",
             ended.rows == 1 && ended.rowsRead == 1 && !ended.more && one.ids.size() == 1);
  stopped.Close();
  original.ConfigureFailure(false, false, true, false);
  agiru::PageWindowSession<Provider> stalled;
  stalled.Prepare_Borrowed(original);
  bool refused = false;
  try {
    static_cast<void>(stalled.Open_Window(agiru::PageOpenMode::View, 1, none));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageWindowProgress"; }
  CHECK_TRUE("stalled custom navigation refuses even when the duplicate is only the probe",
             refused && !stalled.IsOpen());
  original.ConfigureFailure(false, false, false, true);
  agiru::PageWindowSession<Provider> failing;
  failing.Prepare_Borrowed(original);
  refused = false;
  {
    const agiru::detail::Scope boundary;
    try {
      static_cast<void>(failing.Open_Window(agiru::PageOpenMode::View, kListRows, none));
    } catch (const agiru::Error &error) {
      refused = std::string_view(error.what()) == "provider window next failed";
    }
    CHECK_TRUE("custom window Next errors close the page with the original diagnostic",
               refused && !failing.IsOpen());
  }
  const auto stored = agiru::Session::Current().Database().Execute(
      R"(SELECT "Value" FROM "Navigation Row" WHERE "ID"=1)");
  CHECK_TEXT("custom window Next errors roll back pending SQL writes",
             stored.Value(0, 0).value_or(""),
             "11");
}

void CustomWindowSelectionAndFilters() {
  using Provider = agiru::Fixture::NavigationProviderWindow_Page;
  constexpr agiru::Integer kPopulation = 5;
  Provider original{};
  original.Configure(true, kPopulation, true);
  original.Rec.SetRange(original.Rec.ID, 2, 4);
  original.Rec.Ascending(false);
  agiru::PageWindowSession<Provider> page;
  page.Prepare_Borrowed(original);
  WindowReceiver first;
  const auto initial = page.Open_Window(agiru::PageOpenMode::View, 2, first);
  CHECK_TRUE("custom temporary windows inherit filters and descending provider order",
             initial.rows == 2 && initial.more &&
                 (first.ids == std::vector<std::string>{"1004", "1003"}));
  WindowReceiver next;
  next.previousCurrentCount = "1";
  const auto following = page.Read_Window(agiru::PageWindowPosition::Next, 2, next);
  CHECK_TRUE("custom descending continuation preserves the source filter",
             following.rows == 1 && !following.more && next.ids.at(0) == "1002");
  WindowReceiver last;
  last.previousCurrentCount = "2";
  static_cast<void>(page.Read_Window(agiru::PageWindowPosition::Last, 2, last));
  CHECK_TRUE("custom last restores declared descending order instead of ascending PK order",
             (last.ids == std::vector<std::string>{"1003", "1002"}));
  CHECK_TRUE("custom selection can choose the other retained filtered row",
             page.Select_Window_Record(last.identities.at(0)));
  const auto selected = page.Window_Current_Record();
  original.ConfigureFailure(true, false, false, false);
  CHECK_TRUE("a missing custom selection restores the prior retained identity and buffer",
             !page.Select_Window_Record(last.identities.at(1)) &&
                 page.Window_Current_Record() == selected);
  CHECK_TEXT(
      "failed custom selection preserves post-trigger fields", page.ControlText("ID"), "1003");
  CHECK_TEXT("failed custom selection preserves the current before-image",
             page.ControlText("SelectedOriginal"),
             "30");
  bool refused = false;
  try {
    WindowReceiver invalid;
    static_cast<void>(page.Read_Window(agiru::PageWindowPosition::Next, 2, invalid));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageWindowAnchor"; }
  CHECK_TRUE("missing custom continuation anchors refuse and close instead of falling back to SQL",
             refused && !page.IsOpen());
}

void ListWindowErrorsCloseAndRollback() {
  using Failing = agiru::Fixture::NavigationWindowError_Page;
  auto list = agiru::MakeInstalledPage(agiru::PageTraits<Failing>::kId);
  WindowReceiver receiver;
  receiver.calculated = false;
  bool refused = false;
  {
    const agiru::detail::Scope boundary;
    try {
      static_cast<void>(list->OpenWindow(agiru::PageOpenMode::View, kListRows, receiver));
    } catch (const agiru::Error &error) {
      refused = std::string_view(error.what()).contains("Window trigger error");
    }
    CHECK_TRUE("a loaded-row AL error closes the page without successful completion",
               refused && !list->IsOpen());
    CHECK_TRUE("the continuation probe is never exposed as a completed row",
               receiver.ids.size() == 1);
  }
  const auto stored = agiru::Session::Current().Database().Execute(
      R"(SELECT "Value" FROM "Navigation Row" WHERE "ID" = 1)");
  CHECK_TEXT("the caller's rollback boundary undoes earlier row-trigger writes",
             stored.Value(0, 0).value_or(""),
             "11");
}

void CheckListWindowBoundary(agiru::PageInstance &page, std::size_t population, std::size_t bound) {
  WindowReceiver receiver;
  receiver.calculated = false;
  const auto state = page.OpenWindow(agiru::PageOpenMode::View, bound, receiver);
  CHECK_TRUE("generated windows respect zero/one/39/40/41 and configurable row bounds",
             state.rows == std::min(population, bound) &&
                 state.rowsRead == std::min(population, bound + 1) &&
                 state.more == (population > bound) && receiver.ids.size() == state.rows);
  CHECK_TRUE("display enumeration retains the first selected row, not its last",
             receiver.current.IsEmpty() == (population == 0) &&
                 (population == 0 || receiver.current == receiver.identities.front()));
  if (state.more) {
    WindowReceiver continuation;
    continuation.calculated = false;
    const auto next = page.ReadWindow(agiru::PageWindowPosition::Next, bound, continuation);
    CHECK_TRUE("generated continuation transfers at most bound plus one",
               next.rows == std::min(population - bound, bound) &&
                   next.rowsRead == std::min(population - bound, bound + 1));
    CHECK_TEXT("generated continuation does not skip the row after its boundary",
               continuation.ids.at(0),
               std::to_string(bound + 1));
  }
  page.Close();
}

void ListWindowBoundaries() {
  using Provider = agiru::Fixture::NavigationProviderWindow_Page;
  using FindOnly = agiru::Fixture::NavigationFindOnly_Page;
  constexpr std::array<std::size_t, 6> populations{0, 1, 39, 40, 41, 80};
  constexpr std::array<std::size_t, 3> bounds{7, 40, 80};
  const auto &database = agiru::Session::Current().Database();
  for (const auto population : populations) {
    database.Run(R"(DELETE FROM "Navigation Row")");
    for (std::size_t i = 1; i <= population; ++i) {
      database.Run(R"(INSERT INTO "Navigation Row" ("ID", "Value") VALUES ()" + std::to_string(i) +
                   "," + std::to_string(i) + ")");
    }
    for (const auto bound : bounds) {
      auto ordinary = agiru::MakeInstalledPage(agiru::PageTraits<List>::kId);
      CheckListWindowBoundary(*ordinary, population, bound);
      auto findOnly = agiru::MakeInstalledPage(agiru::PageTraits<FindOnly>::kId);
      CheckListWindowBoundary(*findOnly, population, bound);
      Provider original{};
      original.Configure(true, static_cast<agiru::Integer>(population), false);
      auto temporary = agiru::MakeInstalledPage(agiru::PageTraits<Provider>::kId);
      temporary->PrepareBorrowed(&original, agiru::PageTraits<Provider>::kId);
      CheckListWindowBoundary(*temporary, population, bound);
    }
  }
  Prepare();
}

void ListEditOpensSelectedCard() {
  CHECK_TRUE("generated metadata preserves CardPageId",
             agiru::PageTraits<List>::kPage.cardPageId == agiru::PageTraits<Card>::kId);
  Row row;
  row.Get(2);
  agiru::TestPage<List> list;
  agiru::TestPage<Card> card;
  list.OpenView();
  list.GoToRecord(row);
  CHECK_TRUE("the generated list is read-only", !list.Editable());
  card.Trap();
  list.Edit().Invoke();
  CHECK_TRUE("system Edit opens the declared card on the selected row", card.ID.AsInteger() == 2);
  CHECK_TRUE("the card keeps the selected record value", card.Value.AsInteger() == kSecondValue);
  CHECK_TRUE("OnOpenPage observes editing mode", card.OpeningMode.AsBoolean());
  CHECK_TRUE("the typed value primitive cannot shadow an AL ControlValue field",
             card.ControlValue.AsBoolean());
  CHECK_TRUE("card opening executes OnAfterGetRecord",
             card.LoadedValue.AsInteger() == kSecondValue);
  CHECK_TRUE("opening a card does not change the list mode", !list.Editable());
  card.Close();
  CHECK_TRUE("closing a card leaves the list on the same row", list.ID.AsInteger() == 2);
  list.Close();
}

void ExplicitEditAndStandaloneModes() {
  Row row;
  row.Get(2);
  agiru::TestPage<agiru::Fixture::NavigationOverride_Page> list;
  list.OpenView();
  list.GoToRecord(row);
  list.Edit().Invoke();
  CHECK_TRUE("an explicit Edit action runs instead of system navigation",
             list.Value.AsInteger() == kSecondValue + kOverrideIncrement);
  list.Close();
  agiru::TestPage<Card> card;
  card.OpenView();
  CHECK_TRUE("a standalone card starts in view mode", !card.Editable());
  card.Edit().Invoke();
  CHECK_TRUE("a card without CardPageId changes its own mode", card.Editable());
  card.View().Invoke();
  CHECK_TRUE("a standalone card can return to view mode", !card.Editable());
  card.Close();
}

void CardModificationPolicy() {
  agiru::TestPage<agiru::Fixture::NavigationBlockedList_Page> list;
  list.OpenView();
  bool refused = false;
  try {
    list.Edit().Invoke();
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).find("ModifyAllowed") != std::string_view::npos;
  }
  CHECK_TRUE("system Edit honors the card's ModifyAllowed policy", refused);
  list.Close();
}

void EmptyListDoesNotCreateACard() {
  Row row;
  row.DeleteAll();
  agiru::TestPage<List> list;
  agiru::TestPage<Card> card;
  list.OpenView();
  card.Trap();
  bool refused = false;
  try {
    list.Edit().Invoke();
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).find("no current record") != std::string_view::npos;
  }
  CHECK_TRUE("an empty list cannot open a new card through Edit", refused);
  CHECK_TRUE("refused navigation does not create a database row", row.Count() == 0);
  list.Close();
}

void ProductionLifecycleAndTestErrorPolicy() {
  using Delayed = agiru::Fixture::NavigationDelayed_Page;
  agiru::PageSession<Delayed> page;
  page.OpenNew();
  page.SetControlText("ID", "3");
  page.SetControlText("Value", "33");
  Row row;
  CHECK_TRUE("production delayed input does not insert before row leave", !row.Get(3));
  page.RowLeft();
  CHECK_TRUE("production row leave inserts through the AL page", row.Get(3));
  CHECK_TRUE("SQL row has the validated input", row.Value == 33);
  page.Close();

  agiru::PageSession<Card> card;
  card.OpenEdit();
  CHECK_TEXT(
      "production opening runs the generated page trigger", card.ControlText("OpeningMode"), "Yes");
  CHECK_TRUE("production positioning uses the typed record kernel", card.GoToKey(3));
  CHECK_TEXT(
      "positioning runs the generated after-get trigger", card.ControlText("LoadedValue"), "33");
  card.SetControlText("Value", "44");
  CHECK_TRUE("production edit keeps the same SQL identity", row.Get(3));
  CHECK_TRUE("production edit uses the existing page save path", row.Value == 44);
  bool refused = false;
  try {
    card.SetControlText("Value", "not an integer");
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValidation"; }
  CHECK_TRUE("invalid production input retains its validation classification", refused);
  CHECK_TEXT("invalid input restores the accepted record value", card.ControlText("Value"), "44");
  card.Close();

  page.OpenNew();
  page.SetControlText("ID", "4");
  page.SetControlText("Value", "-1");
  refused = false;
  try {
    page.RowLeft();
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()) == "negative row value";
  }
  CHECK_TRUE("production row-save errors propagate instead of successful collection", refused);
  CHECK_TRUE("production does not use the AL test error list", page.ValidationErrorCount() == 0);
  CHECK_TRUE("failed production insertion has no SQL row", !row.Get(4));
  page.Close();

  agiru::TestPage<Delayed> test;
  test.OpenNew();
  test.ID.SetValue(4);
  test.Value.SetValue(-1);
  test.RowLeft();
  CHECK_TRUE("AL test adapter retains its explicit collection policy",
             test.ValidationErrorCount() == 1);
  CHECK_TEXT("AL test row-save error retains the original text",
             test.GetValidationError(),
             "negative row value");
  CHECK_TRUE("test error collection does not create a SQL row", !row.Get(4));
  test.Close();
  const agiru::Result stored = agiru::Session::Current().Database().Execute(
      R"(SELECT "ID", "Value" FROM "Navigation Row" ORDER BY "ID")");
  CHECK_TRUE("independent SQL retains only the successful insertion", stored.Rows() == 1);
  CHECK_TEXT("independent SQL confirms the saved identity", stored.Value(0, 0).value_or(""), "3");
  CHECK_TEXT("independent SQL confirms the validated edit", stored.Value(0, 1).value_or(""), "44");
}

void SourceInsertedRecordsBecomeExistingPageRows() {
  using Created = agiru::Fixture::NavigationCreated_Page;
  constexpr agiru::Integer kCreatedBase = 910;
  constexpr agiru::Integer kEnteredValue = 71;
  for (agiru::Integer mode = 1; mode <= 3; ++mode) {
    Created original;
    original.Configure(mode, kCreatedBase + mode);
    agiru::PageSession<Created> page;
    page.Prepare_Borrowed(original);
    page.OpenNew();
    const auto identity = original.Rec.SystemId;
    page.SetControlText("Value", "71");
    Row observed;
    CHECK_TRUE("source-inserted new pages persist an ordinary field before leaving",
               observed.Get(kCreatedBase + mode) && observed.Value == kEnteredValue);
    CHECK_TRUE("source-inserted pages retain their original SystemId",
               mode == 3 || observed.SystemId == identity);
    CHECK_TRUE("source insertion reconciles both page new-record markers",
               !original.StandsOnNewRecord());
    page.RowLeft();
    page.Close();
    CHECK_TRUE("leaving a source-inserted page never inserts a duplicate",
               observed.Get(kCreatedBase + mode) && observed.Value == kEnteredValue);
  }
}

void PendingNewRowsDoNotBecomeUnrelatedExistingRows() {
  using Created = agiru::Fixture::NavigationCreated_Page;
  constexpr agiru::Integer kPendingId = 940;
  constexpr agiru::Integer kStoredValue = 19;
  Row existing;
  existing.ID = kPendingId;
  existing.Value = kStoredValue;
  existing.Insert();
  for (const bool matchingId : {false, true}) {
    Created original;
    original.Configure(0, matchingId ? kPendingId + 1 : kPendingId);
    agiru::PageSession<Created> page;
    page.Prepare_Borrowed(original);
    page.OpenNew();
    original.Rec.SystemId = matchingId ? existing.SystemId : agiru::Guid::Create();
    bool refused = false;
    try {
      page.SetControlText("Value", "72");
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("pending pages require both stored primary key and SystemId",
               !refused && original.StandsOnNewRecord() && existing.Get(kPendingId) &&
                   existing.Value == kStoredValue);
    CHECK_TRUE("a copied SystemId does not create the changed primary key",
               !existing.Get(kPendingId + 1));
    static_cast<void>(page.Close_Modal(agiru::Action::Cancel));
  }
  Created original;
  original.Configure(0, kPendingId + 2);
  agiru::PageSession<Created> page;
  page.Prepare_Borrowed(original);
  page.OpenNew();
  {
    agiru::detail::Scope rollback;
    original.Rec.Insert();
    rollback.Rollback();
  }
  page.SetControlText("Value", "73");
  CHECK_TRUE("rolled-back source insertion leaves a pending page, not a stored row",
             original.StandsOnNewRecord() && !existing.Get(kPendingId + 2));
  static_cast<void>(page.Close_Modal(agiru::Action::Cancel));
}

void TemporarySourceInsertedRowsRemainTemporary() {
  using Created = agiru::Fixture::NavigationCreated_Page;
  constexpr agiru::Integer kTemporaryId = 950;
  agiru::Temporary<Row> temporary;
  Created original;
  agiru::detail::RuntimeAdoptTemporary(&original.Rec, &temporary);
  original.Configure(1, kTemporaryId);
  agiru::PageSession<Created> page;
  page.Prepare_Borrowed(original);
  page.OpenNew();
  page.SetControlText("Value", "74");
  Row observed;
  CHECK_TRUE("source-inserted temporary pages save within their own rows",
             original.Rec.Get(kTemporaryId) && original.Rec.Value == 74);
  CHECK_TRUE("temporary source insertion never persists a SQL row", !observed.Get(kTemporaryId));
  page.RowLeft();
  page.Close();
}

void TestHandlesRebindTheirGeneratedControls() {
  agiru::TestPage<Card> owner;
  owner.OpenEdit();
  CHECK_TRUE("test handle positions before copying", owner.GoToKey(3));
  CHECK_TRUE("production Open does not shadow an AL Open control", owner.Open.AsBoolean());
  CHECK_TRUE("production Move does not shadow an AL Move control", owner.Move.AsInteger() == 44);
  CHECK_TRUE("production Declaration does not shadow an AL control",
             owner.Declaration.AsInteger() == 44);
  {
    agiru::TestPage<Card> copied(owner);
    copied.Value.SetValue(kCopiedValue);
    CHECK_TRUE("copy construction retains a handle on the same page",
               owner.Value.AsInteger() == 55);
    agiru::TestPage<Card> assigned;
    assigned = copied;
    CHECK_TRUE("copy assignment rebinds generated controls", assigned.Value.AsInteger() == 55);
    CHECK_TEXT("copy construction binds the filter adapter", copied.Filter.GetFilter("ID"), "");
  }
  agiru::TestPage<Card> moved(std::move(owner));
  CHECK_TRUE("move construction transfers the owning page", moved.IsOpen());
  CHECK_TRUE("move construction rebinds generated fields", moved.Value.AsInteger() == 55);
  agiru::TestPage<Card> assigned;
  assigned = std::move(moved);
  CHECK_TRUE("move assignment transfers the owning page", assigned.IsOpen());
  CHECK_TRUE("move assignment rebinds generated fields", assigned.Value.AsInteger() == 55);
  assigned.Close();
}

void RequestPageAdaptersRetainFieldsAndFilters() {
  using Report = agiru::Fixture::NavigationReport_Report;
  Report report;
  CHECK_TRUE("request page defaults to enabled", report.RequestPageEnabled());
  report.SetRequestPageEnabled(false);
  CHECK_TRUE("AL property setter and native getter share the flag", !report.UseRequestPage());
  Report copy = report;
  copy.SetRequestPageEnabled(true);
  CHECK_TRUE("copied request flag has independent ownership", !report.RequestPageEnabled());
  CHECK_TRUE("copied request flag is enabled", copy.UseRequestPage());
  report.UseRequestPage(true);
  CHECK_TRUE("native setter and AL property getter share the flag", report.RequestPageEnabled());
  agiru::Fixture::NavigationPropertyConsumer_Codeunit consumer;
  CHECK_TRUE("codeunits set and read report instance properties", !consumer.Configure(copy, false));
  CHECK_TRUE("a codeunit property write retains the separate report owner",
             report.UseRequestPage());
  Row view;
  CHECK_TRUE("table methods set and read report instance properties", !view.ReportRequestFlag());
  view.SetRange(view.ID, 3);
  report.SetTableView(view);
  {
    agiru::TestRequestPage<Report> request;
    request.Adopt(&report);
    CHECK_TEXT("request adoption retains the report dataitem view",
               request.Rows.GetFilter(request.Rows.ID).Value(),
               "3");
    request.Limit.SetValue(kRequestLimit);
    CHECK_TRUE("request fields retain their generated bindings", request.Limit.AsInteger() == 7);
    request.Rows.SetRange(request.Rows.ID, 4);
    request.SaveAsXml("", "");
    CHECK_TRUE("request XML action delegates to the live report",
               report.ClosedWith() == agiru::Action::OK);
  }
  {
    agiru::TestRequestPage<Report> request;
    request.Adopt(&report);
    CHECK_TEXT("accepted request filters return to the report",
               request.Rows.GetFilter(request.Rows.ID).Value(),
               "4");
    CHECK_TRUE("adoption reuses the same report globals", request.Limit.AsInteger() == 7);
    request.Rows.SetRange(request.Rows.ID, 3);
    report.CloseWith(agiru::Action::Cancel);
  }
  {
    agiru::TestRequestPage<Report> request;
    request.Adopt(&report);
    CHECK_TEXT("cancelled request filters do not replace the accepted view",
               request.Rows.GetFilter(request.Rows.ID).Value(),
               "4");
  }
}

void PagesSurviveSeparateClientCommands() {
  const gate::OwnedDatabase database("persistent_pages");
  const agiru::Guid principal("00000000-0000-0000-0000-000000000001");
  {
    const agiru::Session seed(database.Dsn());
    agiru::CreateTable(seed.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
    agiru::platform::User user;
    user.UserSecurityID = principal;
    user.UserName = "Page user";
    user.Insert();
    Prepare();
    agiru::Commit();
  }
  agiru::Session persistent(principal);
  persistent.TablePermissions(std::make_shared<NavigationTableAuthority>());
  auto list = agiru::MakeInstalledPage(agiru::PageTraits<List>::kId);
  auto card = agiru::MakeInstalledPage(agiru::PageTraits<Card>::kId);
  CommandAuthorization authorization;
  {
    agiru::Connection connection(database.Dsn());
    agiru::SessionCommand command(persistent, connection);
    list->Open(agiru::PageOpenMode::View);
    CHECK_TRUE("persistent list positions on its first SQL row",
               list->Move(agiru::PagePosition::First));
    CHECK_TRUE("persistent list selects the next SQL row", list->Move(agiru::PagePosition::Next));
    const agiru::RecordId selected = list->CurrentRecord();
    card->Open(agiru::PageOpenMode::Edit);
    CHECK_TRUE("a persistent card selects the list's exact record", card->SelectRecord(selected));
    command.Keep();
  }
  {
    agiru::Connection connection(database.Dsn());
    agiru::SessionCommand command(persistent, connection);
    CHECK_TRUE("both generated production instances remain open between commands",
               list->IsOpen() && card->IsOpen());
    CHECK_TEXT("the card retains its selected row", card->Controls().ControlText("ID"), "2");
    agiru::PageDispatcher dispatcher(card->Declaration(), card->Controls(), authorization);
    static_cast<void>(dispatcher.Execute(
        {.operation = agiru::PageControlOperation::Set, .control = "Value", .text = "55"}));
    CHECK_TEXT("typed dispatch reads the validated value on the persistent card",
               dispatcher.Execute({agiru::PageControlOperation::ReadValue, "Value", ""}).text,
               "55");
    command.Keep();
  }
  const agiru::Connection observer(database.Dsn());
  const auto stored = observer.Execute(
      R"(SELECT "Value", "SystemModifiedBy"::text FROM "Navigation Row" WHERE "ID" = 2)");
  CHECK_TRUE("the page edit is committed independently of the next browser command",
             stored.Rows() == 1);
  CHECK_TEXT("independent SQL sees the shared kernel's saved value",
             stored.Value(0, 0).value_or(""),
             "55");
  CHECK_TEXT("independent SQL attributes the page edit to its authenticated user",
             stored.Value(0, 1).value_or(""),
             principal.ToStorageText());
  {
    agiru::Connection connection(database.Dsn());
    agiru::SessionCommand command(persistent, connection);
    CHECK_TRUE("a committed cursor is safely reopened for later list navigation",
               list->Move(agiru::PagePosition::Previous));
    CHECK_TEXT("later list navigation preserves filtered key order",
               list->Controls().ControlText("ID"),
               "1");
    card->Close();
    list->Close();
    command.Keep();
  }
  CHECK_TRUE("client page closure does not retain an active database lease",
             persistent.Transaction().Depth() == 0);
}

}

int main(int argc, char **argv) {
  return gate::Run("Generated Page Navigation", [argc, argv] {
    if (argc != 2) { throw std::runtime_error("expected the dedicated gate database DSN"); }
    BorrowedModalLifecycle(argv[1]);
    ModalCallbacksRefuseBeforeOpening(argv[1]);
    const agiru::Session session(argv[1]);
    const agiru::detail::Scope isolation;
    Prepare();
    PageRunArgumentsBorrowTheLastUsableRecord();
    InstalledPageLifecycle();
    GeneratedListWindows();
    CustomSourceNavigation();
    CustomNavigationErrorsCloseAndRollback();
    CopiedBuffersRetainSeekAnchorsWithoutSharingRows();
    CustomStoredWindows();
    CustomTemporaryWindows();
    CustomWindowRefusals();
    CustomWindowSelectionAndFilters();
    ListWindowErrorsCloseAndRollback();
    ListWindowBoundaries();
    ListEditOpensSelectedCard();
    ExplicitEditAndStandaloneModes();
    CardModificationPolicy();
    EmptyListDoesNotCreateACard();
    ProductionLifecycleAndTestErrorPolicy();
    SourceInsertedRecordsBecomeExistingPageRows();
    PendingNewRowsDoNotBecomeUnrelatedExistingRows();
    TemporarySourceInsertedRowsRemainTemporary();
    TestHandlesRebindTheirGeneratedControls();
    RequestPageAdaptersRetainFieldsAndFilters();
    PagesSurviveSeparateClientCommands();
  });
}
