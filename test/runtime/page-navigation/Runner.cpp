#include "meta/PageDef.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageSession.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Transaction.h"
#include "runtime/test/TestPage.h"
#include "runtime/test/TestRequestPage.h"
#include "type/Action.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/page/NavigationBlockedList.h"
#include "fixture/page/NavigationCard.h"
#include "fixture/page/NavigationDelayed.h"
#include "fixture/page/NavigationList.h"
#include "fixture/page/NavigationOverride.h"
#include "fixture/report/NavigationReport.h"
#include "fixture/table/NavigationRow.h"

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
  Row view;
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

}

int main(int argc, char **argv) {
  return gate::Run("Generated Page Navigation", [argc, argv] {
    if (argc != 2) { throw std::runtime_error("expected the dedicated gate database DSN"); }
    const agiru::Session session(argv[1]);
    const agiru::detail::Scope isolation;
    Prepare();
    ListEditOpensSelectedCard();
    ExplicitEditAndStandaloneModes();
    CardModificationPolicy();
    EmptyListDoesNotCreateACard();
    ProductionLifecycleAndTestErrorPolicy();
    TestHandlesRebindTheirGeneratedControls();
    RequestPageAdaptersRetainFieldsAndFilters();
  });
}
