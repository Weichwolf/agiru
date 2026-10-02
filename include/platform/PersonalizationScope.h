#pragma once

#include "meta/EnumDef.h"
#include "type/Option.h"

#include <array>
#include <cstdint>

/// \file
/// \brief Shared System/Tenant option vocabulary from the pinned System table declarations.

namespace agiru::platform {

/// \brief Source Scope members of All Profile and User Personalization.
enum class PersonalizationScope : std::int32_t {
  System = 0, ///< Platform-provided scope.
  Tenant = 1, ///< Tenant-specific scope.
};

}

/// \brief Source Scope names, captions and ordinary option positions.
template <> struct agiru::OptionTraits<agiru::platform::PersonalizationScope> {
  /// \brief The two source members, shared without a table-header dependency.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "System", .caption = "System"},
      {.ordinal = 1, .name = "Tenant", .caption = "Tenant"},
  }};
};
