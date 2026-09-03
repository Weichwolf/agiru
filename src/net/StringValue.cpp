#include "type/StringValue.h"

#include "type/Char.h"
#include "type/Integer.h"

#include <cstddef>
#include <string>

namespace agiru {

Char StringValue::operator[](Integer index) const {
  if (index < 1) {
    throw StringError("the string index " + std::to_string(index) +
                      " is below one, and AL counts from one");
  }
  return Char{detail::CodePointAt(Stored(), static_cast<std::size_t>(index) - 1)};
}

}
