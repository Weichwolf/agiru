#include "runtime/test/Handlers.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <string_view>
#include <utility>

namespace {
thread_local bool failAllocation = false;

void Answer(std::string_view, void *) {}

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

// Allocation failure is confined to this gate process.
void *operator new(std::size_t size) {
  if (std::exchange(failAllocation, false)) { throw std::bad_alloc(); }
  if (void *memory = std::malloc(size == 0 ? 1 : size)) { return memory; }
  throw std::bad_alloc();
}

void operator delete(void *memory) noexcept {
  std::free(memory);
}

void operator delete(void *memory, std::size_t) noexcept {
  std::free(memory);
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
