#include "meta/ModuleDef.h"
#include "meta/PageDef.h"

#include "Check.h"
#include "fixture/page/ExtensionControls.h"
#include "fixture/report/ExtensionRequestControls.h"
#include "other/page/DefaultPageProperties.h"

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>

namespace {

const agiru::ControlDef &Required(std::span<const agiru::ControlDef> controls,
                                  std::string_view name) {
  const auto *found = agiru::Control(controls, name);
  if (found == nullptr) { throw std::runtime_error("missing authored control"); }
  return *found;
}

void Ordered(std::span<const agiru::ControlDef> controls, std::span<const std::string_view> names) {
  CHECK_TRUE("extension controls retain the entire sibling population",
             controls.size() == names.size());
  if (controls.size() != names.size()) { throw std::runtime_error("wrong sibling population"); }
  for (std::size_t i = 0; i < names.size(); ++i) {
    CHECK_TEXT("extension operations preserve declared sibling order", controls[i].name, names[i]);
    CHECK_TRUE("extension operations are consumed, not unknown controls",
               controls[i].kind != agiru::ControlKind::Unknown);
  }
}

void Extended(const agiru::PageDef &page) {
  CHECK_TRUE("an extended page retains its original application", page.module != nullptr);
  if (page.module == nullptr) { throw std::runtime_error("missing original page application"); }
  CHECK_TEXT("application identity is borrowed from the declaring app",
             page.module->id,
             "118874ab-44bc-4ccb-9daf-59763539ab16");
  CHECK_TEXT(
      "page namespace remains its original AL spelling", page.nameSpace, "Microsoft.Fixture");
  CHECK_TRUE("page/request-page MultipleNewLines survives extension composition",
             page.multipleNewLines);
  constexpr std::array<std::string_view, 6> fields{
      "First", "Before", "Original", "After", "Later", "Dependent"};
  constexpr std::array<std::string_view, 6> actions{"FirstAction",
                                                    "BeforeAction",
                                                    "OriginalAction",
                                                    "AfterAction",
                                                    "LaterAction",
                                                    "DependentAction"};
  Ordered(Required(page.layout, "Fields").children, fields);
  CHECK_TRUE("the declared action area remains Processing",
             page.actions.size() == 1 && page.actions.front().area == agiru::AreaKind::Processing);
  if (page.actions.empty()) { throw std::runtime_error("missing action area"); }
  Ordered(page.actions.front().children, actions);
  CHECK_TEXT("modify replaces the existing field caption",
             Required(page.layout, "Original").caption,
             "Changed caption");
  CHECK_TEXT(
      "modify adds an absent field property", Required(page.layout, "Original").editable, "false");
  CHECK_TEXT("deferred modify observes a subsequently inserted control",
             Required(page.layout, "Dependent").caption,
             "Changed dependent");
  CHECK_TEXT("modify preserves action identity and updates its caption",
             Required(page.actions, "OriginalAction").caption,
             "Changed action");
}

}

int main() {
  return gate::Run("Generated Control Extensions", [] {
    Extended(agiru::PageTraits<agiru::Fixture::ExtensionControls_Page>::kPage);
    Extended(agiru::PageTraits<agiru::Fixture::ExtensionRequestControls_Report>::kPage);
    const auto &defaults = agiru::PageTraits<agiru::Other::DefaultPageProperties_Page>::kPage;
    CHECK_TRUE("MultipleNewLines defaults to false", !defaults.multipleNewLines);
    CHECK_TRUE("pages in different namespaces borrow the same declaring application",
               defaults.module ==
                   agiru::PageTraits<agiru::Fixture::ExtensionControls_Page>::kPage.module);
    CHECK_TEXT("a second namespace does not change application ownership",
               defaults.nameSpace,
               "Microsoft.Other");
    CHECK_TEXT("the original AL object name remains independent",
               defaults.name,
               "Default Page Properties");
    CHECK_TEXT("a different caption is retained", defaults.caption, "A distinct page caption");
  });
}
