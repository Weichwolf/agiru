#pragma once

#include "Ast.h"

#include <stdexcept>
#include <string_view>

namespace agiru::al {

class ParseError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

TableObject ParseTable(std::string_view source);

CodeunitObject ParseCodeunit(std::string_view source);

EnumObject ParseEnum(std::string_view source);

InterfaceObject ParseInterface(std::string_view source);

/// \brief Parses one AL `page` object.
/// \param source The whole `.al` file.
/// \return The page.
PageObject ParsePage(std::string_view source);

} // namespace agiru::al
