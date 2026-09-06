#pragma once

#include "Ast.h"

#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

struct RefusedProperty {
  std::string property;
  std::string where;
};

/// Whether the transpiler refuses this property by DECISION rather than by omission -- the list
/// the run summary counts against (board:0067).
bool RefusedByName(std::string_view name);

void CollectRefused(const std::vector<al::Property> &properties,
                    std::string_view where,
                    std::vector<RefusedProperty> &into);

std::vector<RefusedProperty> Refused(const al::TableObject &table);

std::vector<RefusedProperty> Refused(const al::PageObject &page);

std::vector<RefusedProperty> Refused(const al::CodeunitObject &codeunit);

std::vector<RefusedProperty> Refused(const al::EnumObject &declared);

}
