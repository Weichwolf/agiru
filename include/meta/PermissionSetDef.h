#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief Immutable permission declarations shared by AL metadata and tenant policy providers.

namespace agiru {

/// \brief Original System Access Control.Scope ordinals; not commercial entitlements.
enum class PermissionScope : std::uint8_t { System = 0, Tenant = 1 };

/// \brief Original System Permission Object Type ordinals, including TableData separately.
enum class PermissionObject : std::uint8_t {
  TableData = 0,
  Table = 1,
  Report = 3,
  Codeunit = 5,
  XmlPort = 6,
  MenuSuite = 7,
  Page = 8,
  Query = 9,
  System = 10
};

/// \brief Independent object rights; Execute is not a TableData read or write permission.
enum class PermissionOperation : std::uint8_t { Read, Insert, Modify, Delete, Execute };

/// \brief Original Tenant Permission field ordinals; Direct overrides Indirect despite numbering.
enum class PermissionLevel : std::uint8_t { None = 0, Direct = 1, Indirect = 2 };

/// \brief Exact permission-set identity; providers normalize app UUID text before publication.
/// Names are original AL role IDs; neither matching role names nor blank apps imply shared scope.
struct PermissionSetIdentity {
  std::string_view app;  ///< Canonical UUID from app.json or System/tenant data.
  std::string_view role; ///< Original role ID, not a display caption.
  PermissionScope scope = PermissionScope::System; ///< Exact original assignment scope.
  /// \brief Compares the complete identity, including app and scope.
  [[nodiscard]] constexpr auto operator<=>(const PermissionSetIdentity &) const = default;
};

/// \brief One object permission, before role-local include/exclude composition.
struct PermissionEntry {
  /// \brief R/I/M/D/X levels, indexed by PermissionOperation.
  using Rights =
      std::array<PermissionLevel, static_cast<std::size_t>(PermissionOperation::Execute) + 1>;
  Rights rights{};         ///< None by default; never an implicit grant.
  std::int32_t object = 0; ///< Original object number; zero is the native wildcard.
  PermissionObject kind = PermissionObject::TableData; ///< Original object category.
  bool securityFiltered = false; ///< Nonempty original filter, not an enforced row predicate.
};

/// \brief An extension contributes rights/includes to its exact target, never independent grants.
/// ExcludedPermissionSets is not an extension property in the AL platform contract.
struct PermissionSetExtensionDef {
  std::span<const PermissionEntry> permissions{};    ///< Original extension's rights.
  std::span<const PermissionSetIdentity> included{}; ///< Original included sets.
};

/// \brief One immutable system/tenant set with its target-bound extensions.
/// The owning catalogue keeps every borrowed identity/array alive throughout resolution.
struct PermissionSetDef {
  PermissionSetIdentity identity;                          ///< Exact app/role/scope owner.
  std::span<const PermissionEntry> permissions{};          ///< This set's own rights.
  std::span<const PermissionSetIdentity> included{};       ///< Recursively added sets.
  std::span<const PermissionSetIdentity> excluded{};       ///< Rights removed at this level only.
  std::span<const PermissionSetExtensionDef> extensions{}; ///< Extensions installed for this set.
};

}
