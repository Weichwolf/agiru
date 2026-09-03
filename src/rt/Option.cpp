#include "type/Option.h"

#include "runtime/Error.h"

#include <string>
#include <string_view>

namespace agiru {

Option<> RefusedOption(std::string_view what) {
  throw Error(std::string(what) +
              " scopes through an enumeration this run does not have (board:0032)");
}

} // namespace agiru
