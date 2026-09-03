#include "runtime/TestRunner.h"

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"

#include <algorithm>
#include <span>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

std::vector<const TestCatalogue *> &Registered() {
  static std::vector<const TestCatalogue *> registered;
  return registered;
}

TestResult RunOne(const TestCatalogue &codeunit, const TestMethod &method) {
  detail::Scope scope;
  try {
    method.invoke();
  } catch (const Error &e) {
    scope.Discard(e.what());
    return TestResult{
        .codeunit = codeunit.Name(), .method = method.name, .passed = false, .error = e.what()};
  }
  scope.Discard("");
  return TestResult{
      .codeunit = codeunit.Name(), .method = method.name, .passed = true, .error = {}};
}

} // namespace

TestCatalogue::TestCatalogue(CodeunitId id,
                             std::string_view name,
                             void (*onRun)(),
                             std::span<const TestMethod> methods)
    : id_(id), name_(name), onRun_(onRun), methods_(methods) {
  Registered().push_back(this);
}

std::vector<const TestCatalogue *> RegisteredTestCodeunits() {
  std::vector<const TestCatalogue *> all = Registered();
  std::ranges::sort(all, [](const TestCatalogue *a, const TestCatalogue *b) {
    return a->Id().Value() < b->Id().Value();
  });
  return all;
}

TestRun RunRegisteredTests(std::string_view codeunit) {
  TestRun run;
  for (const TestCatalogue *catalogue : RegisteredTestCodeunits()) {
    if (!codeunit.empty() && catalogue->Name() != codeunit) { continue; }
    if (catalogue->OnRun() != nullptr) {
      detail::Scope scope;
      try {
        catalogue->OnRun()();
        scope.Discard("");
      } catch (const Error &e) {
        scope.Discard(e.what());
        run.results.push_back(TestResult{
            .codeunit = catalogue->Name(), .method = "OnRun", .passed = false, .error = e.what()});
        ++run.failed;
        continue;
      }
    }
    for (const TestMethod &method : catalogue->Methods()) {
      run.results.push_back(RunOne(*catalogue, method));
      if (run.results.back().passed) {
        ++run.passed;
      } else {
        ++run.failed;
      }
    }
  }
  return run;
}

} // namespace agiru
