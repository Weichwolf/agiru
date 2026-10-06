#include "runtime/ProcessDiagnostics.h"

#include <optional>
#include <string>
#include <string_view>

#include <unistd.h>

namespace agiru::detail {

namespace {

std::optional<std::string> StartupValue(std::string_view name) {
  if (environ == nullptr) { return std::nullopt; }
  for (char **entry = environ; *entry != nullptr; ++entry) {
    const std::string_view value(*entry);
    if (value.size() > name.size() && value.starts_with(name) && value[name.size()] == '=') {
      return std::string(value.substr(name.size() + 1));
    }
  }
  return std::nullopt;
}

const auto kSql = StartupValue("AGIRU_TRACE_SQL");
const bool kErrors = StartupValue("AGIRU_TRACE_ERRORS").has_value();
const bool kUi = StartupValue("AGIRU_TRACE_UI").has_value();
const auto kProcedures = StartupValue("AGIRU_TEST_PROCEDURE");

}

bool TraceSql() noexcept {
  return kSql.has_value();
}

bool TraceSqlRows() noexcept {
  return kSql == "2";
}

bool TraceErrors() noexcept {
  return kErrors;
}

bool TraceUi() noexcept {
  return kUi;
}

const char *SelectedTestProcedures() noexcept {
  return kProcedures.has_value() ? kProcedures->c_str() : nullptr;
}

}
