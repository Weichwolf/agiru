#include "dotnet/ALConfigSettings.h"
#include "dotnet/NavTenantSettingsHelper.h"
#include "dotnet/NavTestExecution.h"
#include "platform/Tenant.h"
#include "runtime/Session.h"
#include "runtime/test/Handlers.h"

#include "Check.h"

namespace {

using agiru::dotnet::ALConfigSettings;
using agiru::dotnet::NavTenantSettingsHelper;
using agiru::dotnet::NavTestExecution;

void AnswersFollowTheCurrentSession() {
  agiru::Session parent(AGIRU_TEST_DSN);
  parent.Tenant().saas = true;
  parent.Tenant().sandbox = true;
  parent.Tenant().environmentName = "Parent";
  parent.Tenant().applicationFamily = "Parent Family";
  parent.Tenant().ringName = "Parent Ring";
  CHECK_TRUE("configuration bridge uses the parent session", ALConfigSettings::IsSaaS());
  CHECK_TRUE("Excel bridge uses the parent session", ALConfigSettings::IsSaasExcelAddinEnabled());
  CHECK_TRUE("extension bridge uses the parent session",
             ALConfigSettings::EnableSaasExtensionInstallConfigSetting());
  CHECK_TRUE("sandbox bridge uses the parent session", NavTenantSettingsHelper::IsSandbox());
  CHECK_TRUE("production bridge uses the parent session", !NavTenantSettingsHelper::IsProduction());
  CHECK_TEXT("environment bridge", NavTenantSettingsHelper::GetEnvironmentName(), "Parent");
  CHECK_TEXT("family bridge", NavTenantSettingsHelper::GetApplicationFamily(), "Parent Family");
  CHECK_TEXT("ring bridge", NavTenantSettingsHelper::GetRingName(), "Parent Ring");
  {
    const agiru::Session child(AGIRU_TEST_DSN);
    CHECK_TRUE("child configuration does not adopt parent state", !ALConfigSettings::IsSaaS());
    CHECK_TRUE("child tenant does not adopt parent state", !NavTenantSettingsHelper::IsSandbox());
    CHECK_TEXT("child environment does not adopt parent state",
               NavTenantSettingsHelper::GetEnvironmentName(),
               "");
  }
  CHECK_TRUE("parent configuration restored", ALConfigSettings::IsSaaS());
  CHECK_TEXT(
      "parent environment restored", NavTenantSettingsHelper::GetEnvironmentName(), "Parent");
}

void TestModeFollowsTheExistingHandlerBoundary() {
  CHECK_TRUE("outside a case", !NavTestExecution::IsInTestMode());
  agiru::HandlerTable::Install({}, {});
  CHECK_TRUE("inside a case even without dialog handlers", NavTestExecution::IsInTestMode());
  agiru::HandlerTable::Reset();
  CHECK_TRUE("case cleanup is observed", !NavTestExecution::IsInTestMode());
}

}

int main() {
  return gate::Run("SessionBridge", [] {
    AnswersFollowTheCurrentSession();
    TestModeFollowsTheExistingHandlerBoundary();
  });
}
