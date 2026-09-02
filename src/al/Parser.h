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

/// \brief Parses one AL `tableextension` object.
/// \param source The whole `.al` file.
/// \return The extension, carrying the name of the table it extends.
TableExtensionObject ParseTableExtension(std::string_view source);

/// \brief Parses one AL `enumextension` object.
/// \param source The whole `.al` file.
/// \return The extension, carrying the name of the enumeration it extends.
EnumExtensionObject ParseEnumExtension(std::string_view source);

/// \brief Parses one AL `pageextension` object.
/// \param source The whole `.al` file.
/// \return The extension, carrying the name of the page it extends.
PageExtensionObject ParsePageExtension(std::string_view source);

} // namespace agiru::al
