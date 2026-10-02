#include "type/Option.h"
#include <array>
#include <cstdint>
enum class Members : std::int32_t { First = 4912, Second = 31489 };
template <> struct agiru::OptionTraits<Members> {
  
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 4912, .name = "First", .caption = "First"},
      {.ordinal = 31489, .name = "Second", .caption = "Second"},
  }};
};
static_assert(agiru::Option<Members>{Members::First}.Name() == "First");
