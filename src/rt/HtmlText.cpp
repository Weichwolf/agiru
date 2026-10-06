#include "HtmlText.h"

#include "runtime/ErrorValue.h"
#include "type/Utf8.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::detail {

void AppendHtmlText(std::string &output, std::string_view text, std::size_t bytes) {
  if (text.size() > bytes) { throw Error("Page HTML text budget exceeded", "PageHtmlLimit"); }
  if (!IsValidUtf8(text)) { throw Error("Page HTML requires valid UTF-8", "PageHtmlText"); }
  const auto append = [&](std::string_view escaped) {
    if (output.size() > bytes || escaped.size() > bytes - output.size()) {
      throw Error("Page HTML output budget exceeded", "PageHtmlLimit");
    }
    output += escaped;
  };
  for (const unsigned char unit : text) {
    switch (unit) {
      case '&': append("&amp;"); break;
      case '<': append("&lt;"); break;
      case '>': append("&gt;"); break;
      case '"': append("&quot;"); break;
      case '\'': append("&#39;"); break;
      case '\r': append("&#13;"); break;
      case '\n': append("&#10;"); break;
      case '\t': append("&#9;"); break;
      default:
        constexpr unsigned char kAsciiDelete = 127;
        if (unit < ' ' || unit == kAsciiDelete) {
          throw Error("Control character cannot round-trip through HTML", "PageHtmlText");
        }
        append(std::string_view(reinterpret_cast<const char *>(&unit), 1));
        break;
    }
  }
}

}
