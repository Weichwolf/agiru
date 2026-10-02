struct Owner {
  Owner() = default;
  ~Owner() { if (uses != nullptr) { delete uses; } }
  long *uses = nullptr;
};
struct State {
  void *record = nullptr;
  const void *table = nullptr;
  Owner owned;
  long uses = 1;
};
int main() {
  auto *state = new State{};
  if (state->uses != 1 || state->record != nullptr || state->table != nullptr ||
      state->owned.uses != nullptr) { return 1; }
  ++state->uses;
  delete state;
}
