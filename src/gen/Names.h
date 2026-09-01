#pragma once

#include <string>
#include <string_view>

namespace agiru::gen {

std::string Identifier(std::string_view alName);

std::string EnumeratorName(std::string_view optionMember);

std::string OptionEnumName(std::string_view tableName, std::string_view fieldName);

std::string TypeName(std::string_view alType);

std::string Literal(std::string_view text);

} // namespace agiru::gen
