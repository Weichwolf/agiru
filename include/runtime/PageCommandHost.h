#pragma once

#include "runtime/PageHostOptions.h"

#include <cstdint>
#include <functional>
#include <memory>

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

/// \brief Installs host-owned context/receipt storage using trusted migration authority.
/// \param connection Database containing the original User and agiru_client credential store.
/// \note Grants no users or ERP permissions; caller owns commit/rollback. Migration preserves
/// receipts but invalidates legacy contexts without a credential verifier; never adopts them.
void InstallPageCommandHost(const Connection &connection);

/// \brief Shared generated-page execution with SQL ownership/revisions and durable receipts.
/// Request-local connections are released after each command. Private AL pages remain bounded
/// in this process; PostgreSQL owns user/company/credential identity, expiry, fencing and outcomes.
/// Separate credentials for one user cannot share page/call/dialog/receipt handles.
/// A host restart refuses old handles rather than fabricating recovered AL state. This initial
/// single-company adapter exposes HTTPS development-credential exchange, not password login.
/// It does not implement company schema routing,
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
