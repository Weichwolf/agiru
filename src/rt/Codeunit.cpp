#include "runtime/Codeunit.h"

#include "runtime/Error.h"

#include <string>
#include <string_view>

namespace agiru::detail {

void RefuseRun(std::string_view name) {
  throw Error("Codeunit.Run(" + std::string(name) +
              ") needs the transaction boundary an error rolls back to (board:0021)");
}

} // namespace agiru::detail
