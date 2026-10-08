#pragma once

#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <memory>
#include <span>
#include <string_view>

/// \file
/// \brief Session-owned dialog transport; never an AL or HTTP capability flag.
namespace agiru {

class Session;
class PageInstance;

/// \brief A live progress variable, read only on its owning AL execution worker.
/// The host may retain this binding until the dialog/object closes, not after its AL
/// stack unwinds. Client output contains copied typed values, never addresses or callbacks.
struct UiValueBinding {
  const void *value;             ///< Borrowed AL variable, whose lifetime encloses the open dialog.
  Variant (*read)(const void *); ///< Exact typed snapshot without mutating the variable.

  /// \brief Bind a progress variable without replacing it with its initial value.
  /// \tparam Value The AL variable type. \param value The live variable.
  /// \return A non-owning binding; its read callback preserves the AL type.
  template <typename Value> static UiValueBinding Bind(Value &value) {
    return {&value, +[](const void *held) { return Variant(*static_cast<const Value *>(held)); }};
  }
};

/// \brief A working dialog endpoint owned exclusively by one session.
/// Calls execute on the owning AL worker. Questions return explicit client answers,
/// never defaults; an unavailable or cancelled transport must throw and unwind.
/// Message delivery is deferred until execution completes or pauses for interaction.
/// Transport implementations own HTTP admission and modal suspension, not this interface.
class UiHost {
public:
  virtual ~UiHost() = default;

  /// \brief Queue a copied message; do not display it immediately inside AL execution.
  /// \param text Formatted Unicode text. \throws Error for unavailable/bounded transport.
  virtual void QueueMessage(std::string_view text) = 0;

  /// \brief Await an explicit yes/no answer without committing or replaying AL execution.
  /// \param text Formatted question. \param defaultButton Presentation only, not consent.
  /// \return The explicit answer. \throws Error on cancellation or transport failure.
  virtual Boolean Confirm(std::string_view text, Boolean defaultButton) = 0;

  /// \brief Await explicit menu selection; zero means cancel.
  /// \param options AL comma-separated options. \param defaultChoice Presentation only.
  /// \param instruction Unicode instruction. \return Explicit one-based selection or zero.
  /// \throws Error for transport failure; no implicit/default selection is allowed.
  virtual Integer
  StrMenu(std::string_view options, Integer defaultChoice, std::string_view instruction) = 0;

  /// \brief Runs the original borrowed AL page modally without replay or automatic closure.
  /// \param page Closed, prepared adapter; owned by the waiting AL caller.
  /// \return Explicit normalized close action; the page variable remains usable by AL.
  /// \throws Error for an unavailable transport, cancellation or unqualified native capability.
  /// \note The host authorizes before opening and every operation, runs AL only on its owner
  /// worker, restricts input to the active modal and preserves the caller transaction/stack.
  virtual Action RunModal(PageInstance &page);

  /// \brief Open a progress window, retaining live variable bindings until closure.
  /// \param owner Private live-dialog identity; never expose its address to a client.
  /// \param text Original AL mask. \param values Ordered live variable bindings.
  virtual void OpenProgress(const void *owner,
                            std::string_view text,
                            std::span<const UiValueBinding> values) = 0;

  /// \brief Refresh bound fields, or replace one field with the supplied exact typed value.
  /// \param owner Private dialog identity. \param number Zero updates all bound variables.
  /// \param value Empty refreshes a binding; a nonempty value is an explicit replacement.
  virtual void UpdateProgress(const void *owner, Integer number, const Variant &value) = 0;

  /// \brief Close one progress window; object/command teardown must close remaining windows.
  /// \param owner Private live-dialog identity.
  virtual void CloseProgress(const void *owner) = 0;
};

/// \brief Install or remove the trusted host's actual dialog transport on an idle session.
/// \param session The exclusive owner, including a detached authenticated session.
/// \param host Owned endpoint, or null to detach it. Never inferred from authentication.
/// \throws SessionError while a command or transaction boundary is active.
void InstallUiHost(Session &session, std::unique_ptr<UiHost> host);

/// \brief The active session's actual host, or null without one; never a thread-global host.
/// \return A borrowed pointer valid only while the owning session/host remains alive.
[[nodiscard]] UiHost *CurrentUiHost();

namespace detail {
/// \brief Enforces the active session's trusted callback-in-write-transaction policy.
/// Applies before native interaction or an explicit AL test handler; does not commit,
/// roll back or detach the session. Without an active session the default enabled policy applies.
/// \throws Error with UiWriteTransaction when callbacks are disabled in a write transaction.
void RequireUiCallback();

/// \brief Route live progress bindings to the owning host; AL tests remain headless.
/// \param owner Live dialog identity. \param text AL mask. \param values Live bindings.
/// \throws Error when neither an interactive host nor the explicit AL test adapter exists.
void OpenUiProgress(const void *owner,
                    std::string_view text,
                    std::span<const UiValueBinding> values);
}

}
