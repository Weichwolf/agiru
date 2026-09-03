#pragma once

#include <string>

namespace agiru::gen {

struct FormatRequest {
  std::string source;
  std::string stylePath;
  std::string assumedName;
};

std::string Formatted(const FormatRequest &request);

}
