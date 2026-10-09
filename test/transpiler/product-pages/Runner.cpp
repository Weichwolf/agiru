#include "runtime/ErrorValue.h"

#include "Check.h"
#include "fixture/page/FeatureHost.h"

#include <string>
#include <string_view>

namespace {

template <class Call> void Refused(std::string_view claim, std::string_view path, Call call) {
  bool refused = false;
  try {
    call();
  } catch (const agiru::Error &error) { refused = std::string(error.what()).contains(path); }
  CHECK_TRUE(claim, refused);
}

void Execute() {
  agiru::Fixture::FeatureHost_Page page;
  page.OpenFeatures();
  CHECK_TRUE("discarded product calls preserve argument effects once in source order",
             page.Current() == 12355);
  CHECK_TRUE("the required generic part still executes its real body", page.RequiredTouches() == 1);
  page.ExtensionFeature();
  CHECK_TRUE("composed extension parts use the same product policy", page.Current() == 1235589);
  Refused("consumed product calls refuse instead of inventing return values",
          "Cloud.Page.ReadValue",
          [&] { static_cast<void>(page.ConsumeCloud()); });
  CHECK_TRUE("consumed refusal preserves its argument effect exactly once",
             page.Current() == 12355896);
  Refused("unselected ERP calls still refuse explicitly", "Missing.Page.Touch", [&] {
    page.MissingERP();
  });
  CHECK_TRUE("unselected ERP refusal retains its argument evaluation", page.Current() == 123558967);
  Refused("generic business chart add-ins remain gaps, never successful stubs",
          "Chart.Refresh",
          [&] { page.UnknownAddIn(); });
  CHECK_TRUE("chart refusal retains its argument evaluation", page.Current() == 1235589678);
  CHECK_TRUE("excluded parts have no runtime PartInstance",
             page.PartInstance("Cloud") == nullptr && page.PartInstance("CloudNumber") == nullptr &&
                 page.PartInstance("ExtensionCloud") == nullptr &&
                 page.PartInstance("Required") != nullptr);
}

}

int main() {
  return gate::Run("ProductPages", Execute);
}
