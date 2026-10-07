#pragma once

#include "runtime/PermissionSets.h"

#include <span>

/// \file
/// \brief Immutable installed system permission metadata, never user/company authority.
namespace agiru {

/// \brief Registers an original system declaration before any catalogue read.
/// \param declaration Process-lifetime immutable declaration and referenced arrays.
/// \throws Error for missing/noncanonical system identity, duplicate identity or late registration.
/// \note Does not synthesize roles or grant users. Tenant declarations remain in PostgreSQL.
void RegisterPermissionSet(const PermissionSetDef *declaration);

/// \return Frozen declarations, ordered by exact app/role/scope identity.
[[nodiscard]] std::span<const PermissionSetDef *const> InstalledPermissionSetDefinitions();

/// \brief Exact lookup over frozen compiled system metadata; absence remains a counted gap.
class InstalledPermissionSets final : public PermissionSetCatalog {
public:
  /// \param identity Exact original app/role/scope key.
  /// \return Borrowed immutable declaration, or null for an uninstalled identity.
  [[nodiscard]] const PermissionSetDef *Find(const PermissionSetIdentity &identity) const override;
};

}
