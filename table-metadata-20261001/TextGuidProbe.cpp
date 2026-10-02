#include "type/Guid.h"
#include "type/Text.h"

agiru::Text<0> Join(const agiru::Guid &guid) {
  return agiru::Text<0>("Configured new external BC company:") + guid;
}
