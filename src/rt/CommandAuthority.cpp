#include "CommandAuthority.h"

#include "SessionState.h"

namespace agiru::detail {

void CheckCommandAuthority() {
  const auto *state = SessionState::Peek();
  if (state != nullptr && state->commandAuthority != nullptr) { state->commandAuthority->Check(); }
}

void LockCommandCommit(const Connection &connection) {
  const auto *state = SessionState::Peek();
  if (state != nullptr && state->commandAuthority != nullptr) {
    state->commandAuthority->LockCommit(connection);
  }
}

}
