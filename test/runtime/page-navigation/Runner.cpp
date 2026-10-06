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
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/TablePermissions.h"
#include "runtime/Transaction.h"
#include "runtime/test/TestPage.h"
#include "runtime/test/TestRequestPage.h"
#include "type/Action.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Variant.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "fixture/codeunit/NavigationPropertyConsumer.h"
#include "fixture/page/NavigationBlockedList.h"
#include "fixture/page/NavigationCard.h"
#include "fixture/page/NavigationDelayed.h"
#include "fixture/page/NavigationList.h"
#include "fixture/page/NavigationOverride.h"
#include "fixture/report/NavigationReport.h"
#include "fixture/table/NavigationRow.h"

#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {

using Row = agiru::Fixture::NavigationRow_Table;
using Card = agiru::Fixture::NavigationCard_Page;
using List = agiru::Fixture::NavigationList_Page;
constexpr agiru::Integer kFirstValue = 11;
constexpr agiru::Integer kSecondValue = 22;
constexpr agiru::Integer kOverrideIncrement = 100;
constexpr agiru::Integer kCopiedValue = 55;
constexpr agiru::Integer kRequestLimit = 7;

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
  CHECK_TRUE("semantic HTML keeps unqualified computed values visible as gaps",
             html.unsupported == 6 && authority.calls == 17);
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
    const agiru::Session session(argv[1]);
    const agiru::detail::Scope isolation;
    Prepare();
    PageRunArgumentsBorrowTheLastUsableRecord();
    InstalledPageLifecycle();
    ListEditOpensSelectedCard();
    ExplicitEditAndStandaloneModes();
    CardModificationPolicy();
    EmptyListDoesNotCreateACard();
    ProductionLifecycleAndTestErrorPolicy();
    TestHandlesRebindTheirGeneratedControls();
    RequestPageAdaptersRetainFieldsAndFilters();
    PagesSurviveSeparateClientCommands();
  });
}
