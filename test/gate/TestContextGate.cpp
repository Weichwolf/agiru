#include "meta/Ids.h"
#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/DataSourceContext.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/TestHandlerContext.h"

#include "Check.h"
#include "TestContextScope.h"

#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <utility>

namespace {

using agiru::detail::TestContextPhase;
using agiru::detail::TestContextScope;
constexpr agiru::CodeunitId kTest{50250};
constexpr agiru::CodeunitId kProvider{50251};
constexpr std::string_view kName = "Fixture.Test Contexts";

static_assert(std::same_as<decltype(agiru::DataSourceContext{}.AppId()), agiru::Guid>);
static_assert(std::same_as<decltype(agiru::DataSourceContext{}.CodeunitId()), agiru::Integer>);
static_assert(std::same_as<decltype(agiru::TestHandlerContext{}.CodeunitId()), agiru::Integer>);
static_assert(std::same_as<decltype(agiru::TestHandlerContext{}.Success()), agiru::Boolean>);

template <typename Call> bool Refuses(Call &&call) {
  try {
    std::forward<Call>(call)();
  } catch (const agiru::Error &) { return true; }
  return false;
}

void EveryOriginalPhaseKeepsItsOwnIdentityAndAuthority() {
  constexpr std::array phases{TestContextPhase::BeforeCodeunit,
                              TestContextPhase::AfterCodeunit,
                              TestContextPhase::BeforeProcedure,
                              TestContextPhase::AfterProcedure,
                              TestContextPhase::BeforeCase,
                              TestContextPhase::AfterCase};
  for (const auto phase : phases) {
    const bool codeunit =
        phase == TestContextPhase::BeforeCodeunit || phase == TestContextPhase::AfterCodeunit;
    const bool dataCase =
        phase == TestContextPhase::BeforeCase || phase == TestContextPhase::AfterCase;
    const bool after = phase == TestContextPhase::AfterCodeunit ||
                       phase == TestContextPhase::AfterProcedure ||
                       phase == TestContextPhase::AfterCase;
    const bool canSkip =
        phase == TestContextPhase::BeforeProcedure || phase == TestContextPhase::BeforeCase;
    const TestContextScope scope({.codeunit = kTest,
                                  .qualifiedName = kName,
                                  .procedure = codeunit ? "" : "Run",
                                  .caseName = dataCase ? "first" : ""},
                                 phase,
                                 true);
    const auto context = scope.Context();
    CHECK_TRUE("consumer codeunit identity survives every phase",
               context.CodeunitId() == kTest.Value());
    CHECK_TEXT("full AL spelling is retained", std::string_view(context.CodeunitName()), kName);
    CHECK_TEXT("procedure exists only at procedure/case level",
               std::string_view(context.ProcedureName()),
               codeunit ? "" : "Run");
    CHECK_TEXT("case identifier exists only at case level",
               std::string_view(context.TestCaseName()),
               dataCase ? "first" : "");
    CHECK_TRUE("before phases cannot claim success", context.Success() == after);
    agiru::TestHandlerContext copied(context);
    copied.Skip("owned reason");
    CHECK_TRUE("only permitted before phases change skip state", scope.Skipped() == canSkip);
    CHECK_TEXT("AL value copies share the live skip reason",
               scope.SkipReason(),
               canSkip ? "owned reason" : "");
    copied = {};
    CHECK_TRUE("rebinding a copied context leaves the original identity intact",
               context.CodeunitId() == kTest.Value());
    CHECK_TRUE("a rebound empty context cannot invent its old identity",
               Refuses([&] { static_cast<void>(copied.CodeunitId()); }));
  }
}

void CopiesDoNotExtendSkipAuthorityOrBorrowCallerStrings() {
  agiru::TestHandlerContext retained;
  {
    std::string reason = "temporary reason";
    const TestContextScope scope({.codeunit = kTest, .qualifiedName = kName, .procedure = "Run"},
                                 TestContextPhase::BeforeProcedure);
    retained = scope.Context();
    retained.Skip(reason);
    reason.clear();
    CHECK_TEXT("skip reasons are owned", scope.SkipReason(), "temporary reason");
    retained.Skip("");
    CHECK_TRUE("empty reasons still request skips", scope.Skipped());
    scope.finish();
    retained.Skip("expired authority");
    CHECK_TEXT("expired value copies cannot change the completed reason", scope.SkipReason(), "");
  }
  retained.Skip("expired authority");
  CHECK_TEXT(
      "retained immutable metadata survives", std::string_view(retained.CodeunitName()), kName);
  const TestContextScope next({.codeunit = kTest, .qualifiedName = kName, .procedure = "Run"},
                              TestContextPhase::BeforeProcedure);
  CHECK_TRUE("a later invocation does not inherit a skip", !next.Skipped());
  const TestContextScope other(
      {.codeunit = kProvider, .qualifiedName = "Other.Run", .procedure = "Second"},
      TestContextPhase::BeforeProcedure);
  other.Context().Skip("other invocation");
  CHECK_TRUE("independent owners cannot skip each other's work", !next.Skipped());
}

void ProviderIdentityIsNotTheConsumerIdentity() {
  const agiru::Guid app("118874ab-44bc-4ccb-9daf-59763539ab16");
  const auto source = agiru::detail::makeDataSourceContext(app, kProvider);
  const auto copied = source;
  CHECK_TRUE("source context names the provider, not the test",
             copied.CodeunitId() == kProvider.Value());
  CHECK_TRUE("source context preserves the test app", copied.AppId() == app);
  CHECK_TRUE("zero provider identities refuse", Refuses([&] {
               static_cast<void>(agiru::detail::makeDataSourceContext(app, agiru::CodeunitId{}));
             }));
}

void InvalidAndUnboundContextsRefuseInsteadOfInventingMetadata() {
  CHECK_TRUE("unbound handler context has no guessed ID",
             Refuses([] { static_cast<void>(agiru::TestHandlerContext{}.CodeunitId()); }));
  CHECK_TRUE("unbound source context has no guessed app",
             Refuses([] { static_cast<void>(agiru::DataSourceContext{}.AppId()); }));
  CHECK_TRUE(
      "case identity cannot leak into a procedure scope", Refuses([] {
        const TestContextScope invalid(
            {.codeunit = kTest, .qualifiedName = kName, .procedure = "Run", .caseName = "first"},
            TestContextPhase::BeforeProcedure);
      }));
  CHECK_TRUE("procedure identity cannot leak into a codeunit scope", Refuses([] {
               const TestContextScope invalid(
                   {.codeunit = kTest, .qualifiedName = kName, .procedure = "Run"},
                   TestContextPhase::BeforeCodeunit);
             }));
  CHECK_TRUE("invalid phases never acquire authority", Refuses([] {
               const TestContextScope invalid(
                   {.codeunit = kTest, .qualifiedName = kName, .procedure = "Run"},
                   TestContextPhase::Invalid);
             }));
}

}

int main() {
  return gate::Run("TestContext", [] {
    EveryOriginalPhaseKeepsItsOwnIdentityAndAuthority();
    CopiesDoNotExtendSkipAuthorityOrBorrowCallerStrings();
    ProviderIdentityIsNotTheConsumerIdentity();
    InvalidAndUnboundContextsRefuseInsteadOfInventingMetadata();
  });
}
