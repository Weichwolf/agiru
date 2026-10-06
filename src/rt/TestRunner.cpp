#include "runtime/TestRunner.h"

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/ProcessDiagnostics.h"
#include "runtime/Transaction.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/PageTraps.h"
#include "type/Integer.h"
#include "type/JsonObject.h"
#include "type/StringValue.h"
#include "type/TransactionModel.h"

#include "BuiltinsWritten.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <exception>
#include <mutex>
#include <new>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <execinfo.h>
#include <unistd.h>

namespace agiru {

namespace {

void TracedAllocationFailure_() {
  std::println(stderr, "out of memory at:");
  constexpr std::size_t kAllocationTraceFrames = 64;
  std::array<void *, kAllocationTraceFrames> frames{};
  const int depth = backtrace(frames.data(), static_cast<int>(frames.size()));
  backtrace_symbols_fd(frames.data(), depth, STDERR_FILENO);
  std::set_new_handler(nullptr);
  throw std::bad_alloc();
}

void TraceAllocationFailures_() {
  static std::once_flag once;
  std::call_once(once, [] {
    if (detail::TraceErrors()) { std::set_new_handler(&TracedAllocationFailure_); }
  });
}

bool Named(std::string_view list, std::string_view name) {
  for (std::size_t at = 0; at <= list.size();) {
    const std::size_t comma = list.find(',', at);
    const std::string_view one =
        list.substr(at, comma == std::string_view::npos ? std::string_view::npos : comma - at);
    if (one == name) { return true; }
    if (comma == std::string_view::npos) { break; }
    at = comma + 1;
  }
  return false;
}

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

constexpr ::agiru::Integer kSeedBeforeEachTest = 1;

void *&CurrentInstance() {
  thread_local void *instance = nullptr;
  return instance;
}

struct Driven {
  explicit Driven(const TestCatalogue &of) : owner(of), instance(of.Make()) {
    SetCurrentTestInstance(instance);
  }

  Driven(const Driven &) = delete;
  Driven &operator=(const Driven &) = delete;

  ~Driven() {
    SetCurrentTestInstance(nullptr);
    owner.Free(instance);
  }

  const TestCatalogue &owner;
  void *instance;
};

struct MethodResources {
  MethodResources() { detail::ClearTraps(); }

  MethodResources(const MethodResources &) = delete;
  MethodResources &operator=(const MethodResources &) = delete;

