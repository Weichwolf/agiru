#pragma once

#include "meta/Ids.h"

#include <map>
#include <memory>

namespace agiru::detail {

struct SessionState {
  using OwnedInstance = std::unique_ptr<void, void (*)(void *)>;
  std::map<CodeunitId, OwnedInstance> singles;

  static SessionState &Current();
  static SessionState *Peek();
  void ReleaseSingles();
};

}
