#include "meta/Ids.h"
#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/DataSourceContext.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/TestHandlerContext.h"

#include "TestContextScope.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace agiru::detail {

struct TestContextState {
  CodeunitId codeunit;
  std::string qualifiedName;
  std::string procedure;
  std::string caseName;
  bool success;
  bool canSkip;
  bool active = true;
  bool skipped = false;
  std::string reason;
};

namespace {

bool isAfter(TestContextPhase phase) {
  switch (phase) {
    case TestContextPhase::BeforeCodeunit:
    case TestContextPhase::BeforeProcedure:
    case TestContextPhase::BeforeCase: return false;
    case TestContextPhase::AfterCodeunit:
    case TestContextPhase::AfterProcedure:
    case TestContextPhase::AfterCase: return true;
    case TestContextPhase::Invalid: break;
  }
  throw Error("Invalid TestHandlerContext phase");
}

}

TestContextScope::TestContextScope(TestContextIdentity identity,
                                   TestContextPhase phase,
                                   bool success) {
  const bool after = isAfter(phase);
  const bool codeunitPhase =
      phase == TestContextPhase::BeforeCodeunit || phase == TestContextPhase::AfterCodeunit;
  const bool casePhase =
      phase == TestContextPhase::BeforeCase || phase == TestContextPhase::AfterCase;
  if (identity.codeunit.Value() <= 0 || identity.qualifiedName.empty() ||
      codeunitPhase != identity.procedure.empty() || casePhase != !identity.caseName.empty()) {
    throw Error("Inconsistent TestHandlerContext scope identity");
  }
  state_ = std::make_shared<TestContextState>(
      TestContextState{.codeunit = identity.codeunit,
                       .qualifiedName = std::string(identity.qualifiedName),
                       .procedure = std::string(identity.procedure),
                       .caseName = std::string(identity.caseName),
                       .success = after && success,
                       .canSkip = phase == TestContextPhase::BeforeProcedure ||
                                  phase == TestContextPhase::BeforeCase,
                       .reason = {}});
}

TestContextScope::~TestContextScope() {
  finish();
}

void TestContextScope::finish() const noexcept {
  state_->active = false;
}

TestHandlerContext TestContextScope::Context() const {
  return TestHandlerContext(state_);
}

bool TestContextScope::Skipped() const {
  return state_->skipped;
}

std::string_view TestContextScope::SkipReason() const {
  return state_->reason;
}

DataSourceContext makeDataSourceContext(Guid app, CodeunitId provider) {
  if (provider.Value() <= 0) {
    throw Error("DataSourceContext requires a positive provider codeunit ID");
  }
  return {app, provider};
}

}

namespace agiru {

Guid DataSourceContext::AppId() const {
  if (provider_.Value() <= 0) { throw Error("DataSourceContext has no provider context"); }
  return app_;
}

Integer DataSourceContext::CodeunitId() const {
  if (provider_.Value() <= 0) { throw Error("DataSourceContext has no provider context"); }
  return provider_.Value();
}

TestHandlerContext::TestHandlerContext(std::shared_ptr<detail::TestContextState> state)
    : state_(std::move(state)) {}

detail::TestContextState &TestHandlerContext::state() const {
  if (!state_) { throw Error("TestHandlerContext has no hook context"); }
  return *state_;
}

Integer TestHandlerContext::CodeunitId() const {
  return state().codeunit.Value();
}

Text<0> TestHandlerContext::CodeunitName() const {
  return Text<0>{state().qualifiedName};
}

Text<0> TestHandlerContext::ProcedureName() const {
  return Text<0>{state().procedure};
}

Text<0> TestHandlerContext::TestCaseName() const {
  return Text<0>{state().caseName};
}

Boolean TestHandlerContext::Success() const {
  return state().success;
}

void TestHandlerContext::Skip(std::string_view reason) const {
  auto &control = state();
  if (!control.active || !control.canSkip) { return; }
  control.reason = reason;
  control.skipped = true;
}

}