  ~MethodResources() {
    HandlerTable::Reset();
    detail::ClearTraps();
  }
};

TestResult RunOne(const TestCatalogue &codeunit,
                  const TestMethod &method,
                  void *instance,
                  bool commitOnSuccess) {
  TraceAllocationFailures_();
  const MethodResources resources;
  ClearLastError();
  Randomize(kSeedBeforeEachTest);
  detail::Scope scope;
  const detail::AutoRollbackTest commitPolicy(method.model.has_value() &&
                                              *method.model == TransactionModel::AutoRollback);
  try {
    HandlerTable::Install(codeunit.Handlers(), method.handlers);
    method.invoke(instance);
  } catch (const Error &e) {
    static_cast<void>(HandlerTable::Uninstall());
    scope.Discard(e);
    return TestResult{
        .codeunit = codeunit.Name(), .method = method.name, .passed = false, .error = e.what()};
  } catch (const std::exception &e) {
    static_cast<void>(HandlerTable::Uninstall());
    scope.Discard(e.what());
    return TestResult{.codeunit = codeunit.Name(),
                      .method = method.name,
                      .passed = false,
                      .error = std::string("this case left a C++ exception rather than an AL "
                                           "error, which is a defect in the runtime and not in "
                                           "the test: ") +
                               e.what()};
  }
  const std::vector<std::string_view> missed = HandlerTable::Uninstall();
  if (!missed.empty()) {
    const std::string error =
        "The handler(s) " + Missed(missed) + " were named and never ran (board:0054)";
    scope.Discard(error);
    return TestResult{
        .codeunit = codeunit.Name(), .method = method.name, .passed = false, .error = error};
  }
  if (method.model.has_value() && *method.model == TransactionModel::AutoRollback) {
    scope.Discard("");
  } else {
    if (commitOnSuccess) {
      try {
        Commit();
      } catch (const Error &e) {
        scope.Discard(e);
        return TestResult{
            .codeunit = codeunit.Name(), .method = method.name, .passed = false, .error = e.what()};
      }
    }
    scope.Keep();
  }
  return TestResult{
      .codeunit = codeunit.Name(), .method = method.name, .passed = true, .error = {}};
}

TestResult RunSelectedMethod(const TestCatalogue &codeunit,
                             const TestMethod &method,
                             void *instance,
                             TestIsolation isolation) {
  if (isolation == TestIsolation::Function) {
    detail::Scope methodIsolation;
    const detail::IsolationFloor methodFloor(methodIsolation.Depth());
    TestResult result = RunOne(codeunit, method, instance, false);
    methodIsolation.Discard("");
    return result;
  }
  return RunOne(codeunit,
                method,
                instance,
                isolation == TestIsolation::Disabled && method.model != TransactionModel::None);
}

void AppendResult(TestRun &run, TestResult result, void *context, ContextTestReport report) {
  run.results.push_back(std::move(result));
  if (run.results.back().passed) {
    ++run.passed;
  } else {
    ++run.failed;
  }
  if (report != nullptr) { report(context, run.results.back()); }
}

std::string_view IsolationName(TestIsolation isolation) {
  switch (isolation) {
    case TestIsolation::Disabled: return "Disabled";
    case TestIsolation::Codeunit: return "Codeunit";
    case TestIsolation::Function: return "Function";
  }
  return {};
}

bool SameIsolationName(std::string_view left, std::string_view right) {
  return std::ranges::equal(left, right, [](unsigned char a, unsigned char b) {
    return std::tolower(a) == std::tolower(b);
  });
}

std::string IsolationError(const TestCatalogue &catalogue, TestIsolation isolation) {
  const std::string_view selected = IsolationName(isolation);
  if (selected.empty()) { return "Invalid TestIsolation runner policy"; }
  const std::string_view required = catalogue.Def().requiredTestIsolation;
  if (required.empty() || SameIsolationName(required, "None") ||
      SameIsolationName(required, selected)) {
    return {};
  }
  constexpr std::array known{"Disabled", "Codeunit", "Function"};
  if (!std::ranges::any_of(
          known, [required](std::string_view name) { return SameIsolationName(required, name); })) {
    return "Invalid RequiredTestIsolation: " + std::string(required);
  }
  return "RequiredTestIsolation " + std::string(required) + " is incompatible with TestIsolation " +
         std::string(selected);
}

bool AcceptIsolation(const TestCatalogue &catalogue,
                     TestIsolation isolationPolicy,
                     TestRun &run,
                     void *context,
                     ContextTestReport report) {
  const std::string refusal = IsolationError(catalogue, isolationPolicy);
  if (refusal.empty()) { return true; }
  for (const TestMethod &method : catalogue.Methods()) {
    AppendResult(
        run,
        TestResult{
            .codeunit = catalogue.Name(), .method = method.name, .passed = false, .error = refusal},
        context,
        report);
  }
  return false;
}

}

void *CurrentTestInstance() {
  return CurrentInstance();
}

void SetCurrentTestInstance(void *instance) {
  CurrentInstance() = instance;
}

TestCatalogue::TestCatalogue(const CodeunitDef &definition,
                             void *(*make)(),
                             void (*free)(void *),
                             void (*onRun)(void *),
                             std::span<const TestMethod> methods,
                             std::span<const TestHandler> handlers)
    : definition_(definition),
      make_(make),
      free_(free),
      onRun_(onRun),
      methods_(methods),
      handlers_(handlers) {
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

std::string TestResultJson(CodeunitId id, const TestResult &result) {
  constexpr Integer kResultSchema = 1;
  JsonObject row;
  row.Add("schema", kResultSchema);
  row.Add("codeunit_id", id.Value());
  row.Add("codeunit", result.codeunit);
  row.Add("method", result.method);
  row.Add("passed", result.passed);
  row.Add("error", std::string_view(result.error));
  Text<0> encoded;
  row.WriteTo(encoded);
  return std::string(std::string_view(encoded));
}

TestRun RunRegisteredTests(std::string_view codeunit) {
  return RunRegisteredTests(codeunit, nullptr);
}

TestRun RunRegisteredTests(std::string_view codeunit, TestIsolation isolation) {
  return RunRegisteredTests(codeunit, nullptr, nullptr, isolation);
}

TestRun RunRegisteredTests(std::string_view codeunit, TestReport report) {
  return RunRegisteredTests(
      codeunit, static_cast<void *>(&report), [](void *context, const TestResult &result) {
        const auto callback = *static_cast<TestReport *>(context);
        if (callback != nullptr) { callback(result); }
      });
}

TestRun RunRegisteredTests(std::string_view codeunit,
                           void *context,
                           ContextTestReport report,
                           TestIsolation isolationPolicy) {
  TestRun run;
  for (const TestCatalogue *catalogue : RegisteredTestCodeunits()) {
    if (!codeunit.empty() && catalogue->Name() != codeunit) { continue; }
    if (!AcceptIsolation(*catalogue, isolationPolicy, run, context, report)) { continue; }
    std::optional<detail::Scope> isolation;
    std::optional<detail::IsolationFloor> floor;
    if (isolationPolicy == TestIsolation::Codeunit) {
      isolation.emplace();
      floor.emplace(isolation->Depth());
    }
    const Driven driven(*catalogue);
    if (catalogue->OnRun() != nullptr) {
      try {
        catalogue->OnRun()(driven.instance);
      } catch (const std::exception &e) {
        if (isolation.has_value()) { isolation->Discard(e.what()); }
        AppendResult(run,
                     TestResult{.codeunit = catalogue->Name(),
                                .method = "OnRun",
                                .passed = false,
                                .error = e.what()},
                     context,
                     report);
        continue;
      }
    }
    const char *const only = detail::SelectedTestProcedures();
    for (const TestMethod &method : catalogue->Methods()) {
      if (only != nullptr && !Named(only, method.name)) { continue; }
      AppendResult(run,
                   RunSelectedMethod(*catalogue, method, driven.instance, isolationPolicy),
                   context,
                   report);
    }
    if (isolation.has_value()) { isolation->Discard(""); }
  }
  return run;
}

}
