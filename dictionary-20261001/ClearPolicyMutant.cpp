#include "dotnet/Generic.h"

namespace agiru::dotnet {

void GenericDictionary2::Clear() {
  Store &store = Held();
  if (!store.entries.empty()) {
    store.entries.clear();
    ++store.version;
  }
}

}
