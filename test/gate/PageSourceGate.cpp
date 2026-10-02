#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Session.h"
#include "runtime/test/TestField.h"
#include "runtime/test/TestPage.h"
#include "type/Decimal.h"
#include "type/StringValue.h"

#include "Check.h"

#include <array>
#include <string>
#include <string_view>

namespace {

class SourcePage : public agiru::Page<SourcePage> {
public:
  agiru::Text<0> Read() { return std::to_string(++reads_); }

  agiru::Text<0> Amount() { return "999999999999999.99"; }

  agiru::Text<0> Fail() { throw agiru::Error("source failed", "SourceFailure"); }

  agiru::Text<0> Empty() { return {}; }

  std::string Legacy() const { return legacy_; }

  void Legacy(std::string_view value) { legacy_ = value; }

private:
  int reads_ = 0;
  std::string legacy_ = "before";
};

struct Controls {};

constexpr std::array kControls{
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Computed"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Amount"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Fail"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Empty"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Legacy"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Unsupported"},
};
constexpr agiru::PageDef kPage{
    .id = agiru::PageId{50130}, .name = "Source Page", .layout = kControls};
constexpr std::array kReaders{
    agiru::ControlTrigger<SourcePage>{.control = "Computed", .sourceText = &SourcePage::Read},
    agiru::ControlTrigger<SourcePage>{.control = "Amount", .sourceText = &SourcePage::Amount},
    agiru::ControlTrigger<SourcePage>{.control = "Fail", .sourceText = &SourcePage::Fail},
    agiru::ControlTrigger<SourcePage>{.control = "Empty", .sourceText = &SourcePage::Empty},
    agiru::ControlTrigger<SourcePage>{
        .control = "Legacy",
        .set = [](SourcePage &page, std::string_view value) { page.Legacy(value); },
        .text = [](const SourcePage &page) { return page.Legacy(); }},
};

}

template <> struct agiru::PageTraits<SourcePage> {
  static constexpr agiru::PageId kId{50130};
  static constexpr std::string_view kName = "Source Page";
  static constexpr const agiru::PageDef &kPage = ::kPage;
  static constexpr auto kControlTriggers = kReaders;
  template <typename, typename, template <typename> class> using Controls = ::Controls;
};

static_assert(agiru::PageTraits<SourcePage>::kPage.id == agiru::PageTraits<SourcePage>::kId);
static_assert(agiru::PageTraits<SourcePage>::kPage.name == agiru::PageTraits<SourcePage>::kName);

namespace {

void ReadsUseTheirOwningInstance() {
  agiru::TestPage<SourcePage> first;
  agiru::TestPage<SourcePage> second;
  first.OpenView();
  second.OpenView();
  CHECK_TEXT("computed read reaches the current page", first.ControlText("Computed"), "1");
  CHECK_TEXT("a source is evaluated again on the next read", first.ControlText("Computed"), "2");
  CHECK_TEXT("another instance has its own state", second.ControlText("Computed"), "1");
  const auto &readOnly = first;
  CHECK_TEXT(
      "const harness does not make the AL page const", readOnly.ControlText("Computed"), "3");
  CHECK_TEXT("an empty computed value is supported", first.ControlText("Empty"), "");
  agiru::TestField amount{"Amount"};
  amount.Bind(first);
  CHECK_TEXT("Decimal values do not pass through binary floats",
             amount.AsDecimal().ToInvariantString(),
             "999999999999999.99");
  first.Close();
  first.OpenView();
  CHECK_TEXT("reopening evaluates a new page instance", first.ControlText("Computed"), "1");
  first.Close();
  second.Close();
}

void ErrorsAndLegacySourcesRemainExplicit() {
  agiru::TestPage<SourcePage> page;
  page.OpenEdit();
  CHECK_TEXT("legacy variable reader stays available", page.ControlText("Legacy"), "before");
  page.SetControlText("Legacy", "after");
  CHECK_TEXT("legacy writeback stays available", page.ControlText("Legacy"), "after");
  for (const char *const name : {"Fail", "Unsupported", "Unknown"}) {
    bool raised = false;
    try {
      static_cast<void>(page.ControlText(name));
    } catch (const agiru::Error &error) {
      raised = true;
      if (std::string_view(name) == "Fail") {
        CHECK_TEXT("source errors retain their diagnostic", error.what(), "source failed");
        CHECK_TEXT("source errors retain their code", error.Code(), "SourceFailure");
      }
    }
    CHECK_TRUE("source errors, unsupported sources and unknown controls refuse", raised);
  }
  page.Close();
}

}

int main() {
  return gate::Run("PageSource", [] {
    const agiru::Session session(AGIRU_TEST_DSN);
    ReadsUseTheirOwningInstance();
    ErrorsAndLegacySourcesRemainExplicit();
  });
}
