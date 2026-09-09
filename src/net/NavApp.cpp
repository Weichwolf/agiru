#include "type/NavApp.h"

#include "type/Boolean.h"
#include "type/Guid.h"
#include "type/ModuleInfo.h"
#include "type/Version.h"

#include <string>

namespace agiru {

namespace {

void Describe(::agiru::ModuleInfo &info, const ModuleDef &module) {
  const Version version = Version::FromText(module.version);
  info.Describe(
      Guid(module.id), std::string(module.name), std::string(module.publisher), version, version);
}

}

::agiru::Boolean NavApp::GetCallerModuleInfo(::agiru::ModuleInfo &Info, const ModuleDef &module) {
  Describe(Info, module);
  return true;
}

::agiru::Boolean NavApp::GetCurrentModuleInfo(::agiru::ModuleInfo &Info, const ModuleDef &module) {
  Describe(Info, module);
  return true;
}

}
