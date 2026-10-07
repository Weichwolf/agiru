#include "runtime/PageHtml.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageCore.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageValue.h"
#include "runtime/SecureToken.h"
#include "type/RecordId.h"

#include "HtmlText.h"
#include "PageListHtml.h"

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

  [[nodiscard]] std::size_t ControlCount() const { return controls_; }

  void Row(const PageListRow &row) {
    if (row.controls > limits_.controls - controls_) {
      throw Error("Page HTML declaration budget exceeded", "PageHtmlLimit");
    }
    controls_ += row.controls;
    Raw(row.presentation.html);
    unsupported_ += row.presentation.unsupported;
  }

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

void FieldHtml(Writer &out, PageDispatcher &dispatcher, const ControlDef &control, bool readOnly) {
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
  if (!readOnly && field.editable && field.enabled) {
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
          std::size_t depth,
          bool readOnly = false) {
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
      Tree(
          out, declaration, page, authorization, dispatcher, control.children, depth + 1, readOnly);
      out.Raw("</section>");
    } else if (control.kind == ControlKind::Field) {
      FieldHtml(out, dispatcher, control, readOnly);
    } else if (control.kind == ControlKind::Action) {
      if (readOnly) {
        out.Unsupported(control.name, "PageListRowAction");
      } else {
        ActionHtml(out, dispatcher, control);
      }
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

const ControlDef *Repeater(Writer &walk,
                           const PageDef &declaration,
                           PageCore &page,
                           PageAuthorization &authorization,
                           std::span<const ControlDef> controls,
                           std::size_t depth) {
  const ControlDef *result = nullptr;
  for (const auto &control : controls) {
    walk.Visit(depth);
    authorization.Require(declaration.id,
                          {.operation = PageControlOperation::Inspect, .control = control.name});
    if (!page.ControlVisible(control.name)) { continue; }
    const ControlDef *found =
        control.kind == ControlKind::Repeater
            ? &control
            : Repeater(walk, declaration, page, authorization, control.children, depth + 1);
    if (found == nullptr) { continue; }
    if (result != nullptr) {
      throw Error("Multiple list repeaters are not qualified", "PageListLayout");
    }
    result = found;
  }
  return result;
}

void ListRows(Writer &out, const PageListView &view) {
  if (view.limit == 0 || view.rows.size() > view.limit || view.rows.size() != view.state.rows) {
    throw Error("Invalid list window presentation", "PageListState");
  }
  out.Visit(0);
  out.Raw(R"(<section data-kind="rows" data-control="$agiru.rows">)");
  for (const auto &row : view.rows) {
    if (!Token(row.handle)) { throw Error("Invalid row handle", "PageListState"); }
    out.Visit(1);
    out.Raw(R"(<section data-kind="row")");
    out.Attribute("data-row", row.handle);
    out.Attribute("data-selected", row.identity == view.selected ? "true" : "false");
    out.Raw("><h2>");
    out.Text(row.identity.ToText());
    out.Raw("</h2>");
    out.Row(row);
    out.Visit(2);
    out.Action("$agiru.row_" + row.handle, "Select row", true);
    out.Raw("</section>");
  }
  out.Raw("</section>");
}

PageHtmlResult Render(const PageDef &declaration,
                      PageCore &page,
                      PageAuthorization &authorization,
                      const PageHtmlContext &context,
                      const PageListView *view,
                      PageHtmlLimits limits) {
  CheckContext(context);
  Writer out(limits, context);
  PageDispatcher dispatcher(declaration, page, authorization);
  out.Raw(view == nullptr ? R"(<article data-agiru-profile="1" data-view="current-row")"
                          : R"(<article data-agiru-profile="2" data-view="list")");
  if (view != nullptr) {
    out.Attribute("data-limit", std::to_string(view->limit));
    out.Attribute("data-more", view->state.more ? "true" : "false");
    out.Attribute("data-direction", view->backwards ? "backward" : "forward");
  }
  out.Attribute("data-page", std::to_string(declaration.id.Value()));
  out.Attribute("data-handle", context.pageHandle);
  out.Attribute("data-revision", context.revision);
  out.Raw("><h1>");
  out.Text(declaration.caption.empty() ? declaration.name : declaration.caption);
  out.Raw("</h1>");
  Tree(out, declaration, page, authorization, dispatcher, declaration.layout, 0);
  if (view != nullptr) { ListRows(out, *view); }
  Tree(out, declaration, page, authorization, dispatcher, declaration.actions, 0);
  HostActions(out, declaration, context);
  out.Raw("<output");
  out.Attribute("data-unsupported-count", std::to_string(out.UnsupportedCount()));
  out.Raw("></output></article>");
  return out.Take();
}

}

PageHtmlResult RenderPageHtml(const PageDef &declaration,
                              PageCore &page,
                              PageAuthorization &authorization,
                              const PageHtmlContext &context,
                              PageHtmlLimits limits) {
  return Render(declaration, page, authorization, context, nullptr, limits);
}

PageHtmlResult RenderPageListHtml(const PageDef &declaration,
                                  PageCore &page,
                                  PageAuthorization &authorization,
                                  const PageHtmlContext &context,
                                  const PageListView &view,
                                  PageHtmlLimits limits) {
  return Render(declaration, page, authorization, context, &view, limits);
}

PageListLoader::PageListLoader(const PageDef &declaration,
                               PageAuthorization &authorization,
                               PageListView &view,
                               PageHtmlLimits limits)
    : declaration_(declaration), authorization_(authorization), view_(view), limits_(limits) {
  for (const auto &row : view_.rows) {
    bytes_ += row.presentation.html.size();
    controls_ += row.controls;
  }
  if (bytes_ > limits_.bytes || controls_ > limits_.controls) {
    throw Error("Page HTML output budget exceeded", "PageHtmlLimit");
  }
}

PageListRow PageListLoader::Capture_(const RecordId &identity, PageCore &controls) {
  const PageHtmlContext unused;
  Writer walk(limits_, unused);
  const auto *repeater =
      Repeater(walk, declaration_, controls, authorization_, declaration_.layout, 0);
  if (repeater == nullptr) { throw Error("The list has no visible repeater", "PageListLayout"); }
  Writer out({.bytes = limits_.bytes - bytes_,
              .controls = limits_.controls - controls_,
              .depth = limits_.depth},
             unused);
  PageDispatcher dispatcher(declaration_, controls, authorization_);
  Tree(out, declaration_, controls, authorization_, dispatcher, repeater->children, 0, true);
  const auto count = out.ControlCount();
  auto row = PageListRow{.handle = GenerateSecureToken(),
                         .identity = identity,
                         .presentation = out.Take(),
                         .controls = count};
  bytes_ += row.presentation.html.size();
  controls_ += row.controls;
  return row;
}

void PageListLoader::Row(const RecordId &identity, PageCore &controls) {
  if (view_.rows.size() >= view_.limit) { throw Error("List row limit exceeded", "PageListLimit"); }
  view_.rows.push_back(Capture_(identity, controls));
}

void PageListLoader::Current(const RecordId &identity, PageCore &controls) {
  view_.selected = identity;
  if (identity.IsEmpty() || view_.rows.empty()) { return; }
  const auto found = std::ranges::find(view_.rows, identity, &PageListRow::identity);
  if (found == view_.rows.end()) {
    throw Error("Selected row is outside the window", "PageListSelection");
  }
  bytes_ -= found->presentation.html.size();
  controls_ -= found->controls;
  auto updated = Capture_(identity, controls);
  updated.handle = found->handle;
  *found = std::move(updated);
}

}
