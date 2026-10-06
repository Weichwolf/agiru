#include "../../src/cli/ChildExitStatus.h"
#include "Check.h"

#include <array>
#include <csignal>
#include <string>
#include <string_view>

#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace {

constexpr int kLinuxExitCodeBits = 8;
constexpr int kExitCodeCount = 1 << kLinuxExitCodeBits;
constexpr int kCoreDumpFlag = 0x80;
constexpr int kStoppedFlag = 0x7f;
constexpr int kContinuedStatus = 0xffff;

void NormalExitMustAgreeWithTheReportedTestCounts() {
  for (int code = 0; code < kExitCodeCount; ++code) {
    const int status = code << kLinuxExitCodeBits;
    CHECK_TRUE("only exit zero completes an all-green result",
               agiru::cli::MatchesExpectedTestExit(status, true) == (code == 0));
    CHECK_TRUE("only exit one completes a result with failed tests",
               agiru::cli::MatchesExpectedTestExit(status, false) == (code == 1));
  }
}

void AbnormalStatusesNeverCompleteAReportedResult() {
  constexpr std::array statuses{-1,
                                SIGTERM,
                                SIGSEGV,
                                SIGABRT | kCoreDumpFlag,
                                (SIGSTOP << kLinuxExitCodeBits) | kStoppedFlag,
                                kContinuedStatus};
  for (const int status : statuses) {
    CHECK_TRUE("a signal, stopped/continued child or wait error cannot report all-green",
               !agiru::cli::MatchesExpectedTestExit(status, true));
    CHECK_TRUE("an abnormal child cannot masquerade as completed failed tests",
               !agiru::cli::MatchesExpectedTestExit(status, false));
  }
}

int ChildStatus(std::string_view command) {
  std::string shell = "/bin/sh";
  std::string option = "-c";
  std::string expression(command);
  std::array arguments{
      shell.data(), option.data(), expression.data(), static_cast<char *>(nullptr)};
  std::array<char *, 1> environment{nullptr};
  pid_t pid = 0;
  const int spawned =
      posix_spawn(&pid, shell.c_str(), nullptr, nullptr, arguments.data(), environment.data());
  CHECK_TRUE("the independent child starts", spawned == 0);
  if (spawned != 0) { return -1; }
  int status = -1;
  CHECK_TRUE("the independent child is reaped", waitpid(pid, &status, 0) == pid);
  return status;
}

void RealChildrenProvideTheLinuxWaitEncodingIndependently() {
  const int success = ChildStatus("exit 0");
  const int failure = ChildStatus("exit 1");
  const int unexpected = ChildStatus("exit 2");
  const int killed = ChildStatus("kill -TERM $$");
  CHECK_TRUE("the killed child actually terminates through SIGTERM", killed == SIGTERM);
  CHECK_TRUE("a real successful child agrees with all-green counts",
             agiru::cli::MatchesExpectedTestExit(success, true));
  CHECK_TRUE("a real successful child contradicts failed-test counts",
             !agiru::cli::MatchesExpectedTestExit(success, false));
  CHECK_TRUE("a real exit-one child agrees with failed-test counts",
             agiru::cli::MatchesExpectedTestExit(failure, false));
  CHECK_TRUE("a real exit-one child contradicts all-green counts",
             !agiru::cli::MatchesExpectedTestExit(failure, true));
  for (const int status : std::array{unexpected, killed}) {
    CHECK_TRUE("an unexpected exit or killed child cannot report all-green",
               !agiru::cli::MatchesExpectedTestExit(status, true));
    CHECK_TRUE("an unexpected exit or killed child cannot complete failed-test counts",
               !agiru::cli::MatchesExpectedTestExit(status, false));
  }
}

}

int main() {
  return gate::Run("ChildExitStatus", [] {
    NormalExitMustAgreeWithTheReportedTestCounts();
    AbnormalStatusesNeverCompleteAReportedResult();
    RealChildrenProvideTheLinuxWaitEncodingIndependently();
  });
}
