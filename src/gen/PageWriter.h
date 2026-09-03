#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"

#include <string>

namespace agiru::gen {

struct PageHeader {
  std::string text;
  DotNetUse dotnet;
  DotNetUse absent;
};

PageHeader
WritePage(const al::PageObject &object, const std::string &source, const Objects &objects);

std::string PageHeaderPath(const al::PageObject &object);

} // namespace agiru::gen
