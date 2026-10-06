#include "runtime/test/Handlers.h"

#include "Check.h"
#include "FailingAllocation.h"

#include <array>
#include <new>
#include <string_view>
#include <utility>

using gate::failAllocation;

namespace {

void Answer([[maybe_unused]] std::string_view message, [[maybe_unused]] void *response) {}

constexpr std::array<agiru::TestHandler, 1> kOriginal{{{.name = "Original",
                                                        .kind = agiru::HandlerKind::Message,
                                                        .object = 0,
                                                        .invoke = &Answer,
                                                        .optional = false}}};
constexpr std::array<agiru::TestHandler, 1> kReplacement{{{.name = "Replacement",
                                                           .kind = agiru::HandlerKind::Message,
                                                           .object = 0,
                                                           .invoke = &Answer,
                                                           .optional = false}}};
constexpr std::array<std::string_view, 1> kOriginalNames{"Original"};
constexpr std::array<std::string_view, 2> kReplacementNames{"Replacement", "Missing"};
}

namespace {
void FailedReportingStillUninstalls() {
  agiru::HandlerTable::Install(kOriginal, kOriginalNames);
  bool refused = false;
  failAllocation = true;
  try {
    static_cast<void>(agiru::HandlerTable::Uninstall());
  } catch (const std::bad_alloc &) { refused = true; }
  failAllocation = false;
  CHECK_TRUE("missing-handler reporting encountered the injected failure", refused);
  CHECK_TRUE("failed reporting still detaches the handler table",
             !agiru::HandlerTable::Installed());
  CHECK_TRUE("no callback survives failed reporting",
             agiru::HandlerTable::For(agiru::HandlerKind::Message) == nullptr);
  static_cast<void>(agiru::HandlerTable::Uninstall());
}

void FailedInstallationKeepsThePreviousTableCoherent() {
  agiru::HandlerTable::Install(kOriginal, kOriginalNames);
  bool refused = false;
  failAllocation = true;
  try {
    agiru::HandlerTable::Install(kReplacement, kReplacementNames);
  } catch (const std::bad_alloc &) { refused = true; }
  failAllocation = false;
  CHECK_TRUE("installation encountered the injected failure", refused);
  CHECK_TRUE("failed installation keeps the previous complete table",
             agiru::HandlerTable::For(agiru::HandlerKind::Message) == &kOriginal.front());
  static_cast<void>(agiru::HandlerTable::Uninstall());
}

void AbortedMethodCleanupDoesNotAllocate() {
  agiru::HandlerTable::Install(kOriginal, kOriginalNames);
  failAllocation = true;
  agiru::HandlerTable::Reset();
  const bool untouched = std::exchange(failAllocation, false);
  CHECK_TRUE("aborted-method cleanup does not allocate", untouched);
  CHECK_TRUE("aborted-method cleanup detaches handlers", !agiru::HandlerTable::Installed());
}
}

int main() {
  return gate::Run("HandlerLifetime", [] {
    FailedReportingStillUninstalls();
    FailedInstallationKeepsThePreviousTableCoherent();
    AbortedMethodCleanupDoesNotAllocate();
  });
}
