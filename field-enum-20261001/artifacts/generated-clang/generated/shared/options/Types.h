// Generated from every option AL declares by its members. Do not edit.
// Do not edit.

#pragma once

#include "meta/EnumDef.h"
#include "type/Option.h"

#include <array>
#include <cstdint>

namespace agiru::options {

enum class OptionNoneReady : std::int32_t {
  None = 0,
  Ready = 1,
};

} // namespace agiru::options

template <> struct agiru::OptionTraits<agiru::options::OptionNoneReady> {
  static constexpr std::array<EnumValueDef, 2> kValues{{
      EnumValueDef{.ordinal = 0, .name = "None", .caption = "None"},
      EnumValueDef{.ordinal = 1, .name = "Ready", .caption = "Ready"},
  }};
};

