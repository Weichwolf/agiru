#include "type/Variant.h"

#include "runtime/Error.h"

namespace agiru {

void Variant::Refuse() {
  throw Error("the Variant does not hold that type");
}

}

namespace agiru {

Variant::Variant(const Variant &o) = default;
Variant::Variant(Variant &&o) noexcept = default;
Variant &Variant::operator=(const Variant &o) = default;
Variant &Variant::operator=(Variant &&o) noexcept = default;
Variant::~Variant() = default;

bool Variant::operator==(const Variant &o) const {
  return held_ == o.held_;
}

}
