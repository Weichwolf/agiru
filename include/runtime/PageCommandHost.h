#pragma once

#include "runtime/SessionOptions.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

/// \file
/// \brief Authenticated native command host over generated pages, shared by web/CMD/MCP.

namespace agiru {

class Connection;
struct PageDef;
struct PageControlCommand;
struct ServerHttpRequest;
struct ServerHttpResponse;
class TablePermissionAuthority;

/// \brief Authorization vocabulary for lifecycle operations, separate from AL controls.
enum class PageHostOperation : std::uint8_t {
  OpenView,
  OpenEdit,
  OpenNew,
  Read,
  Move,
  Save,
  Close,
  Control
};

/// \brief Mandatory host authorization in the currently active User/company/SQL command.
/// The callback must enforce page/table/company permissions, including transitive AL access.
/// It runs before lifecycle triggers and every control operation; absence refuses startup.
/// This dependency is not a BC permission-set provider or an implicit allowlist.
using PageHostAuthorization =
    std::function<void(const PageDef &, PageHostOperation, const PageControlCommand &)>;

/// \brief Explicit initial resource/deployment bounds, not production scale claims.
struct PageHostOptions {
  static constexpr std::size_t kDefaultContexts = 64;  ///< Initial private-window admission bound.
  static constexpr std::size_t kDefaultDepth = 16;     ///< Initial retained navigation-stack bound.
  static constexpr std::size_t kDefaultCommands = 128; ///< Initial per-window receipt census bound.
  static constexpr std::size_t kDefaultReceiptBytes = 4194304; ///< Four-MiB stored-response bound.
  std::string database; ///< One verified company's SQL connection string; never from a URL.
  std::string company;  ///< Exact configured company; other names refuse, never relabel SQL.
  std::string origin;   ///< Trusted externally visible origin for browser command CSRF checks.
  std::size_t contexts = kDefaultContexts;     ///< Maximum retained windows with private AL state.
  std::size_t navigationDepth = kDefaultDepth; ///< Maximum retained list/card navigation stack.
  std::size_t commands = kDefaultCommands;     ///< Maximum durable command receipts per window.
  std::size_t receiptBytes = kDefaultReceiptBytes;       ///< Stored-response budget per window.
  std::chrono::seconds lifetime = std::chrono::hours(1); ///< Fixed bounded context lifetime.
  SessionOptions session{}; ///< Trusted immutable runtime policy for every retained context.
};

/// \brief Validates page/session configuration without connecting to SQL or running AL.
/// \param options Trusted company, origin and positive bounded host settings.
/// \throws Error with PageHostConfiguration for missing authority context or invalid limits.
void ValidatePageHostOptions(const PageHostOptions &options);

/// \brief Installs host-owned context/receipt storage using trusted migration authority.
/// \param connection Database containing the original User and agiru_client credential store.
/// \note Grants no users or ERP permissions; caller owns commit/rollback.
void InstallPageCommandHost(const Connection &connection);

/// \brief Shared generated-page execution with SQL ownership/revisions and durable receipts.
/// Request-local connections are released after each command. Private AL pages remain bounded
/// in this process; PostgreSQL owns user/context identity, expiry, fencing and command outcomes.
/// A host restart refuses old handles rather than fabricating recovered AL state. This initial
/// single-company adapter does not implement company schema routing, passwords, modal suspension,
/// a connection pool, full URL/bookmark/filter semantics or a BC permission-set provider.
class PageCommandHost {
public:
  /// \brief Starts a host; does not create schema or grant permissions.
  /// \param options Trusted deployment configuration.
  /// \param authorization Mandatory thread-safe runtime permission authority.
  /// \param tableAuthorization Mandatory authority for every transitive AL record access.
  /// \throws Error for missing/invalid configuration or missing authorization.
  PageCommandHost(PageHostOptions options,
                  PageHostAuthorization authorization,
                  std::shared_ptr<const TablePermissionAuthority> tableAuthorization);
  /// \brief Releases private pages without implicit saves/close-trigger execution.
  ~PageCommandHost();
  PageCommandHost(const PageCommandHost &) = delete;
  PageCommandHost &operator=(const PageCommandHost &) = delete;

  /// \brief Handles authenticated page GET and explicit application/x-www-form-urlencoded POST.
  /// \param request Parsed/bounded native transport request, not raw HTTP framing.
  /// \return Semantic HTML with Content-Location for the stable handle path; failures are explicit.
  /// \note Unknown parameters/operations refuse. Completed identical command bodies replay their
  /// stored outcome without AL execution. Started/failed outcomes never automatically retry;
  /// explicit AL Commit may have survived the error. On execution failure the private page is
  /// invalidated, preventing a rolled-back SQL operation from retaining authoritative AL edits.
  [[nodiscard]] ServerHttpResponse Handle(const ServerHttpRequest &request);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
