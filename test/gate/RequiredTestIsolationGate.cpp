#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "meta/Subtype.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"

#include "Check.h"
#include "CodeunitWriter.h"
#include "Parser.h"

#include <array>
#include <bit>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

using agiru::TestIsolation;

struct Calls {
  int made = 0;
  int freed = 0;
  int setup = 0;
  int methods = 0;
};

Calls calls;

const agiru::Connection &Db() {
  return agiru::Session::Current().Database();
}

void Write() {
  Db().Run("INSERT INTO required_isolation_gate DEFAULT VALUES");
}

void *Make() {
  ++calls.made;
  Write();
  return &calls;
}

void Free([[maybe_unused]] void *instance) {
  ++calls.freed;
}

void Setup([[maybe_unused]] void *instance) {
  ++calls.setup;
  Write();
}

void Method([[maybe_unused]] void *instance) {
  ++calls.methods;
  Write();
}

constexpr std::array kMethods{
    agiru::TestMethod{.name = "First", .invoke = Method, .model = {}, .handlers = {}},
    agiru::TestMethod{.name = "Second", .invoke = Method, .model = {}, .handlers = {}}};

// Gate-only identities; requirements and runner policies are independent inputs.
constexpr std::array kDefinitions{agiru::CodeunitDef{.id = agiru::CodeunitId{960100},
                                                     .name = "Omitted",
                                                     .subtype = agiru::Subtype::Test},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960101},
                                                     .name = "None",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = "None"},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960102},
                                                     .name = "Disabled",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = "Disabled"},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960103},
                                                     .name = "Codeunit",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = "Codeunit"},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960104},
                                                     .name = "Function",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = "Function"},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960105},
                                                     .name = "MixedCase",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = "dIsAbLeD"},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960106},
                                                     .name = "Invalid",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = "Unknown"},
                                  agiru::CodeunitDef{.id = agiru::CodeunitId{960107},
                                                     .name = "Whitespace",
                                                     .subtype = agiru::Subtype::Test,
                                                     .requiredTestIsolation = " Disabled"}};

const std::array kCatalogues{agiru::TestCatalogue{kDefinitions[0], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[1], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[2], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[3], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[4], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[5], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[6], Make, Free, Setup, kMethods},
                             agiru::TestCatalogue{kDefinitions[7], Make, Free, Setup, kMethods}};

void Collect(void *context, const agiru::TestResult &result) {
  static_cast<std::vector<agiru::TestResult> *>(context)->push_back(result);
}

void CheckRun(std::size_t index, TestIsolation policy, bool allowed) {
  Db().Run("TRUNCATE required_isolation_gate");
  agiru::Commit();
  calls = {};
  const auto &catalogue = kCatalogues[index];
  std::vector<agiru::TestResult> reported;
  const auto run = agiru::RunRegisteredTests(catalogue.Name(), &reported, Collect, policy);
  const std::string claim =
      std::string(catalogue.Name()) + "/" + std::to_string(static_cast<int>(policy));
  CHECK_TRUE(claim + " retains authoritative metadata", &catalogue.Def() == &kDefinitions[index]);
  CHECK_TRUE(claim + " retains ID", catalogue.Id() == kDefinitions[index].id);
  CHECK_TRUE(claim + " counts every method", run.results.size() == kMethods.size());
  CHECK_TRUE(claim + " reports every method", reported.size() == kMethods.size());
  CHECK_TRUE(claim + " has the required outcome",
             run.passed == (allowed ? 2U : 0U) && run.failed == (allowed ? 0U : 2U));
  CHECK_TRUE(claim + " guards construction and cleanup",
             calls.made == (allowed ? 1 : 0) && calls.freed == calls.made);
  CHECK_TRUE(claim + " guards OnRun", calls.setup == (allowed ? 1 : 0));
  CHECK_TRUE(claim + " guards method execution", calls.methods == (allowed ? 2 : 0));
  CHECK_TRUE(claim + " releases current instance", agiru::CurrentTestInstance() == nullptr);
  for (std::size_t at = 0; at < reported.size(); ++at) {
    CHECK_TEXT(claim + " keeps source name", reported[at].codeunit, catalogue.Name());
    CHECK_TEXT(
        claim + " keeps method declaration order", reported[at].method, kMethods.at(at).name);
    CHECK_TRUE(claim + " retains diagnostics", reported[at].error.empty() == allowed);
    if (!allowed) {
      CHECK_TRUE(claim + " identifies the isolation contract",
                 reported[at].error.find("TestIsolation") != std::string::npos);
    }
  }
  if (!allowed) {
    const agiru::Connection observer(AGIRU_TEST_DSN);
    const auto rows = observer.Execute("SELECT count(*) FROM required_isolation_gate");
    CHECK_TEXT(
        claim + " produces no committed database effects", rows.Value(0, 0).value_or(""), "0");
  }
}

