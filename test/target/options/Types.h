#pragma once

#include "meta/EnumDef.h"
#include "type/Option.h"

#include <array>
#include <cstdint>

namespace agiru::options {

enum class OptionResourceGroupResourceAll : std::int32_t {
  Resource = 0,
  GroupResource = 1,
  All = 2
};
enum class OptionFixedPercentExtraLCYExtra : std::int32_t {
  Fixed = 0,
  PercentExtra = 1,
  LCYExtra = 2
};

}

template <> struct agiru::OptionTraits<agiru::options::OptionResourceGroupResourceAll> {
  static constexpr std::array<EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "Resource", .caption = "Resource"},
      {.ordinal = 1, .name = "Group(Resource)", .caption = "Group(Resource)"},
      {.ordinal = 2, .name = "All", .caption = "All"},
  }};
};

template <> struct agiru::OptionTraits<agiru::options::OptionFixedPercentExtraLCYExtra> {
  static constexpr std::array<EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "Fixed", .caption = "Fixed"},
      {.ordinal = 1, .name = "% Extra", .caption = "% Extra"},
      {.ordinal = 2, .name = "LCY Extra", .caption = "LCY Extra"},
  }};
};
