#pragma once

#include "Token.h"

#include <stdexcept>
#include <string_view>
#include <vector>

namespace agiru::al {

class LexError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

std::vector<Token> Tokenize(std::string_view source);

} // namespace agiru::al
