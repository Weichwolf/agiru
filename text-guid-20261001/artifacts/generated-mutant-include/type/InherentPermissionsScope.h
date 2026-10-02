#pragma once

#include <cstdint>

/// \file
/// \brief AL `InherentPermissionsScope` -- The different types of scope that the
/// InherentPermissions attribute can apply to.
///
/// The members and their order come from
/// `methods-auto/inherentpermissionsscope/inherentpermissionsscope-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `InherentPermissionsScope`. The different types of scope that the InherentPermissions
/// attribute can apply to.
enum class InherentPermissionsScope : std::int32_t {
  Permissions,  ///< The Permissions scope
  Entitlements, ///< The Entitlements scope
  Both,         ///< The Both scope
};

}
