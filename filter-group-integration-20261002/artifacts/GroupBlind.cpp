#include "runtime/Record.h"
#include "runtime/RecordState.h"

namespace agiru::detail {
bool RuntimeHasFilter(const void *record) {
  const RecordState *state = reinterpret_cast<const StateHandle *>(record)->Peek();
  return state != nullptr && !state->filters.empty();
}
}
