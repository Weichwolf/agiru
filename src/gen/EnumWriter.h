#pragma once

#include "Ast.h"

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

struct Objects;

struct EnumRef {
  std::string identifier;
  std::string header;
  std::map<std::string, int> ordinals;
  std::map<std::string, std::string> members;
};

using EnumIndex = std::map<std::string, EnumRef>;

std::string
WriteEnum(const al::EnumObject &object, const std::string &sourcePath, const Objects &objects);

std::string EnumHeaderPath(const al::EnumObject &object);

[[nodiscard]] std::string WriteEnumSource(const al::EnumObject &object,
                                          const std::string &sourcePath,
                                          const Objects &objects);

[[nodiscard]] std::string EnumSourcePath(const al::EnumObject &object);

struct ForeignImplementationRef {
  std::string enumName;
  int ordinal = 0;
  std::string face;
  std::string codeunit;
};

[[nodiscard]] std::string
ImplementationKey(std::string_view enumName, int ordinal, std::string_view face);

[[nodiscard]] std::vector<ForeignImplementationRef>
UnresolvedImplementations(const al::EnumObject &object, const Objects &objects);

[[nodiscard]] std::string WriteEnumExtensionSource(const al::EnumExtensionObject &extension,
                                                   const std::string &sourcePath,
                                                   const std::set<std::string> &pending,
                                                   const Objects &objects,
                                                   std::vector<std::string> &registered);

[[nodiscard]] std::string EnumExtensionSourcePath(const al::EnumExtensionObject &extension);

std::string LowerKey(const std::string &alName);

}
