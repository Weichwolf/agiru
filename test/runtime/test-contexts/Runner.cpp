#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "type/Boolean.h"
#include "type/DataSourceContext.h"
#include "type/Guid.h"
#include "type/Integer.h"

#include "Check.h"
#include "TestContextScope.h"
#include "fixture/codeunit/TestContextConsumer.h"
#include "fixture/page/TryPage.h"
#include "fixture/table/TryRow.h"

#include <stdexcept>
#include <string_view>

namespace {

void SortContexts() {
  agiru::Fixture::TryRow_Table row;
  CHECK_TRUE("implicit record sorting accepts an unindexed field", row.SortValue());
  CHECK_TEXT("the selected key remains visible to AL", row.CurrentKey(), "Sort Order");
  CHECK_TRUE("consumed implicit Blob sorting returns false", !row.SortPayload());
  bool raised = false;
  try {
    row.DiscardPayloadSort();
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()).find("not sortable") != std::string_view::npos;
  }
  CHECK_TRUE("discarded implicit Blob sorting raises", raised);
  agiru::Fixture::TestContextConsumer_Codeunit consumer;
  CHECK_TRUE("named record sorting accepts an unindexed field", consumer.SelectValue(row));
  CHECK_TRUE("consumed FlowFilter sorting returns false", !consumer.SelectFilter(row));
  raised = false;
  try {
    consumer.DiscardFilterSort(row);
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()).find("not sortable") != std::string_view::npos;
  }
  CHECK_TRUE("discarded named FlowFilter sorting raises", raised);
  CHECK_TEXT("failed sorting preserves the preceding key", row.CurrentKey(), "Sort Order");
}

void TryScopes() {
  agiru::Fixture::TryRow_Table row;
  agiru::ClearLastError();
  CHECK_TRUE("a consumed table-local TryFunction catches its error", !row.CatchLocal());
  CHECK_TEXT("table-local calls retain their exact last error",
             agiru::GetLastErrorText(),
             "table failure");
  CHECK_TRUE("the caught call preserves its preceding field mutation", row.Value == 1);
  CHECK_TRUE("an implicit record receiver resolves the same TryFunction", !row.CatchRecord());
  CHECK_TRUE("qualified calls use the actual record variable", row.Value == 2);
  bool raised = false;
  try {
    row.DiscardLocal();
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()) == "table failure";
  }
  CHECK_TRUE("discarded table-local TryFunctions still raise", raised && row.Value == 3);
  CHECK_TRUE("a successful early exit remains true", row.SucceedLocal());
  CHECK_TRUE("assignment statements consume their right-hand TryFunction", !row.CatchAssigned());
  CHECK_TRUE("assignment catches retain their preceding mutation", row.Value == 4);
  auto valueBefore = row.Value;
  CHECK_TRUE("conditions consume their TryFunction", !row.CatchConditional());
  CHECK_TRUE("conditional catches retain their preceding mutation", row.Value == valueBefore + 1);
  valueBefore = row.Value;
  CHECK_TRUE("unary expressions consume their TryFunction", row.CatchNegated());
  CHECK_TRUE("unary catches retain their preceding mutation", row.Value == valueBefore + 1);
  valueBefore = row.Value;
  CHECK_TRUE("case selectors consume their TryFunction", row.CatchSelected() == 2);
  CHECK_TRUE("case catches retain their preceding mutation", row.Value == valueBefore + 1);
  agiru::Fixture::TryPage_Page page;
  CHECK_TRUE("page-local TryFunctions catch consumed failures", !page.CatchLocal());
  CHECK_TEXT("page-local calls replace the last error", agiru::GetLastErrorText(), "page failure");
  CHECK_TRUE("page record calls resolve the source table's attributes", !page.CatchRecord());
  CHECK_TEXT(
      "page record calls replace the last error", agiru::GetLastErrorText(), "table failure");
  raised = false;
  try {
    page.DiscardLocal();
  } catch (const agiru::Error &error) { raised = std::string_view(error.what()) == "page failure"; }
  CHECK_TRUE("discarded page-local TryFunctions still raise", raised);
  agiru::Fixture::TestContextConsumer_Codeunit consumer;
  CHECK_TRUE("codeunit self receivers retain TryFunction attributes", !consumer.CatchSelf());
  CHECK_TEXT(
      "self calls retain their exact last error", agiru::GetLastErrorText(), "codeunit failure");
  raised = false;
  try {
    consumer.DiscardSelf();
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()) == "codeunit failure";
  }
  CHECK_TRUE("discarded self calls still raise", raised);
  agiru::Boolean result = true;
  consumer.DiscardOuter(result);
  CHECK_TRUE("discarding an outer result still consumes its argument", !result);
  result = true;
  CHECK_TRUE("a consumed nested TryFunction does not fail the outer call",
             consumer.CatchOuter(result));
  CHECK_TRUE("the outer call receives the caught argument's false value", !result);
  result = true;
  CHECK_TRUE("ordinary argument errors belong inside the outer catch",
             !consumer.CatchArgumentError(result));
  CHECK_TRUE("a failed argument prevents the outer body from running", result);
  CHECK_TEXT("ordinary argument errors retain their exact message",
             agiru::GetLastErrorText(),
             "argument failure");
  agiru::Integer count = 1;
  CHECK_TRUE("ordinary case selectors retain their first result across ranges",
             consumer.SelectOnce(count) == 2);
  CHECK_TRUE("range comparisons do not repeat a selector's side effects", count == 2);
  count = 3;
  CHECK_TRUE("nonmatching cases use the else branch", consumer.SelectOnce(count) == 0);
  CHECK_TRUE("the else branch does not repeat the selector", count == 4);
}

}

