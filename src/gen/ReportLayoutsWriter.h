#pragma once

#include "Ast.h"

#include <string>

namespace agiru::gen {

std::string ReportLayoutsIncludes(const al::PageObject &report);

std::string ReportLayoutsDeclaration(const al::PageObject &report);

std::string ReportLayoutsTrait(const al::PageObject &report);

std::string ReportLayoutDefinitions(const al::PageObject &report);

}
