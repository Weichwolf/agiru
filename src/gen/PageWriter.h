#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"

#include <map>
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

[[nodiscard]] std::map<std::string, std::string> ControlIdentifiers(const al::PageObject &object);

[[nodiscard]] std::string ControlIdentifier(const std::map<std::string, std::string> &named,
                                            std::string_view alName);

}
