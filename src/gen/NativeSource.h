#pragma once

#include "Ast.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

struct TranspileScope;

struct NativeSourceIssue {
  std::string source;
  std::string reason;
};

struct NativeSourceExclusion {
  std::string source;
  std::string reason;
  std::string kind;
  int id = 0;
  std::string name;
  std::string nameSpace;
};

struct NativeAppIdentity {
  std::string id;
  std::string name;
  std::string publisher;
  std::string version;
  std::string minimumRuntime{};
};

bool IsAppGuid(std::string_view value);

NativeAppIdentity ParseAppIdentity(std::string_view text);

struct NativeSources {
  std::vector<al::TableObject> tables;
  std::vector<std::string> paths;
  std::vector<al::PageObject> reports;
  std::vector<std::string> reportPaths;
  std::vector<al::EnumObject> enums;
  std::vector<std::string> enumPaths;
  std::vector<al::InterfaceObject> interfaces;
  std::vector<std::string> interfacePaths;
  std::vector<al::CodeunitObject> codeunits;
  std::vector<std::string> codeunitPaths;
  NativeAppIdentity app;
  std::vector<NativeSourceIssue> issues;
  std::vector<std::string> otherSources;
  std::vector<NativeSourceExclusion> excluded;
};

NativeAppIdentity ReadNativeIdentity(const std::filesystem::path &package);

NativeSources ReadNativeSources(const std::filesystem::path &package);

void SelectNativeSources(NativeSources &sources, const TranspileScope &scope);

}
