#include "type/RecordId.h"

#include "runtime/Error.h"
#include "type/Integer.h"

#include <cstddef>
#include <string>

namespace agiru {

Integer RecordId::TableNo() const {
  if (IsEmpty()) { throw Error("the RecordId is blank and names no table"); }
  return static_cast<Integer>(table_.Value());
}

std::string RecordId::ToText() const {
  if (IsEmpty()) { return {}; }
  std::string out = caption_ + ": ";
  for (std::size_t i = 0; i < key_.size(); ++i) {
    if (i != 0) { out += ","; }
    out += key_[i];
  }
  return out;
}

} // namespace agiru
