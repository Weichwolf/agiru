#include "runtime/Codeunit.h"

#include "meta/Ids.h"

#include <cstdint>
#include <map>
#include <utility>

namespace agiru::detail {

namespace {

struct Single {
  void *instance;
  void (*free)(void *);
};

std::map<std::int32_t, Single> &Singles() {
  thread_local std::map<std::int32_t, Single> singles;
  return singles;
}

}

void *SingleInstanceOf(CodeunitId id, void *(*make)(), void (*free)(void *)) {
  std::map<std::int32_t, Single> &singles = Singles();
  auto found = singles.find(id.Value());
  if (found == singles.end()) {
    found = singles.emplace(id.Value(), Single{.instance = make(), .free = free}).first;
  }
  return found->second.instance;
}

void ReleaseSingleInstances() {
  std::map<std::int32_t, Single> &singles = Singles();
  std::map<std::int32_t, Single> released;
  released.swap(singles);
  for (auto &[id, single] : released) { single.free(single.instance); }
}

}
