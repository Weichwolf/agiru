#include "meta/Ids.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"
#include "type/JsonObject.h"
#include "type/Text.h"

#include "Check.h"

#include <array>
#include <string>
#include <vector>

namespace {

struct Fixture {};

void First([[maybe_unused]] void *instance) {}

void Second([[maybe_unused]] void *instance) {
  throw agiru::Error("fixture failure");
}

// [SET] A gate-only codeunit ID; it is not part of the AL population.
constexpr agiru::CodeunitId kFixtureId{950001};
constexpr std::array kMethods{
    agiru::TestMethod{.name = "First", .invoke = First, .model = {}, .handlers = {}},
    agiru::TestMethod{.name = "Second", .invoke = Second, .model = {}, .handlers = {}}};
const agiru::TestCatalogue kCatalogue{kFixtureId,
                                      "Report Fixture",
                                      agiru::MakeTestCodeunit<Fixture>,
                                      agiru::FreeTestCodeunit<Fixture>,
                                      nullptr,
                                      kMethods};

void ReportsBelongToTheirInvocation() {
  std::vector<agiru::TestResult> first;
  const auto collect = [](void *context, const agiru::TestResult &result) {
    static_cast<std::vector<agiru::TestResult> *>(context)->push_back(result);
  };
  const auto run = agiru::RunRegisteredTests(kCatalogue.Name(), &first, collect);
  CHECK_TRUE("the success and failure are counted", run.passed == 1 && run.failed == 1);
  CHECK_TRUE("every completed method has an identity", first.size() == kMethods.size());
  if (first.size() != kMethods.size()) { return; }
  CHECK_TEXT("declaration order starts with First", first.front().method, "First");
  CHECK_TRUE("the passing method is reported", first.front().passed);
  CHECK_TEXT("the failure keeps its diagnostic", first.back().error, "fixture failure");
  std::vector<agiru::TestResult> second;
  static_cast<void>(agiru::RunRegisteredTests(kCatalogue.Name(), &second, collect));
  CHECK_TRUE("the next invocation uses its own context", second.size() == kMethods.size());
  CHECK_TRUE("the previous context was not retained", first.size() == kMethods.size());
}

void IdentitiesAndDiagnosticsRoundTripAsOneJsonLine() {
  const agiru::TestResult original{.codeunit = "Quoted \" UT",
                                   .method = "Line\nBreak\t\\",
                                   .passed = false,
                                   .error = "first\r\nsecond"};
  const std::string encoded = agiru::TestResultJson(kFixtureId, original);
  CHECK_TRUE("embedded newlines cannot create extra result records",
             encoded.find('\n') == std::string::npos);
  agiru::JsonObject decoded;
  CHECK_TRUE("the emitted result is JSON", decoded.ReadFrom(encoded));
  CHECK_TRUE("the codeunit ID survives", decoded.GetInteger("codeunit_id") == kFixtureId.Value());
  CHECK_TEXT("the codeunit name is preserved",
             std::string_view(decoded.GetText("codeunit")),
             original.codeunit);
  CHECK_TEXT("the procedure name is preserved",
             std::string_view(decoded.GetText("method")),
             original.method);
  CHECK_TEXT(
      "the error text is preserved", std::string_view(decoded.GetText("error")), original.error);
  CHECK_TRUE("a failing result cannot become success", !decoded.GetBoolean("passed"));
}

void OutputFailureAbortsTheRun() {
  bool refused = false;
  try {
    static_cast<void>(agiru::RunRegisteredTests(
        kCatalogue.Name(),
        nullptr,
        []([[maybe_unused]] void *context, [[maybe_unused]] const agiru::TestResult &result) {
          throw agiru::Error("output unavailable");
        }));
  } catch (const agiru::Error &error) {
    refused = std::string(error.what()) == "output unavailable";
  }
  CHECK_TRUE("reporting failures propagate to the caller", refused);
}

}

int main() {
  return gate::Run("TestReport", [] {
    agiru::Session session(AGIRU_TEST_DSN);
    ReportsBelongToTheirInvocation();
    OutputFailureAbortsTheRun();
    IdentitiesAndDiagnosticsRoundTripAsOneJsonLine();
  });
}
