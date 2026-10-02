#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "type/Integer.h"

namespace agiru::detail {
Integer RuntimeFilterGroup(void *record, Integer group) {
  RecordState &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  const Integer was = state.group;
  state.group = group;
  return was;
}
}
