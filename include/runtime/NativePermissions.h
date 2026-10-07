#pragma once

#include "meta/PermissionSetDef.h"
#include "runtime/PermissionSets.h"
#include "runtime/TablePermissions.h"

#include <cstddef>
#include <cstdint>

/// \file
/// \brief Original SQL Access Control/Tenant Permission authority, shared by every native client.

namespace agiru {

struct PageDef;
struct TableDef;

/// \brief Bounds for one PostgreSQL policy snapshot; not native BC or production scale limits.
struct NativePermissionLimits {
  static constexpr std::size_t kDefaultAssignments = 128; ///< Initial assignment admission bound.
  static constexpr std::size_t kDefaultBytes = 8388608;   ///< Eight-MiB aggregate SQL text bound.
  PermissionSetLimits composition{}; ///< Shared graph/entry/work limits including system metadata.
  std::size_t assignments = kDefaultAssignments; ///< Maximum applicable user/company assignments.
  std::size_t bytes = kDefaultBytes; ///< Sum of nonnull SQL cell text bytes; row bounds also apply.
};

/// \brief Stateless shared provider for an already authenticated, active native session.
/// One SQL statement snapshots applicable original Access Control rows and reachable tenant sets,
/// permissions and relations. System definitions come from the borrowed immutable catalogue;
/// absent definitions refuse, never a synthetic SUPER, imported grant or license gate.
/// The original four permission tables must exist in the trusted connection's storage layout.
/// This provider does not create/migrate them or change company/schema routing. Reads stay below
/// the AL authorization boundary, not an exemption for AL access to system tables.
/// Indirect execution contexts, row filters and in-flight revocation fencing remain unqualified.
class NativePermissions final : public TablePermissionAuthority {
public:
  /// \param system Immutable installed system permission declarations; outlives this provider.
  /// \param limits Positive admission bounds; counts must fit PostgreSQL's integer LIMIT binds.
  /// \throws Error for invalid bounds. Does not open SQL or grant any account.
  explicit NativePermissions(const PermissionSetCatalog &system,
                             NativePermissionLimits limits = {});

  /// \brief Resolves original R/I/M/D/X levels for the current authenticated user/company.
  /// \param object Actual nonzero object ID. \param kind Original object kind.
  /// \param operation Actual requested right, not a page-derived grant.
  /// \return None, Direct or Indirect; an indirect result is not permission to perform an
  /// operation.
  /// \throws Error for unavailable/malformed/unbounded policy, missing sets or security filters.
  /// \note No state or authority is retained between calls or users; the active connection is
  /// borrowed.
  [[nodiscard]] PermissionLevel
  Level(std::int32_t object, PermissionObject kind, PermissionOperation operation) const;

  /// \brief Enforces direct TableData access through the session's existing record boundary.
  /// \param table Original declaration. \param operation Actual R/I/M/D operation.
  /// \return True only for a direct grant; false for absence.
  /// \throws Error for indirect access until an authorized AL execution context exists.
  [[nodiscard]] bool Allows(const TableDef &table, TableOperation operation) const override;

  /// \brief Rechecks Page Execute and source TableData Read before cached values are exposed.
  /// \param page Original installed declaration, never a client-supplied permission identity.
  /// \throws Error with Permission for denial; unsupported indirect access refuses explicitly.
  /// \note Does not grant writes; actual transitive records enforce their individual operations.
  void RequirePage(const PageDef &page) const;

private:
  const PermissionSetCatalog &system_;
  NativePermissionLimits limits_;
};

}
