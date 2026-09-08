#include "type/Option.h"

#include "runtime/Error.h"

#include <string_view>

namespace agiru {

void RefusedOptionValue::Throw() const {
  throw Error(what_ + " scopes through an enumeration this run does not have (board:0032)");
}

RefusedOptionValue RefusedOption(std::string_view what) {
  return RefusedOptionValue(what);
}

}
