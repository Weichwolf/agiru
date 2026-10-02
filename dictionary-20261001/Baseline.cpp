#include "Check.h"
#include "dotnet/Generic.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"
#include "type/Variant.h"

namespace {

using namespace agiru;

void ExistingAPIControls() {
  dotnet::GenericDictionary2 dictionary;
  dictionary = dictionary.Dictionary();
  dictionary.Add(Variant{Integer{18}}, Integer{1});
  dictionary.Add(Variant{"18"}, Integer{2});
  CHECK_TRUE("Integer and Text keys stay distinct", dictionary.Count() == 2);
  CHECK_TRUE("Integer key retains its value", dictionary.Item(Variant{Integer{18}}) == Variant{1});
  bool raised = false;
  try {
    dictionary.Add(Variant{"18"}, Integer{3});
  } catch (const Error &) {
    raised = true;
  }
  CHECK_TRUE("duplicate Add raises", raised);
  CHECK_TRUE("duplicate Add preserves its prior value", dictionary.Item(Variant{"18"}) == Variant{2});
  auto alias = dictionary;
  alias.Add(Variant{"alias"}, Integer{4});
  CHECK_TRUE("copies alias one store", dictionary.ContainsKey(Variant{"alias"}));
  alias.Remove(Variant{"18"});
  CHECK_TRUE("alias removal is shared", !dictionary.ContainsKey(Variant{"18"}));
  dictionary.Remove(Variant{Integer{18}});
  CHECK_TRUE("Integer removal does not remove Text", !dictionary.ContainsKey(Variant{Integer{18}}));
}

}

int main() { return gate::Run("DictionaryBaseline", ExistingAPIControls); }
