#pragma once

#include <cstddef>
#include <string>
#include <string_view>

/// \file
/// \brief Bounded semantic HTML over the shared production page command adapter.

namespace agiru {

class PageCore;
class PageAuthorization;
struct PageDef;

/// \brief Server-owned command envelope, not authorization or a session implementation.
/// Handles/revisions/IDs must be checked by the HTTP command receiver. Commands are
/// emitted as forms so browsers and agents discover the same operation contract.
struct PageHtmlContext {
  static constexpr std::size_t kHandleBytes = 128; ///< Version-one opaque-token safety budget.
  std::string_view pageHandle{};                   ///< Opaque authenticated-session page handle.
  std::string_view revision{};      ///< Exact decimal revision text, never a JS number.
  std::string_view commandPrefix{}; ///< Unique receipt-ID prefix for this rendered response.
  std::string_view csrf{};          ///< Server-issued CSRF token; never an implicit confirmation.
  std::string_view commandPath = "/commands"; ///< Same-origin root-relative ASCII endpoint.
};

/// \brief Explicit version-one transport budgets, not BC control-count guarantees.
struct PageHtmlLimits {
  static constexpr std::size_t kDefaultBytes = 1048576; ///< One-MiB protocol safety default.
  static constexpr std::size_t kDefaultControls = 4096; ///< Protocol declaration-walk default.
  static constexpr std::size_t kDefaultDepth = 64;      ///< Protocol nesting safety default.
  std::size_t bytes = kDefaultBytes;       ///< Maximum emitted bytes, including escaped text.
  std::size_t controls = kDefaultControls; ///< Maximum walked declarations, including hidden ones.
  std::size_t depth = kDefaultDepth;       ///< Maximum visited nesting depth.
};

/// \brief HTML and counted unsupported controls; partial is not workflow acceptance.
struct PageHtmlResult {
  std::string html{};          ///< UTF-8 fragment, data-agiru-profile="1".
  std::size_t unsupported = 0; ///< Visible kinds or scalar bindings lacking a renderer.
};

/// \brief Renders the current row without navigation, saves or action execution.
/// Fields read through ReadValue; action discovery uses Inspect, both reauthorized.
/// Every visited node is authorized before evaluating its dynamic visibility.
/// Containers retain declared order; unsupported kinds/bindings remain visible alerts.
/// \param declaration Immutable declaration matching the supplied live page.
/// \param page The shared page control adapter, not a presentation-specific runtime.
/// \param authorization Mandatory user/company authorization for every visible leaf.
/// \param context Server-issued handles and command tokens; not trusted client input.
/// \param limits Bounds for output, walked controls and nesting; excess refuses atomically.
/// \return One owned current-row fragment. List windows, parts and dialogs remain separate.
/// \throws Error for invalid envelopes, unsafe text, exceeded budgets or AL/permission errors.
[[nodiscard]] PageHtmlResult RenderPageHtml(const PageDef &declaration,
                                            PageCore &page,
                                            PageAuthorization &authorization,
                                            const PageHtmlContext &context,
                                            PageHtmlLimits limits = {});

}
