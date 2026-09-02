#include "type/Variant.h"

#include "runtime/Error.h"

namespace agiru {

void Variant::Refuse() {
  throw Error("the Variant does not hold that type");
}

} // namespace agiru
