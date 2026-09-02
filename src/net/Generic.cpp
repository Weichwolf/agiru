#include "dotnet/Generic.h"

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <algorithm>
#include <cstddef>
#include <string>

namespace agiru::dotnet {

const Variant &GenericList1::Item(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= items_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside a list of " +
                std::to_string(items_.size()) + ", which .NET counts from zero");
  }
  return items_[static_cast<std::size_t>(index)];
}

Boolean GenericList1::Contains(const Variant &item) const {
  return std::ranges::find(items_, item) != items_.end();
}

const Variant &GenericDictionary2::Item(const std::string &key) const {
  const auto found = entries_.find(key);
  if (found == entries_.end()) { throw Error("the dictionary holds nothing under '" + key + "'"); }
  return found->second;
}

} // namespace agiru::dotnet
