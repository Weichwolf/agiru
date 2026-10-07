#pragma once

#include "meta/PermissionSetDef.h"

#include <cstddef>
#include <span>

/// \file
/// \brief Bounded, presentation-independent resolution of original BC permission-set composition.

namespace agiru {

/// \brief Immutable or command-owned source of permission declarations, never a global user cache.
class PermissionSetCatalog {
public:
  virtual ~PermissionSetCatalog() = default;
  /// \brief Resolves an exact app/role/scope identity, including its installed extensions.
  /// \param identity Trusted assignment/include/exclude identity.
  /// \return Exact declaration or null; all borrowed data stays alive for the resolution call.
  /// \note Providers must reject duplicate identities and preserve unsupported policy as errors.
  [[nodiscard]] virtual const PermissionSetDef *
  Find(const PermissionSetIdentity &identity) const = 0;
};

/// \brief Explicit admission bounds per resolution, independent of production scale claims.
struct PermissionSetLimits {
  static constexpr std::size_t kDefaultSets =
      512; ///< Initial host admission bound, not a BC limit.
  static constexpr std::size_t kDefaultDepth =
      64; ///< Initial host recursion bound, not a BC limit.
  static constexpr std::size_t kDefaultEdges = 4096;    ///< Initial reference-work admission bound.
  static constexpr std::size_t kDefaultEntries = 65536; ///< Initial entry-work admission bound.
  std::size_t sets = kDefaultSets;   ///< Maximum distinct catalogue identities visited.
  std::size_t depth = kDefaultDepth; ///< Maximum active include/exclude path length.
  std::size_t edges =
      kDefaultEdges; ///< Maximum references/extensions, including cached references.
  std::size_t entries =
      kDefaultEntries; ///< Maximum examined entries, including installed extensions.
};

/// \brief Combines assigned sets, with exclusions confined to each recursively expanded set.
/// \param catalog Exact declarations, not a permissive role-name lookup.
/// \param assigned Sets already selected for the authenticated user/company by its provider.
/// \param object Actual original nonzero object number; zero is only a declaration wildcard.
/// \param kind Original object category, distinct from its numeric ID.
/// \param operation Independent requested R/I/M/D/X right.
/// \param limits Positive per-call resource bounds; no cross-session memoized authority.
/// \return None, Direct or Indirect. Indirect still requires authorized AL execution context;
/// this result alone never grants a table/page/object operation. Empty assignments deny.
/// \throws Error for missing/mismatched/invalid declarations, cycles or exhausted bounds.
/// \throws Error with PermissionFilterUnsupported for applicable included security filters;
/// row predicates are not implemented here. Excluded-set filters do not negate included filters.
[[nodiscard]] PermissionLevel ResolvePermission(const PermissionSetCatalog &catalog,
                                                std::span<const PermissionSetIdentity> assigned,
                                                std::int32_t object,
                                                PermissionObject kind,
                                                PermissionOperation operation,
                                                PermissionSetLimits limits = {});

}
