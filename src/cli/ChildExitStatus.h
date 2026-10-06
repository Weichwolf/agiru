#pragma once

namespace agiru::cli {

inline constexpr int kLinuxWaitExitCodeUnit = 1 << 8;

[[nodiscard]] constexpr bool MatchesExpectedTestExit(int status, bool allPassed) {
  return status == (allPassed ? 0 : kLinuxWaitExitCodeUnit);
}

}
