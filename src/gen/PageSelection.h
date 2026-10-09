#pragma once

#include "Apps.h"

#include <filesystem>
#include <string>
#include <vector>

namespace agiru::al {
struct PageObject;
}

namespace agiru::gen {
struct Objects;

struct PageSelectionRow {
  std::string kind;
  std::string control;
  std::string target;
  std::string targetSource;
  std::string reason;
  std::string location;
  std::string member;
};

void IndexProductPage(const TranspileScope &scope,
                      const std::filesystem::path &source,
                      const al::PageObject &page,
                      Objects &objects,
                      SourceDomain domain = SourceDomain::BCApps);

std::vector<PageSelectionRow> SelectProductPageParts(al::PageObject &page, Objects &objects);

}
