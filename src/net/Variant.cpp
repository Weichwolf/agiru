#include "type/Variant.h"

#include "runtime/Error.h"

#include <string>

namespace agiru {

void Variant::Refuse() const {
  throw Error("the Variant does not hold that type (it holds alternative " +
              std::to_string(held_.index()) + " of Variant::Held)");
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