void RequirementsMatchExactly() {
  constexpr std::array policies{
      TestIsolation::Disabled, TestIsolation::Codeunit, TestIsolation::Function};
  Db().Run("DROP TABLE IF EXISTS required_isolation_gate");
  Db().Run("CREATE TABLE required_isolation_gate (id bigserial PRIMARY KEY)");
  agiru::Commit();
  for (std::size_t index = 0; index < kDefinitions.size(); ++index) {
    for (std::size_t policy = 0; policy < policies.size(); ++policy) {
      const bool allowed = index < 2 || (index >= 2 && index <= 4 && index - 2 == policy) ||
                           (index == 5 && policy == 0);
      CheckRun(index, policies[policy], allowed);
    }
  }
  CheckRun(0, std::bit_cast<TestIsolation>(std::underlying_type_t<TestIsolation>{-1}), false);
  Db().Run("DROP TABLE required_isolation_gate");
}

std::string Source(std::string_view requirement) {
  return "codeunit 960200 \"Generated Isolation UT\" { Subtype = Test; RequiredTestIsolation = " +
         std::string(requirement) +
         "; trigger OnRun() begin end; [Test] procedure First() begin end; "
         "[Test] procedure Second() begin end; }";
}

void GeneratedCatalogueBorrowsSourceMetadata() {
  constexpr std::array requirements{"None", "Disabled", "Codeunit", "Function", "dIsAbLeD"};
  for (const std::string_view requirement : requirements) {
    const auto unit = agiru::al::ParseCodeunit(Source(requirement));
    const auto generated = agiru::gen::WriteCodeunitSource(unit, "Fixture.Codeunit.al", {});
    CHECK_TRUE("the generated catalogue borrows the declaration",
               generated.find(
                   "kTestCatalogue{CodeunitTraits<GeneratedIsolationUT_Codeunit>::kCodeunit,") !=
                   std::string::npos);
    CHECK_TRUE("the original requirement reaches the declaration",
               generated.find(".requiredTestIsolation = \"" + std::string(requirement) + "\"") !=
                   std::string::npos);
  }
}

void WriteFile(const std::filesystem::path &path, const std::string &source) {
  std::ofstream file(path);
  file.exceptions(std::ios::badbit | std::ios::failbit);
  file << source;
}

void Emit(const std::filesystem::path &root) {
  const auto unit = agiru::al::ParseCodeunit(Source("Disabled"));
  std::filesystem::create_directories(root);
  WriteFile(root / "GeneratedIsolationUT.h",
            agiru::gen::WriteCodeunit(unit, "Fixture.Codeunit.al", {}).text);
  WriteFile(root / "GeneratedIsolationUT.cpp",
            agiru::gen::WriteCodeunitSource(unit, "Fixture.Codeunit.al", {}));
}

}

int main(int argc, char **argv) {
  return gate::Run("RequiredTestIsolation", [argc, argv] {
    if (argc == 2) {
      Emit(argv[1]);
      return;
    }
    if (argc != 1) { throw std::runtime_error("expected at most one output directory"); }
    const agiru::Session session(AGIRU_TEST_DSN);
    RequirementsMatchExactly();
    GeneratedCatalogueBorrowsSourceMetadata();
  });
}
