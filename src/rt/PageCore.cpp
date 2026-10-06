#include "runtime/PageCore.h"

#include "runtime/ErrorValue.h"
#include "runtime/PageValue.h"

#include <string_view>

namespace agiru {

PageValue PageCore::Control_Value(std::string_view control) const {
  static_cast<void>(control);
  throw Error("Control has no exact typed value binding", "PageValueUnsupported");
}

}
