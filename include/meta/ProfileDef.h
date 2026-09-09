#pragma once

#include "meta/Ids.h"

#include <string_view>

/// \file
/// \brief The static declaration of an AL PROFILE: the object kind that names a role centre.
///
/// A profile has no number and no code -- `profile "ORDER PROCESSOR" { Caption; ProfileDescription;
/// RoleCenter; Enabled; Promoted; }` is the whole grammar (`devenv-profile-object.md`) -- so its
/// translation is one `constexpr` row, and the platform's `All Profile` table is that row set: BC
/// shows every installed profile there, and reads the default role centre out of it.

namespace agiru {

/// \brief One `profile` object, as the transpiler read it.
struct ProfileDef {
  std::string_view profileId{};   ///< The object name, which is the `Profile ID` AL filters on.
  std::string_view caption{};     ///< The `Caption` property.
  std::string_view description{}; ///< The `ProfileDescription` property.
  PageId roleCenter{};            ///< The `RoleCenter` property, resolved to the page's number.
  bool enabled = true;            ///< The `Enabled` property; a profile is enabled unless it says.
  bool promoted = false;          ///< The `Promoted` property.
};

}
