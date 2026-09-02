#include "dotnet/NavTenantSettingsHelper.h"

#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Tenant.h"
#include "type/Boolean.h"

#include <string>

namespace agiru::dotnet {

namespace {

const TenantSettings &Tenant() {
  static const TenantSettings kSelfHosted;
  return Session::HasCurrent() ? Session::Current().Tenant() : kSelfHosted;
}

} // namespace

Boolean NavTenantSettingsHelper::IsProduction() {
  return !Tenant().sandbox;
}

Boolean NavTenantSettingsHelper::IsSandbox() {
  return Tenant().sandbox;
}

std::string NavTenantSettingsHelper::GetEnvironmentName() {
  return Tenant().environmentName;
}

std::string NavTenantSettingsHelper::GetApplicationFamily() {
  return Tenant().applicationFamily;
}

std::string NavTenantSettingsHelper::GetRingName() {
  return Tenant().ringName;
}

void NavTenantSettingsHelper::EnableM365Collaboration() {
  throw Error("enabling Microsoft 365 collaboration changes the tenant, and this runtime has no "
              "tenant service to change");
}

} // namespace agiru::dotnet
