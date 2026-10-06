#pragma once

#include <string_view>

/// \file
/// \brief Strict UTF-8 validation using the existing Unicode codec.

namespace agiru {

/// \brief Checks RFC 3629 scalar encoding without replacing or normalizing input bytes.
/// \param bytes Input bytes, including any literal Unicode replacement characters.
/// \return False for truncation, overlong encoding, surrogates or out-of-range scalars.
/// ASCII controls are valid UTF-8; the consuming protocol decides whether to allow them.
[[nodiscard]] bool IsValidUtf8(std::string_view bytes);

}
