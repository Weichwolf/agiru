#pragma once

#include "Scope.h"

#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

std::string Identifier(std::string_view alName);

std::string Unprefixed(std::string_view identifier);

std::vector<std::string> Distinct(const std::vector<std::string> &alNames,
                                  std::set<std::string> taken = {});

std::string EnumeratorName(std::string_view optionMember);

std::vector<std::string> EnumeratorNames(const std::vector<std::string> &members);

std::string OptionContentName(const std::vector<std::string> &members);

std::string OptionEnumName(std::string_view tableName,
                           std::string_view fieldName,
                           const std::vector<std::string> &members = {});

std::string TypeName(std::string_view alType);

bool IsAlTypeName(std::string_view alType);

void NoteObjectName(std::string_view identifier);

bool ShadowsADoorType(std::string_view name);

std::string Literal(std::string_view text);

std::string ClassName(std::string_view identifier, ObjectKind kind);

std::string InNamespace(std::string_view space, std::string_view name);

std::string TraitsOf(std::string_view traits, std::string_view space, std::string_view name);

struct ObjectDeclaration {
  bool found = false;
  int id = 0;
  std::string name;
  std::string nameSpace;
};

ObjectDeclaration DeclarationOf(std::string_view source, ObjectKind kind);

struct ReportControls {
  std::vector<std::pair<std::string, std::string>> dataItems;
  std::vector<std::string> requestFields;
};

ReportControls ReportControlsOf(std::string_view source);

}
