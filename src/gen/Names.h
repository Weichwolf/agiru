#pragma once

#include "Scope.h"

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

std::string Identifier(std::string_view alName);

std::vector<std::string> Distinct(const std::vector<std::string> &alNames,
                                  std::set<std::string> taken = {});

std::string EnumeratorName(std::string_view optionMember);

std::vector<std::string> EnumeratorNames(const std::vector<std::string> &members);

/// The name an option enumeration carries: its CONTENT, in the one namespace every object shares.
/// Two options with one member list are ONE type wherever they are declared, which is what AL
/// means by an option and what lets a field bind to a `var Option` parameter (board:0586).
std::string OptionContentName(const std::vector<std::string> &members);

std::string OptionEnumName(std::string_view tableName,
                           std::string_view fieldName,
                           const std::vector<std::string> &members = {});

std::string TypeName(std::string_view alType);

bool IsAlTypeName(std::string_view alType);

std::string Literal(std::string_view text);

std::string ClassName(std::string_view identifier, ObjectKind kind);

std::string ClassAlias(std::string_view identifier, ObjectKind kind);

ObjectKind KindOfNamespace(std::string_view space);

struct ObjectDeclaration {
  bool found = false;
  int id = 0;
  std::string name;
  std::string nameSpace;
};

ObjectDeclaration DeclarationOf(std::string_view source, ObjectKind kind);

}
