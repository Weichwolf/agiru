#include "runtime/RecordRef.h"
void Probe() {
  auto *state = new agiru::detail::RecordRefState{};
  ++state->uses;
  state->owned.Reset();
  delete state;
}
