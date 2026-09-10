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

PageObject ParsePage(std::string_view source);

QueryObject ParseQuery(std::string_view source);

PageObject ParseReport(std::string_view source);

PageObject ParseXmlPort(std::string_view source);

PageExtensionObject ParseReportExtension(std::string_view source);

ProfileObject ParseProfile(std::string_view source);

TableExtensionObject ParseTableExtension(std::string_view source);

EnumExtensionObject ParseEnumExtension(std::string_view source);

PageExtensionObject ParsePageExtension(std::string_view source);

}
