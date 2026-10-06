#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageCore.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageHtml.h"
#include "runtime/PageValue.h"
#include "type/Boolean.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <string>
#include <string_view>

namespace {

constexpr std::array kChildren{
    agiru::ControlDef{
        .kind = agiru::ControlKind::Field, .name = "Amount", .caption = "Not the identity"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Big"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Hidden"},
    agiru::ControlDef{.kind = agiru::ControlKind::Field, .name = "Computed"},
    agiru::ControlDef{.kind = agiru::ControlKind::Part, .name = "Lines"},
};
constexpr std::array kLayout{agiru::ControlDef{.kind = agiru::ControlKind::Group,
                                               .name = "General",
                                               .caption = "Grüezi & <data>",
                                               .children = kChildren}};
constexpr std::array kActions{
    agiru::ControlDef{.kind = agiru::ControlKind::Action, .name = "Post"},
    agiru::ControlDef{.kind = agiru::ControlKind::Action, .name = "Disabled"},
};
constexpr agiru::PageDef kPage{
    .id = agiru::PageId{50400}, .name = "HTML <fixture>", .layout = kLayout, .actions = kActions};
constexpr agiru::PageHtmlContext kContext{.pageHandle = "page_1",
                                          .revision = "9007199254740993",
                                          .commandPrefix = "cmd_1",
                                          .csrf = "test-csrf"};

class Page final : public agiru::PageCore {
public:
  void SetControlText(std::string_view /*control*/, std::string_view /*text*/) override {
    ++writes;
  }

  std::string ControlText(std::string_view control) const override {
    return control == "Big" ? "caption, not numeric" : text;
  }

  std::string ControlOrdinal(std::string_view /*control*/) const override { return {}; }

  agiru::PageValue Control_Value(std::string_view control) const override {
    if (control == "Computed") { return PageCore::Control_Value(control); }
    if (control == "Big") { return {.type = "BigInteger", .value = "9223372036854775807"}; }
    return {.type = "Decimal", .value = "1.2300"};
  }

  void RunControlTrigger(std::string_view /*control*/,
                         agiru::ControlTriggerKind /*kind*/) override {
    ++writes;
  }

  void SetControlFilter(std::string_view /*control*/, std::string_view /*filter*/) override {
    ++writes;
  }

  std::string ControlFilterText(std::string_view /*control*/) const override { return {}; }

  agiru::Boolean ControlVisible(std::string_view control) const override {
    ++visibilityReads;
    return control != "Hidden";
  }

  agiru::Boolean ControlEditable(std::string_view control) const override {
    return editable && control == "Amount";
  }

  agiru::Boolean ControlEnabled(std::string_view control) const override {
    return control != "Disabled";
  }

  std::string ControlCaption(std::string_view control) const override {
    return "Caption " + std::string(control);
  }

  void *PartInstance(std::string_view /*control*/) override { return nullptr; }

  void LinkPart(std::string_view /*control*/,
                void * /*subRecord*/,
                const agiru::TableDef & /*subTable*/) override {
    ++writes;
  }

  void RowLeft() override { ++writes; }

  void AttachPart(agiru::PageCore & /*part*/) override { ++writes; }

  int writes = 0;
  mutable int visibilityReads = 0;
  bool editable = true;
  std::string text = "Grüezi <script> & \"quoted\"\r\n";
};

class Authorization final : public agiru::PageAuthorization {
public:
  void Require(agiru::PageId id, const agiru::PageControlCommand &command) override {
    if (verify) {
      CHECK_TRUE("HTML uses the declaration's page identity", id == kPage.id);
      CHECK_TRUE("rendering only requests reads/discovery",
                 command.operation == agiru::PageControlOperation::ReadValue ||
                     command.operation == agiru::PageControlOperation::Inspect);
    }
    ++calls;
    if (revoked) { throw agiru::Error("revoked", "PermissionDenied"); }
  }

