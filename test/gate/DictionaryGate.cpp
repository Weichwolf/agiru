#include "dotnet/Generic.h"
#include "dotnet/Refused.h"
#include "runtime/ErrorValue.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace {

using namespace agiru;
using dotnet::GenericDictionary2;

constexpr Integer kOriginalValue = 11;
constexpr Integer kConflictingValue = 99;
constexpr Integer kCustomerTableId = 18;
constexpr Integer kItemTableId = 27;
constexpr std::size_t kSupportedKeyTypes = 6;

template <typename Body> bool Raises(Body body) {
  try {
    body();
  } catch (const Error &) { return true; }
  return false;
}

static_assert(
    std::same_as<decltype(std::declval<GenericDictionary2::Entry>().Key()), const Variant &>);
static_assert(!std::is_invocable_v<GenericDictionary2::Binder, const std::string &>);
static_assert(!std::is_invocable_v<GenericDictionary2::Binder, Integer, Integer>);

void TypedKeysStayDistinct() {
  GenericDictionary2 dictionary;
  dictionary = dictionary.Dictionary(static_cast<Integer>(kSupportedKeyTypes));
  const std::array<Variant, kSupportedKeyTypes> keys{Variant{Boolean{true}},
                                                     Variant{Integer{1}},
                                                     Variant{BigInteger{1}},
                                                     Variant{Decimal::FromInvariantString("1.0")},
                                                     Variant{"1"},
                                                     Variant{Guid{}}};
  for (std::size_t at = 0; at < keys.size(); ++at) {
    dictionary.Add(keys[at], Integer{static_cast<Integer>(at)});
  }
  CHECK_TRUE("boxed scalar types do not collapse",
             dictionary.Count() == static_cast<Integer>(keys.size()));
  for (std::size_t at = 0; at < keys.size(); ++at) {
    CHECK_TRUE("typed key remains reachable", dictionary.ContainsKey(keys[at]));
    CHECK_TRUE("each typed key returns its own value",
               dictionary.Item(keys[at]).Get<Integer>() == static_cast<Integer>(at));
  }
  CHECK_TRUE("equal decimals retain CLR numeric equality",
             dictionary.ContainsKey(Decimal::FromInvariantString("1.000")));
  CHECK_TRUE("duplicate scale variants are duplicates", Raises([&] {
               dictionary.Add(Decimal::FromInvariantString("1.00"), kConflictingValue);
             }));
  CHECK_TRUE("duplicate Add leaves the prior value intact",
             dictionary.Item(Decimal{1}).Get<Integer>() == 3);
  CHECK_TRUE("typed remove finds the Integer key", dictionary.Remove(Integer{1}));
  CHECK_TRUE("typed remove does not erase the Text key", dictionary.ContainsKey("1"));
  CHECK_TRUE("typed remove does not erase the BigInteger key",
             dictionary.ContainsKey(BigInteger{1}));
  CHECK_TRUE("a repeated remove reports absence", !dictionary.Remove(Integer{1}));
}

void ReferenceAliasesShareOneStore() {
  GenericDictionary2 dictionary;
  dictionary = dictionary.Dictionary();
  dictionary.Add("source", Integer{1});
  GenericDictionary2 alias = dictionary;
  alias.Add("alias", Integer{2});
  CHECK_TRUE("copy aliases the dictionary", dictionary.ContainsKey("alias"));
  dictionary.Remove("source");
  CHECK_TRUE("removal is shared", !alias.ContainsKey("source"));
  dictionary = dictionary.Dictionary();
  CHECK_TRUE("construction detaches only its variable", dictionary.Count() == 0);
  CHECK_TRUE("old alias retains its store", alias.Count() == 1);
  alias.Clear();
  CHECK_TRUE("Clear empties the shared store", alias.Count() == 0);
  GenericDictionary2 other = dictionary;
  dictionary.Add("later", Boolean{false});
  other.Clear();
  CHECK_TRUE("Clear through an alias affects the owner", dictionary.Count() == 0);
}

void ErrorsDoNotInventValues() {
  GenericDictionary2 dictionary;
  CHECK_TRUE("unassigned .NET reference is null", dictionary.IsNull());
  CHECK_TRUE("null reference operations raise", Raises([&] { (void)dictionary.Count(); }));
  dictionary = dictionary.Dictionary(0);
  CHECK_TRUE("constructed dictionary is not null", !dictionary.IsNull());
  CHECK_TRUE("negative capacity raises", Raises([&] { (void)dictionary.Dictionary(-1); }));
  CHECK_TRUE("null key raises", Raises([&] { dictionary.Add(Variant{}, Integer{1}); }));
  CHECK_TRUE("null ContainsKey raises", Raises([&] { (void)dictionary.ContainsKey(Variant{}); }));
  CHECK_TRUE("unsupported CLR box refuses explicitly",
             Raises([&] { dictionary.Add(Date::FromYmd(2026, 10, 1), Integer{1}); }));
  CHECK_TRUE("refusal inserts no entry", dictionary.Count() == 0);
  dictionary.Add("Key", kOriginalValue);
  CHECK_TRUE("string equality is ordinal and case-sensitive", !dictionary.ContainsKey("key"));
  CHECK_TRUE("missing Item raises", Raises([&] { (void)dictionary.Item("missing"); }));
  CHECK_TRUE("duplicate Add raises", Raises([&] { dictionary.Add("Key", Integer{12}); }));
  CHECK_TRUE("duplicate Add does not replace",
             dictionary.Item("Key").Get<Integer>() == kOriginalValue);
  Variant result{kConflictingValue};
  CHECK_TRUE("TryGetValue reports absence", !dictionary.TryGetValue("missing", result));
  CHECK_TRUE("TryGetValue clears out on absence", result.Is<std::monostate>());
  CHECK_TRUE("TryGetValue finds a value", dictionary.TryGetValue("Key", result));
  CHECK_TRUE("TryGetValue writes through out", result.Get<Integer>() == kOriginalValue);
  result = "Key";
  CHECK_TRUE("key and out may alias", dictionary.TryGetValue(result, result));
  CHECK_TRUE("lookup happens before overwriting its key", result.Get<Integer>() == kOriginalValue);
  dictionary.Add("null-value", Variant{});
  CHECK_TRUE("a null value is present", dictionary.TryGetValue("null-value", result));
  CHECK_TRUE("null value stays null", result.Is<std::monostate>());
}

