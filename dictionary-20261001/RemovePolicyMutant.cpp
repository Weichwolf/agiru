#include "dotnet/Generic.h"

namespace agiru::dotnet {

Boolean GenericDictionary2::Remove(const Variant &key) {
  Store &store = Held();
  if (store.entries.erase(KeyOf(key)) == 0) { return false; }
  ++store.version;
  return true;
}

}