int main(int argc, char **argv) {
  return gate::Run("Generated Test Context", [argc, argv] {
    if (argc != 2) { throw std::runtime_error("expected the dedicated gate database DSN"); }
    const agiru::Session session(argv[1]);
    TryScopes();
    SortContexts();
    constexpr agiru::CodeunitId kTest{50250};
    constexpr agiru::CodeunitId kProvider{50251};
    const agiru::Guid app("118874ab-44bc-4ccb-9daf-59763539ab16");
    const auto source = agiru::detail::makeDataSourceContext(app, kProvider);
    agiru::Fixture::TestContextConsumer_Codeunit consumer;
    CHECK_TRUE("AL property syntax returns the provider ID",
               consumer.Provider(source) == kProvider.Value());
    CHECK_TRUE("AL method syntax returns the declaring test app", consumer.TestApp(source) == app);
    using agiru::detail::TestContextPhase;
    using agiru::detail::TestContextScope;
    const TestContextScope before({.codeunit = kTest,
                                   .qualifiedName = "Microsoft.Fixture.Test Context Consumer",
                                   .procedure = "Read",
                                   .caseName = "case-one"},
                                  TestContextPhase::BeforeCase);
    CHECK_TEXT("generated AL retains exact typed metadata",
               std::string_view(consumer.Identity(before.Context())),
               "50250|Microsoft.Fixture.Test Context Consumer|Read|case-one|No");
    consumer.CopySkip(before.Context());
    CHECK_TRUE("generated AL value copies request skips", before.Skipped());
    CHECK_TEXT(
        "generated skip reasons survive the copy", before.SkipReason(), "requested by AL copy");
    const TestContextScope after({.codeunit = kTest,
                                  .qualifiedName = "Microsoft.Fixture.Test Context Consumer",
                                  .procedure = "Read",
                                  .caseName = "case-one"},
                                 TestContextPhase::AfterCase,
                                 true);
    consumer.CopySkip(after.Context());
    CHECK_TRUE("generated after hooks cannot skip completed work", !after.Skipped());
    CHECK_TEXT("generated success is meaningful after execution",
               std::string_view(consumer.Identity(after.Context())),
               "50250|Microsoft.Fixture.Test Context Consumer|Read|case-one|Yes");
  });
}
