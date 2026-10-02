#pragma once

#include "meta/EnumDef.h"
#include "type/Option.h"

#include <array>
#include <cstdint>

/// \file
/// \brief Shared System-symbol vocabulary for User and User Personalization license fields.

namespace agiru::platform {

/// \brief System `License Type` members in their declared ordinal order.
enum class UserLicenseType : std::int32_t {
  FullUser = 0,              ///< A named user with the full application.
  LimitedUser = 1,           ///< Restricted writable-table access.
  DeviceOnlyUser = 2,        ///< Licensed per device.
  WindowsGroup = 3,          ///< A Windows group.
  ExternalUser = 4,          ///< An external user.
  ExternalAdministrator = 5, ///< A delegated administrator.
  ExternalAccountant = 6,    ///< A delegated accountant.
  Application = 7,           ///< A service principal.
  AADGroup = 8,              ///< An Entra group.
  Agent = 9,                 ///< An agent acting for a user.
};

}

/// \brief The identical option declaration in User and User Personalization System symbols.
template <> struct agiru::OptionTraits<agiru::platform::UserLicenseType> {
  /// \brief Ten declared members; names and captions preserve System spelling.
  static constexpr std::array<agiru::EnumValueDef, 10> kValues{{
      {.ordinal = 0, .name = "Full User", .caption = "Full User"},
      {.ordinal = 1, .name = "Limited User", .caption = "Limited User"},
      {.ordinal = 2, .name = "Device Only User", .caption = "Device Only User"},
      {.ordinal = 3, .name = "Windows Group", .caption = "Windows Group"},
      {.ordinal = 4, .name = "External User", .caption = "External User"},
      {.ordinal = 5, .name = "External Administrator", .caption = "External Administrator"},
      {.ordinal = 6, .name = "External Accountant", .caption = "External Accountant"},
      {.ordinal = 7, .name = "Application", .caption = "Application"},
      {.ordinal = 8, .name = "AAD Group", .caption = "AAD Group"},
      {.ordinal = 9, .name = "Agent", .caption = "Agent"},
  }};
};
