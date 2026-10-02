#pragma once

#include "meta/Ids.h"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace agiru {
class SubscriptionCatalogue;
}

namespace agiru::detail {

struct SessionState {
  struct Binding {
    CodeunitId id;
    const SubscriptionCatalogue *catalogue;
    void *instance;
    std::uint64_t generation;
  };

  using OwnedInstance = std::unique_ptr<void, void (*)(void *)>;
  std::map<CodeunitId, OwnedInstance> singles;
  std::vector<Binding> bindings;
  std::uint64_t lastBinding = 0;

  static SessionState &Current();
  static SessionState *Peek();
  static void ReleaseBindings(CodeunitId id, void *instance) noexcept;
  void ReleaseSingles();
};

}
