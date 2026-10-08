#pragma once

#include <cstddef>
#include <string_view>

namespace agiru::detail {

inline bool CanonicalCredentialDigest(std::string_view text) {
  constexpr std::size_t kSha256HexCharacters = 64;
  return text.size() == kSha256HexCharacters &&
         text.find_first_not_of("0123456789abcdef") == std::string_view::npos;
}

}
