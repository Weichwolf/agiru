#include "meta/Ids.h"
#include "type/DataSourceContext.h"
#include "type/Guid.h"

#include "Check.h"
#include "TestContextScope.h"
#include "fixture/codeunit/TestContextConsumer.h"

#include <string_view>

int main() {
  return gate::Run("Generated Test Context", [] {
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
