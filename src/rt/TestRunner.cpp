#include "runtime/TestRunner.h"

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"

#include <algorithm>
#include <mutex>
#include <span>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

std::vector<const TestCatalogue *> &Registered() {
  static std::vector<const TestCatalogue *> registered;
  return registered;
}

std::string Missed(const std::vector<std::string_view> &names) {
  std::string out;
  for (const std::string_view name : names) {
    if (!out.empty()) { out += ", "; }
    out += name;
  }
  return out;
}

TestResult RunOne(const TestCatalogue &codeunit, const TestMethod &method) {
  detail::Scope scope;
  HandlerTable::Install(codeunit.Handlers(), method.handlers);
  try {
    method.invoke();
  } catch (const Error &e) {
    static_cast<void>(HandlerTable::Uninstall());
    scope.Discard(e.what());
    return TestResult{
        .codeunit = codeunit.Name(), .method = method.name, .passed = false, .error = e.what()};
  }
  const std::vector<std::string_view> missed = HandlerTable::Uninstall();
  if (method.model.has_value() && *method.model == TransactionModel::AutoRollback) {
    scope.Discard("");
  } else {
    scope.Keep();
  }
  if (!missed.empty()) {
    return TestResult{.codeunit = codeunit.Name(),
                      .method = method.name,
                      .passed = false,
                      .error = "The handler(s) " + Missed(missed) +
                               " were named and never ran (board:0054)"};
  }
  return TestResult{
      .codeunit = codeunit.Name(), .method = method.name, .passed = true, .error = {}};
}

}

TestCatalogue::TestCatalogue(CodeunitId id,
                             std::string_view name,
                             void (*onRun)(),
                             std::span<const TestMethod> methods,
                             std::span<const TestHandler> handlers)
    : id_(id), name_(name), onRun_(onRun), methods_(methods), handlers_(handlers) {
  Registered().push_back(this);
}

std::vector<const TestCatalogue *> RegisteredTestCodeunits() {
  static std::once_flag once;
  std::call_once(once, [] {
    std::ranges::sort(Registered(), [](const TestCatalogue *a, const TestCatalogue *b) {
      return a->Id().Value() < b->Id().Value();
    });
  });
  return Registered();
}

TestRun RunRegisteredTests(std::string_view codeunit) {
  return RunRegisteredTests(codeunit, nullptr);
}

TestRun RunRegisteredTests(std::string_view codeunit, TestReport report) {
  TestRun run;
  for (const TestCatalogue *catalogue : RegisteredTestCodeunits()) {
    if (!codeunit.empty() && catalogue->Name() != codeunit) { continue; }
    detail::Scope isolation;
    if (catalogue->OnRun() != nullptr) {
      try {
        catalogue->OnRun()();
      } catch (const Error &e) {
        isolation.Discard(e.what());
        run.results.push_back(TestResult{
            .codeunit = catalogue->Name(), .method = "OnRun", .passed = false, .error = e.what()});
        ++run.failed;
        if (report != nullptr) { report(run.results.back()); }
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
      if (report != nullptr) { report(run.results.back()); }
    }
    isolation.Discard("");
  }
  return run;
}

}
