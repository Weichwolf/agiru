#include "type/Guid.h"
#include "type/Text.h"

#include <type_traits>

static_assert(std::is_same_v<decltype(agiru::Text<0>{} + agiru::Guid{}), agiru::Text<0>>);
static_assert(std::is_same_v<decltype(agiru::Guid{} + agiru::Text<0>{}), agiru::Text<0>>);
static_assert(std::is_same_v<decltype(agiru::Text<2>{} + agiru::Guid{}), agiru::Text<0>>);
static_assert(std::is_same_v<decltype(agiru::Guid{} + agiru::Text<2>{}), agiru::Text<0>>);

agiru::Text<0> Join(const agiru::Guid &identity) {
  return agiru::Text<0>{"Configured new external BC company:"} + identity;
}
