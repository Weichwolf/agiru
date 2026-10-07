#pragma once

#include "meta/Ids.h"
#include "type/CommitBehavior.h"
#include "type/ErrorBehavior.h"

#include "SessionRandom.h"

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace agiru {
class SubscriptionCatalogue;
class Session;
class UiHost;
}

namespace agiru::detail {

class RecordChanges;

struct SessionState {
  SessionState();
  ~SessionState();
  std::atomic_flag commandActive = ATOMIC_FLAG_INIT;
  std::unique_ptr<UiHost> uiHost;
  std::string applicationArea;
  std::unique_ptr<SessionRandom> random;

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
  std::shared_ptr<RecordChanges> recordChanges;
  std::vector<CommitBehavior> commits;
  std::vector<ErrorBehavior> errors;
  std::vector<std::string> collectedErrors;

  static SessionState &Current();
  static SessionState &For(Session &session);
  static SessionState *Peek();
  static void ReleaseBindings(CodeunitId id, void *instance) noexcept;
  void ReleaseSingles();
};

}
