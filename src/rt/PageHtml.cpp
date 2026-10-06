#include "runtime/PageHtml.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageCore.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageValue.h"

#include "HtmlText.h"

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {
namespace {

bool TokenCharacter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
         c == '-';
}

bool Token(std::string_view value) {
  return !value.empty() && value.size() <= PageHtmlContext::kHandleBytes &&
         std::ranges::all_of(value, TokenCharacter);
}

void CheckContext(const PageHtmlContext &context) {
  if (!Token(context.pageHandle) || !Token(context.commandPrefix) || context.csrf.empty() ||
      context.revision.empty() || context.commandPath.empty() || context.commandPath[0] != '/' ||
      context.commandPath.starts_with("//")) {
    throw Error("Invalid page HTML command envelope", "PageHtmlContext");
  }
  for (const char c : context.revision) {
    if (c < '0' || c > '9') { throw Error("Invalid page HTML revision", "PageHtmlContext"); }
  }
  for (const char c : context.commandPath) {
    if (c != '/' && !TokenCharacter(c)) {
      throw Error("Invalid page HTML command path", "PageHtmlContext");
    }
  }
}

class Writer {
public:
  Writer(PageHtmlLimits limits, const PageHtmlContext &context)
      : limits_(limits), context_(context) {}

  void Raw(std::string_view text) {
    if (text.size() > limits_.bytes - output_.size()) {
      throw Error("Page HTML output budget exceeded", "PageHtmlLimit");
    }
    output_ += text;
  }

  void Text(std::string_view text) { detail::AppendHtmlText(output_, text, limits_.bytes); }

  void Attribute(std::string_view name, std::string_view value) {
    Raw(" ");
    Raw(name);
    Raw("=\"");
    Text(value);
    Raw("\"");
  }

  void Visit(std::size_t depth) {
    if (++controls_ > limits_.controls || depth > limits_.depth) {
      throw Error("Page HTML declaration budget exceeded", "PageHtmlLimit");
    }
  }

  std::string ContainerIdentity(const ControlDef &control) const {
    return control.name.empty() ? "$agiru.container." + std::to_string(controls_)
                                : std::string(control.name);
  }

  void Input(std::string_view name, std::string_view value) {
    Raw("<input type=\"hidden\"");
    Attribute("name", name);
    Attribute("value", value);
    Raw(">");
  }

  void Form(std::string_view operation, std::string_view control) {
    Raw("<form method=\"post\"");
    Attribute("action", context_.commandPath);
    Attribute("hx-post", context_.commandPath);
    Raw(R"( hx-target="closest article" hx-swap="outerHTML">)");
    Input("page", context_.pageHandle);
    Input("revision", context_.revision);
    Input("command", std::string(context_.commandPrefix) + "_" + std::to_string(controls_));
    Input("csrf", context_.csrf);
    Input("operation", operation);
    Input("control", control);
  }

  void Action(std::string_view identity, std::string_view caption, bool enabled) {
    Raw("<section data-kind=\"action\"");
    Attribute("data-control", identity);
    Raw(">");
    Form("action", identity);
    Raw("<button type=\"submit\"");
    if (!enabled) { Raw(" disabled"); }
    Raw(">");
    Text(caption);
    Raw("</button></form></section>");
  }

  void Unsupported(std::string_view control, std::string_view reason) {
    ++unsupported_;
    Raw("<aside role=\"alert\"");
    Attribute("data-control", control);
    Attribute("data-unsupported", reason);
    Raw(">Unsupported: ");
    Text(control);
    Raw("</aside>");
  }

  [[nodiscard]] PageHtmlResult Take() {
    return {.html = std::move(output_), .unsupported = unsupported_};
  }

  [[nodiscard]] std::size_t UnsupportedCount() const { return unsupported_; }

private:
  PageHtmlLimits limits_;
  const PageHtmlContext &context_;
  std::string output_;
  std::size_t controls_ = 0;
  std::size_t unsupported_ = 0;
};

void ValueAttributes(Writer &out, const PageValue &value) {
  out.Attribute("data-type", value.type);
  out.Attribute("data-value", value.value);
  out.Attribute("data-domain", value.domain);
  out.Attribute("data-member", value.member);
  out.Attribute("data-undefined", value.undefined ? "true" : "false");
  out.Attribute("data-closing", value.closing ? "true" : "false");
}

void FieldHtml(Writer &out, PageDispatcher &dispatcher, const ControlDef &control) {
  PageControlResult field;
  try {
    field =
        dispatcher.Execute({.operation = PageControlOperation::ReadValue, .control = control.name});
  } catch (const Error &error) {
    if (error.Code() != "PageValueUnsupported") { throw; }
    out.Unsupported(control.name, "PageValueUnsupported");
    return;
  }
  out.Raw("<section");
  out.Attribute("data-control", control.name);
  out.Raw(" data-kind=\"field\">");
  out.Raw("<h3>");
  out.Text(field.caption);
  out.Raw("</h3>");
  if (field.editable && field.enabled) {
    out.Form("set", control.name);
    out.Raw(R"(<input name="text" type="text")");
    out.Attribute("aria-label", field.caption);
    out.Attribute("value", field.text);
    ValueAttributes(out, field.value);
    out.Raw("><button type=\"submit\">Set</button></form>");
  } else {
    out.Raw("<output");
    ValueAttributes(out, field.value);
    out.Raw(">");
    out.Text(field.text);
    out.Raw("</output>");
  }
  out.Raw("</section>");
}

void ActionHtml(Writer &out, PageDispatcher &dispatcher, const ControlDef &control) {
  const auto action =
      dispatcher.Execute({.operation = PageControlOperation::Inspect, .control = control.name});
  out.Action(control.name, action.caption, action.enabled);
}

void HostActions(Writer &out, const PageDef &declaration, const PageHtmlContext &context) {
  for (const auto &action : context.actions) {
    if (action.identity.empty() ||
        std::ranges::count(context.actions, action.identity, &PageHtmlAction::identity) != 1 ||
        Control(declaration.layout, action.identity) != nullptr ||
        Control(declaration.actions, action.identity) != nullptr) {
      throw Error("Host action collides with an AL control", "PageHtmlContext");
    }
    out.Visit(0);
    out.Action(action.identity, action.caption, action.enabled);
  }
}

void Tree(Writer &out,
          const PageDef &declaration,
          PageCore &page,
          PageAuthorization &authorization,
          PageDispatcher &dispatcher,
          std::span<const ControlDef> controls,
          std::size_t depth) {
  for (const ControlDef &control : controls) {
    out.Visit(depth);
    const PageControlCommand presentation{.operation = PageControlOperation::Inspect,
                                          .control = control.name};
    authorization.Require(declaration.id, presentation);
    if (!page.ControlVisible(control.name)) { continue; }
    if (Container(control.kind)) {
      out.Raw("<section data-kind=\"group\"");
      out.Attribute("data-control", out.ContainerIdentity(control));
      out.Raw(">");
      out.Raw("<h2>");
      out.Text(control.caption);
      out.Raw("</h2>");
      Tree(out, declaration, page, authorization, dispatcher, control.children, depth + 1);
      out.Raw("</section>");
    } else if (control.kind == ControlKind::Field) {
      FieldHtml(out, dispatcher, control);
    } else if (control.kind == ControlKind::Action) {
      ActionHtml(out, dispatcher, control);
    } else {
      if (control.kind == ControlKind::Label || control.kind == ControlKind::Separator) {
        out.Raw("<p");
        out.Attribute("data-control", control.name);
        out.Raw(">");
        out.Text(control.caption);
        out.Raw("</p>");
      } else {
        out.Unsupported(control.name, "ControlKind");
      }
    }
  }
}

}

PageHtmlResult RenderPageHtml(const PageDef &declaration,
                              PageCore &page,
                              PageAuthorization &authorization,
                              const PageHtmlContext &context,
                              PageHtmlLimits limits) {
  CheckContext(context);
  Writer out(limits, context);
  PageDispatcher dispatcher(declaration, page, authorization);
  out.Raw(R"(<article data-agiru-profile="1" data-view="current-row")");
  out.Attribute("data-page", std::to_string(declaration.id.Value()));
  out.Attribute("data-handle", context.pageHandle);
  out.Attribute("data-revision", context.revision);
  out.Raw("><h1>");
  out.Text(declaration.caption.empty() ? declaration.name : declaration.caption);
  out.Raw("</h1>");
  Tree(out, declaration, page, authorization, dispatcher, declaration.layout, 0);
  Tree(out, declaration, page, authorization, dispatcher, declaration.actions, 0);
  HostActions(out, declaration, context);
  out.Raw("<output");
  out.Attribute("data-unsupported-count", std::to_string(out.UnsupportedCount()));
  out.Raw("></output>");
  out.Raw("</article>");
  return out.Take();
}

}
