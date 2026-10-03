#pragma once

#include "Ast.h"

#include <filesystem>
#include <string>
#include <vector>

namespace agiru::gen {

struct NativeSourceIssue {
  std::string source;
  std::string reason;
};

struct NativeAppIdentity {
  std::string id;
  std::string name;
  std::string publisher;
  std::string version;
};

struct NativeSources {
  std::vector<al::TableObject> tables;
  std::vector<std::string> paths;
  std::vector<al::PageObject> reports;
  std::vector<std::string> reportPaths;
  NativeAppIdentity app;
  std::vector<NativeSourceIssue> issues;
  std::vector<std::string> otherSources;
};

NativeAppIdentity ReadNativeIdentity(const std::filesystem::path &package);

NativeSources ReadNativeSources(const std::filesystem::path &package);

}
