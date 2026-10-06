#include "runtime/ProcessDiagnostics.h"

#include "Check.h"

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace {

void CheckStartup(std::string_view mode) {
  const bool tracing = mode == "empty" || mode == "verbose";
  CHECK_TRUE("SQL tracing preserves startup presence",
             agiru::detail::TraceSql() == (mode != "unset"));
  CHECK_TRUE("result tracing requires the exact level",
             agiru::detail::TraceSqlRows() == (mode == "verbose"));
  CHECK_TRUE("error tracing preserves startup presence", agiru::detail::TraceErrors() == tracing);
  CHECK_TRUE("UI tracing preserves startup presence", agiru::detail::TraceUi() == tracing);
  const char *procedures = agiru::detail::SelectedTestProcedures();
  if (mode == "unset" || mode == "ordinary") {
    CHECK_TRUE("unset procedure selection remains null", procedures == nullptr);
  } else {
    CHECK_TRUE("set procedure selection remains present", procedures != nullptr);
    if (procedures != nullptr) {
      CHECK_TEXT("procedure selection preserves exact text",
                 procedures,
                 mode == "empty" ? "" : "First,Änderung");
    }
  }
}

void Child(char *executable, std::string mode, std::span<std::string> environment) {
  std::vector<char *> variables;
  for (auto &value : environment) { variables.push_back(value.data()); }
  variables.push_back(nullptr);
  std::array arguments{executable, mode.data(), static_cast<char *>(nullptr)};
  pid_t pid = 0;
  const int spawned =
      posix_spawn(&pid, executable, nullptr, nullptr, arguments.data(), variables.data());
  CHECK_TRUE("diagnostics child starts", spawned == 0);
  if (spawned != 0) { return; }
  int status = 0;
  CHECK_TRUE("diagnostics child is reaped", waitpid(pid, &status, 0) == pid);
  CHECK_TRUE("startup semantics hold in the child", status == 0);
}

void StartupModes(char *executable) {
  std::array<std::string, 0> unset;
  Child(executable, "unset", unset);
  std::array<std::string, 4> empty{
      "AGIRU_TRACE_SQL=", "AGIRU_TRACE_ERRORS=", "AGIRU_TRACE_UI=", "AGIRU_TEST_PROCEDURE="};
  Child(executable, "empty", empty);
  std::array<std::string, 4> verbose{"AGIRU_TRACE_SQL=2",
                                     "AGIRU_TRACE_ERRORS=0",
                                     "AGIRU_TRACE_UI=no",
                                     "AGIRU_TEST_PROCEDURE=First,Änderung"};
  Child(executable, "verbose", verbose);
  std::array<std::string, 1> ordinary{"AGIRU_TRACE_SQL=02"};
  Child(executable, "ordinary", ordinary);
}

}

int main(int argc, char **argv) {
  return gate::Run("Process diagnostics", [&] {
    if (argc == 2) {
      CheckStartup(argv[1]);
    } else {
      StartupModes(argv[0]);
    }
  });
}
