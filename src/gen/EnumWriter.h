#pragma once

#include "Ast.h"

#include <map>
#include <string>

namespace agiru::gen {

struct Objects;

struct EnumRef {
  std::string identifier;
  std::string header;
};

using EnumIndex = std::map<std::string, EnumRef>;

std::string
WriteEnum(const al::EnumObject &object, const std::string &sourcePath, const Objects &objects);

std::string EnumHeaderPath(const al::EnumObject &object);

[[nodiscard]] std::string WriteEnumSource(const al::EnumObject &object,
                                          const std::string &sourcePath,
                                          const Objects &objects);

[[nodiscard]] std::string EnumSourcePath(const al::EnumObject &object);

std::string LowerKey(const std::string &alName);

}
