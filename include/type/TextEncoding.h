#pragma once

#include <cstdint>

/// \file
/// \brief AL `TextEncoding` -- Represents a file encoding.
///
/// The members and their order come from `methods-auto/textencoding/textencoding-option.md`, which
/// is the specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `TextEncoding`. Represents a file encoding.
enum class TextEncoding : std::int32_t {
  MSDos,   ///< MSDos encoding.
  UTF8,    ///< UTF8 encoding.
  UTF16,   ///< UTF16 encoding.
  Windows, ///< Windows encoding.
};

}
