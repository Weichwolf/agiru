#include "dotnet/ALConfigSettings.h"

#include "runtime/Session.h"
#include "type/Boolean.h"

namespace agiru::dotnet {

namespace {

Boolean Service() {
  return Session::HasCurrent() && Session::Current().IsSaaS();
}

} // namespace

Boolean ALConfigSettings::ApiServicesEnabled() {
  return Service();
}

Boolean ALConfigSettings::IsSaaS() {
  return Service();
}

Boolean ALConfigSettings::IsSaasExcelAddinEnabled() {
  return Service();
}

Boolean ALConfigSettings::EnableSaasExtensionInstallConfigSetting() {
  return Service();
}

} // namespace agiru::dotnet
