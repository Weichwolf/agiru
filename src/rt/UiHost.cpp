#include "runtime/UiHost.h"

#include "runtime/Session.h"
#include "runtime/test/Handlers.h"

#include "SessionState.h"

#include <memory>
#include <span>
#include <string_view>
#include <utility>

namespace agiru {

void InstallUiHost(Session &session, std::unique_ptr<UiHost> host) {
  auto &state = detail::SessionState::For(session);
  if (state.commandActive.test() || session.Transaction().Depth() != 0) {
    throw SessionError("UI host installation requires an idle session");
  }
  state.uiHost = std::move(host);
}

UiHost *CurrentUiHost() {
  const auto *state = detail::SessionState::Peek();
  return state == nullptr ? nullptr : state->uiHost.get();
}

void detail::RequireUiCallback() {
  if (!Session::HasCurrent()) { return; }
  const auto &session = Session::Current();
  if (!session.Options().allowSessionCallSuspendWhenWriteTransactionStarted &&
      session.Transaction().IsWriting()) {
    throw Error("Client callbacks during a write transaction are disabled by runtime configuration",
                "UiWriteTransaction");
  }
}

void detail::OpenUiProgress(const void *owner,
                            std::string_view text,
                            std::span<const UiValueBinding> values) {
  if (HandlerTable::Installed()) { return; }
  if (auto *host = CurrentUiHost(); host != nullptr) {
    host->OpenProgress(owner, text, values);
    return;
  }
  throw Error("Dialog.Open requires an interactive UI host", "UiHostUnavailable");
}

}
