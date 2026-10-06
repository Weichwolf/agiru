#pragma once

#include "meta/Ids.h"
#include "type/TestHandlerContext.h"

#include <memory>
#include <string_view>

namespace agiru::detail {

enum class TestContextPhase {
  BeforeCodeunit,
  AfterCodeunit,
  BeforeProcedure,
  AfterProcedure,
  BeforeCase,
  AfterCase,
  Invalid
};

struct TestContextIdentity {
  CodeunitId codeunit;
  std::string_view qualifiedName;
  std::string_view procedure{};
  std::string_view caseName{};
};

class TestContextScope {
public:
  TestContextScope(TestContextIdentity identity, TestContextPhase phase, bool success = false);
  ~TestContextScope();
  TestContextScope(const TestContextScope &) = delete;
  TestContextScope &operator=(const TestContextScope &) = delete;
  TestContextScope(TestContextScope &&) = delete;
  TestContextScope &operator=(TestContextScope &&) = delete;

  [[nodiscard]] TestHandlerContext Context() const;
  [[nodiscard]] bool Skipped() const;
  [[nodiscard]] std::string_view SkipReason() const;
  void finish() const noexcept;

private:
  std::shared_ptr<TestContextState> state_;
};

}
