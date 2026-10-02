#include "runtime/RecordRef.h"
#include "type/Integer.h"

namespace agiru {
Integer RecordRef::FilterGroup() const {
  return const_cast<RecordRef *>(this)->FilterGroup(0);
}
}
