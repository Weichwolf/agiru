// Generated from every option AL declares by its members. Do not edit.
// Do not edit.

#pragma once

#include "meta/EnumDef.h"
#include "type/Option.h"

#include <array>
#include <cstdint>

namespace agiru::options {

enum class OptionBlankAB : std::int32_t {
  Blank = 0,
  A = 1,
  B = 2,
};

} // namespace agiru::options

template <> struct agiru::OptionTraits<agiru::options::OptionBlankAB> {
  static constexpr std::array<EnumValueDef, 3> kValues{{
      EnumValueDef{.ordinal = 0, .name = "Blank", .caption = "Blank"},
      EnumValueDef{.ordinal = 1, .name = "A", .caption = "A"},
      EnumValueDef{.ordinal = 2, .name = "B", .caption = "B"},
  }};
};

namespace agiru::options {

enum class OptionBlankCD : std::int32_t {
  Blank = 0,
  C = 1,
  D = 2,
};

} // namespace agiru::options

template <> struct agiru::OptionTraits<agiru::options::OptionBlankCD> {
  static constexpr std::array<EnumValueDef, 3> kValues{{
      EnumValueDef{.ordinal = 0, .name = "Blank", .caption = "Blank"},
      EnumValueDef{.ordinal = 1, .name = "C", .caption = "C"},
      EnumValueDef{.ordinal = 2, .name = "D", .caption = "D"},
  }};
};