void EntriesRetainTypedSnapshotsAndOwnership() {
  GenericDictionary2 dictionary;
  dictionary = dictionary.Dictionary();
  dictionary.Add(kCustomerTableId, "SORTING(No.)");
  dictionary.Add(kItemTableId, "WHERE(Blocked=CONST(No))");
  Integer seen = 0;
  for (const auto &entry : dictionary) {
    CHECK_TRUE("foreach emits a typed Integer key", entry.Key().IsInteger());
    CHECK_TRUE("foreach emits the corresponding value",
               entry.Value() == dictionary.Item(entry.Key()));
    ++seen;
  }
  CHECK_TRUE("all entries are enumerated", seen == 2);
  const auto snapshot = *dictionary.begin();
  const Integer key = snapshot.Key().Get<Integer>();
  dictionary.Remove(key);
  CHECK_TRUE("KeyValuePair owns its key after removal", snapshot.Key().Get<Integer>() == key);
  CHECK_TRUE("KeyValuePair owns its value after removal", snapshot.Value().IsText());
  auto retained = dictionary.begin();
  dictionary = {};
  CHECK_TRUE("iterator retains the object after its variable clears",
             (*retained).Key().IsInteger());
  GenericDictionary2 alias;
  alias = alias.Dictionary();
  alias.Add("before", Integer{1});
  auto invalidated = alias.begin();
  alias.Add("after", Integer{2});
  CHECK_TRUE("Add invalidates an active iterator", Raises([&] { (void)*invalidated; }));
  auto erased = alias.begin();
  const auto removed = *erased;
  alias.Remove(removed.Key());
  CHECK_TRUE("Remove preserves the current pair snapshot", (*erased).Key() == removed.Key());
  ++erased;
  CHECK_TRUE("Remove permits advancing without touching freed nodes", erased != alias.end());
  CHECK_TRUE("the next remaining entry remains reachable", alias.ContainsKey((*erased).Key()));
  const auto remaining = *erased;
  alias.Clear();
  CHECK_TRUE("Clear preserves the current pair snapshot", (*erased).Key() == remaining.Key());
  ++erased;
  CHECK_TRUE("Clear allows reaching the end", erased == alias.end());
  CHECK_TRUE("empty dictionary has an empty range", alias.begin() == alias.end());
  CHECK_TRUE("past-end dereference raises", Raises([&] { (void)*alias.end(); }));
}

void PairConstructionPreservesValuesAndRefusals() {
  dotnet::GenericKeyValuePair2 pair;
  constexpr auto construct = dotnet::GenericKeyValuePair2::KeyValuePair;
  Variant value{"first"};
  pair = construct(kCustomerTableId, value);
  value = "changed";
  CHECK_TRUE("AL constructor preserves a typed key", pair.Key().Get<Integer>() == kCustomerTableId);
  CHECK_TRUE("AL constructor preserves a value snapshot", pair.Value() == Variant{"first"});
  const auto copy = pair;
  pair = construct(Boolean{true}, Variant{});
  CHECK_TRUE("pair reassignment does not mutate a copied value", copy.Value() == Variant{"first"});
  CHECK_TRUE("pair constructor keeps Boolean keys", pair.Key().IsBoolean());
  CHECK_TRUE("pair constructor permits null values", pair.Value().Is<std::monostate>());
  const dotnet::AbsentType missing;
  CHECK_TRUE("unbuilt CLR values refuse at the constructor",
             Raises([&] { (void)construct(Integer{1}, missing); }));
  CHECK_TRUE("unbuilt CLR keys refuse at the constructor",
             Raises([&] { (void)construct(missing, Integer{1}); }));
  CHECK_TRUE("failed pair construction preserves its prior value", pair.Key().IsBoolean());
}

}

int main() {
  return gate::Run("Dictionary", [] {
    TypedKeysStayDistinct();
    ReferenceAliasesShareOneStore();
    ErrorsDoNotInventValues();
    EntriesRetainTypedSnapshotsAndOwnership();
    PairConstructionPreservesValuesAndRefusals();
  });
}
