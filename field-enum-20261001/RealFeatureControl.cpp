#include "system/environment/configuration/page/FeatureManagement.h"
#include "platform/FeatureKey.h"

#include "Check.h"

int main() {
  return gate::Run("RealFeatureEditability", [] {
    agiru::System::Environment::Configuration::FeatureManagement_Page page;
    page.Rec.IsOneWay = false;
    page.Rec.Enabled = agiru::platform::FeatureKeyEnabled::None;
    CHECK_TRUE("reversible disabled feature is editable", page.OnEditableEnabledFor());
    page.Rec.Enabled = agiru::platform::FeatureKeyEnabled::AllUsers;
    CHECK_TRUE("reversible enabled feature is editable", page.OnEditableEnabledFor());
    page.Rec.IsOneWay = true;
    CHECK_TRUE("one-way enabled feature is not editable", !page.OnEditableEnabledFor());
    page.Rec.Enabled = agiru::platform::FeatureKeyEnabled::None;
    CHECK_TRUE("one-way disabled feature is editable", page.OnEditableEnabledFor());
  });
}
