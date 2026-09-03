#pragma once

#include "Ast.h"

#include <map>
#include <string>

namespace agiru::gen {

struct EnumRef {
  std::string identifier;
  std::string header;
};

using EnumIndex = std::map<std::string, EnumRef>;

std::string WriteEnum(const al::EnumObject &object, const std::string &sourcePath);

std::string EnumHeaderPath(const al::EnumObject &object);

std::string LowerKey(const std::string &alName);

}
