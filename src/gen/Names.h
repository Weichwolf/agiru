#pragma once

#include "Scope.h"

#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

std::string Identifier(std::string_view alName);

std::string EnumeratorName(std::string_view optionMember);

std::vector<std::string> EnumeratorNames(const std::vector<std::string> &members);

std::string OptionEnumName(std::string_view tableName, std::string_view fieldName);

std::string TypeName(std::string_view alType);

bool IsAlTypeName(std::string_view alType);

bool HiddenByABaseMember(std::string_view type);

std::string Literal(std::string_view text);

std::string ClassName(std::string_view identifier, ObjectKind kind);

std::string ClassAlias(std::string_view identifier, ObjectKind kind);

ObjectKind KindOfNamespace(std::string_view space);

struct ObjectDeclaration {
  bool found = false;
  std::string name;
  std::string nameSpace;
};

ObjectDeclaration DeclarationOf(std::string_view source, ObjectKind kind);

}
