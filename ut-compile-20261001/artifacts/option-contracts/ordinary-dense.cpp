#include "type/Option.h"
#include <array>
#include <cstdint>
enum class Members : std::int32_t { First = 0, Second = 1 };
template <> struct agiru::OptionTraits<Members> {
  
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "First", .caption = "First"},
      {.ordinal = 1, .name = "Second", .caption = "Second"},
  }};
};
static_assert(agiru::Option<Members>{Members::First}.Name() == "First");
