#include "type/Version.h"

#include "runtime/Error.h"
#include "type/Integer.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>

namespace agiru {

Version Version::FromText(std::string_view text) {
  std::array<Integer, 4> parts{};
  std::size_t at = 0;
  std::size_t from = 0;
  while (from <= text.size() && at < 4) {
    const std::size_t dot = text.find('.', from);
    const std::string_view part =
        text.substr(from, dot == std::string_view::npos ? text.size() - from : dot - from);
    int value = 0;
    const std::from_chars_result read =
        std::from_chars(part.data(), part.data() + part.size(), value);
    if (read.ec != std::errc{} || read.ptr != part.data() + part.size()) {
      throw Error("'" + std::string(text) + "' is not a version");
    }
    parts[at++] = value;
    if (dot == std::string_view::npos) { break; }
    from = dot + 1;
  }
  return Version{parts[0], parts[1], parts[2], parts[3]};
}

std::string Version::ToText() const {
  return std::to_string(parts_[0]) + "." + std::to_string(parts_[1]) + "." +
         std::to_string(parts_[2]) + "." + std::to_string(parts_[3]);
}

}
