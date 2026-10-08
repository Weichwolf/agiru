#pragma once

#include "runtime/SessionOptions.h"

#include <chrono>
#include <cstddef>
#include <string>

/// \file
/// \brief Trusted page-host configuration without authorization or retained execution state.
namespace agiru {

/// \brief Explicit initial resource/deployment bounds, not production scale claims.
struct PageHostOptions {
  static constexpr std::size_t kDefaultContexts = 64;  ///< Initial private-window admission bound.
  static constexpr std::size_t kDefaultDepth = 16;     ///< Initial retained navigation-stack bound.
  static constexpr std::size_t kDefaultCommands = 128; ///< Initial per-window receipt census bound.
  static constexpr std::size_t kDefaultReceiptBytes = 4194304; ///< Four-MiB stored-response bound.
  static constexpr std::size_t kDefaultListRows =
      40; ///< Shared web/CMD/MCP window, per client contract.
  static constexpr std::size_t kDefaultExecutionWorkers =
      16; ///< Initial AL worker bound, not per session.
  static constexpr std::size_t kDefaultExecutionQueue = 32; ///< Initial queued AL command bound.
  static constexpr unsigned kDefaultResponseWaitMs = 50;    ///< Initial synchronous response probe.
  static constexpr unsigned kDefaultDialogTimeoutSeconds =
      300;              ///< Five-minute explicit-answer bound.
  std::string database; ///< One verified company's SQL connection string; never from a URL.
  std::string company;  ///< Exact configured company; other names refuse, never relabel SQL.
  std::string origin;   ///< Trusted externally visible origin for browser command CSRF checks.
  std::size_t contexts = kDefaultContexts;     ///< Maximum retained windows with private AL state.
  std::size_t navigationDepth = kDefaultDepth; ///< Maximum retained list/card navigation stack.
  std::size_t commands = kDefaultCommands;     ///< Maximum durable command receipts per window.
  std::size_t receiptBytes = kDefaultReceiptBytes; ///< Stored-response budget per window.
  std::size_t listRows = kDefaultListRows; ///< Trusted SQL block bound, never a URL/CLI parameter.
  std::size_t executionWorkers =
      kDefaultExecutionWorkers;                        ///< AL workers separate from HTTP workers.
  std::size_t executionQueue = kDefaultExecutionQueue; ///< Maximum queued AL calls before refusal.
  std::chrono::milliseconds responseWait{
      kDefaultResponseWaitMs};                           ///< Bounded initial response wait.
  std::chrono::seconds lifetime = std::chrono::hours(1); ///< Fixed bounded context lifetime.
  std::chrono::seconds dialogTimeout{
      kDefaultDialogTimeoutSeconds}; ///< Bounded explicit-answer wait.
  SessionOptions session{}; ///< Trusted immutable runtime policy for every retained context.
};

/// \brief Validates page/session configuration without connecting to SQL or running AL.
/// \param options Trusted company, origin and positive bounded host settings.
/// \throws Error with PageHostConfiguration for missing authority context or invalid limits.
void ValidatePageHostOptions(const PageHostOptions &options);

}