  int calls = 0;
  bool revoked = false;
  bool verify = true;
};

void Semantics() {
  Page page;
  Authorization auth;
  const auto result = agiru::RenderPageHtml(kPage, page, auth, kContext);
  const auto &html = result.html;
  CHECK_TRUE("profile declares its current-row boundary",
             html.starts_with("<article data-agiru-profile=\"1\" data-view=\"current-row\""));
  CHECK_TRUE("revision is emitted without JS numeric conversion",
             html.contains("data-revision=\"9007199254740993\""));
  CHECK_TRUE("display markup is escaped as business data",
             html.contains("Grüezi &lt;script&gt; &amp; &quot;quoted&quot;&#13;&#10;"));
  CHECK_TRUE("page captions are escaped", html.contains("HTML &lt;fixture&gt;"));
  CHECK_TRUE("canonical Decimal is separate from display",
             html.contains("data-type=\"Decimal\" data-value=\"1.2300\""));
  CHECK_TRUE("canonical Int64 does not parse the nonnumeric display",
             html.contains("data-type=\"BigInteger\" data-value=\"9223372036854775807\""));
  CHECK_TRUE("commands use declared identities, not captions",
             html.contains("name=\"control\" value=\"Amount\""));
  CHECK_TRUE("commands carry page/revision/receipt/CSRF tokens",
             html.contains("name=\"page\" value=\"page_1\"") &&
                 html.contains("name=\"csrf\" value=\"test-csrf\"") &&
                 html.contains("name=\"command\" value=\"cmd_1_2\""));
  CHECK_TRUE("web and agent discover the same same-origin command forms",
             html.contains("action=\"/commands\" hx-post=\"/commands\""));
  CHECK_TRUE("disabled action remains visible but cannot be submitted normally",
             html.contains("<button type=\"submit\" disabled>Caption Disabled"));
  CHECK_TRUE("hidden control and value are absent", !html.contains("Hidden"));
  CHECK_TRUE("declared sibling order is stable",
             html.find("data-control=\"Amount\"") < html.find("data-control=\"Big\""));
  CHECK_TRUE("unsupported scalar and part remain counted alerts",
             result.unsupported == 2 && html.contains("data-unsupported-count=\"2\"") &&
                 html.contains("data-unsupported=\"PageValueUnsupported\"") &&
                 html.contains("data-unsupported=\"ControlKind\""));
  CHECK_TRUE("presentation state and every visible leaf are reauthorized", auth.calls == 13);
  CHECK_TRUE("rendering never saves/navigates/executes a trigger", page.writes == 0);
  page.editable = false;
  const auto view = agiru::RenderPageHtml(kPage, page, auth, kContext);
  CHECK_TRUE("view mode has no field edit form",
             !view.html.contains("name=\"operation\" value=\"set\""));
  CHECK_TRUE("the exact response byte boundary is accepted",
             agiru::RenderPageHtml(kPage, page, auth, kContext, {.bytes = view.html.size()}).html ==
                 view.html);
  bool refused = false;
  try {
    static_cast<void>(
        agiru::RenderPageHtml(kPage, page, auth, kContext, {.bytes = view.html.size() - 1}));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageHtmlLimit"; }
  CHECK_TRUE("one missing output byte refuses instead of overflowing the envelope", refused);
}

void Refusals() {
  Page page;
  Authorization auth;
  auth.revoked = true;
  bool refused = false;
  try {
    static_cast<void>(agiru::RenderPageHtml(kPage, page, auth, kContext));
  } catch (const agiru::Error &error) { refused = error.Code() == "PermissionDenied"; }
  CHECK_TRUE("permission refusal is not downgraded to an unsupported field", refused);
  CHECK_TRUE("revoked permissions refuse before dynamic visibility executes",
             page.visibilityReads == 0);
  auth.revoked = false;
  for (const agiru::PageHtmlLimits limits : {agiru::PageHtmlLimits{.bytes = 10},
                                             agiru::PageHtmlLimits{.controls = 1},
                                             agiru::PageHtmlLimits{.depth = 0}}) {
    refused = false;
    try {
      static_cast<void>(agiru::RenderPageHtml(kPage, page, auth, kContext, limits));
    } catch (const agiru::Error &error) { refused = error.Code() == "PageHtmlLimit"; }
    CHECK_TRUE("budget excess refuses instead of silently truncating values", refused);
  }
  for (const std::string &text :
       {std::string("bad\0data", 8), std::string("bad\x1b[31m"), std::string("bad\xC0\xAF")}) {
    page.text = text;
    refused = false;
    try {
      static_cast<void>(agiru::RenderPageHtml(kPage, page, auth, kContext));
    } catch (const agiru::Error &error) { refused = error.Code() == "PageHtmlText"; }
    CHECK_TRUE("unsafe or malformed text is never normalized into a different value", refused);
  }
  auto context = kContext;
  context.commandPath = "//attacker/commands";
  refused = false;
  try {
    static_cast<void>(agiru::RenderPageHtml(kPage, page, auth, context));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageHtmlContext"; }
  CHECK_TRUE("protocol-relative command endpoint refuses", refused);
  context = kContext;
  context.revision = "1.0";
  refused = false;
  try {
    static_cast<void>(agiru::RenderPageHtml(kPage, page, auth, context));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageHtmlContext"; }
  CHECK_TRUE("revision has an exact decimal-integer representation", refused);
  CHECK_TRUE("refusal does not execute commands", page.writes == 0);
}

}

int main(int argc, char **argv) {
  if (argc == 2 && std::string_view(argv[1]) == "--html") {
    try {
      Page page;
      Authorization auth;
      auth.verify = false;
      const auto result = agiru::RenderPageHtml(kPage, page, auth, kContext);
      return static_cast<int>(std::fwrite(result.html.data(), 1, result.html.size(), stdout) !=
                              result.html.size());
    } catch (const std::exception &error) {
      std::fputs("PageHtml fixture: ", stderr);
      std::fputs(error.what(), stderr);
      std::fputs("\n", stderr);
      return 1;
    } catch (...) {
      std::fputs("PageHtml fixture: unknown exception\n", stderr);
      return 1;
    }
  }
  if (argc != 1) { return 2; }
  return gate::Run("PageHtml", [] {
    Semantics();
    Refusals();
  });
}
