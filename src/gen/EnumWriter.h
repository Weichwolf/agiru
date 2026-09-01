#pragma once

#include "Ast.h"

#include <map>
#include <string>

namespace agiru::gen {

/// Where one AL enum object landed: the C++ name it took and the header that declares it.
struct EnumRef {
  std::string identifier;
  std::string header;
};

/// Every enum object the run has seen, keyed by its AL name in lower case. A table that names an
/// enum it cannot find here is reported rather than silently emitted against a type that is not
/// there.
using EnumIndex = std::map<std::string, EnumRef>;

std::string WriteEnum(const al::EnumObject &object, const std::string &sourcePath);

std::string EnumHeaderPath(const al::EnumObject &object);

std::string LowerKey(const std::string &alName);

} // namespace agiru::gen
