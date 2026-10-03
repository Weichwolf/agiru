#include "meta/Ids.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"

#include "Check.h"

#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
  return gate::Run("Generated RequiredTestIsolation", [argc, argv] {
    if (argc != 2) { throw std::runtime_error("expected the dedicated gate database DSN"); }
    const agiru::Session session(argv[1]);
    const auto catalogues = agiru::RegisteredTestCodeunits();
    CHECK_TRUE("one generated catalogue was linked", catalogues.size() == 1);
    if (catalogues.size() != 1) { return; }
    const auto &catalogue = *catalogues.front();
    CHECK_TRUE("the AL ID survives", catalogue.Id() == agiru::CodeunitId{960200});
    CHECK_TEXT("the AL name survives", catalogue.Name(), "Generated Isolation UT");
    CHECK_TEXT("the AL requirement reaches runtime metadata",
               catalogue.Def().requiredTestIsolation,
               "Disabled");
    const auto refused =
        agiru::RunRegisteredTests(catalogue.Name(), agiru::TestIsolation::Codeunit);
    CHECK_TRUE("a mismatched runner refuses both source methods",
               refused.passed == 0 && refused.failed == 2);
    CHECK_TRUE("refused results retain the complete method population",
               refused.results.size() == 2);
    for (const auto &result : refused.results) {
      CHECK_TRUE("the refusal identifies the required policy",
                 result.error.find("RequiredTestIsolation Disabled") != std::string::npos);
    }
    const auto accepted =
        agiru::RunRegisteredTests(catalogue.Name(), agiru::TestIsolation::Disabled);
    CHECK_TRUE("the matching runner executes both methods",
               accepted.passed == 2 && accepted.failed == 0);
  });
}
