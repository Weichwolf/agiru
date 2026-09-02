#include "Check.h"
#include "runtime/Error.h"
#include "type/Dictionary.h"
#include "type/List.h"

#include <string>

using agiru::Dictionary;
using agiru::Error;
using agiru::List;

namespace {

constexpr int kTens = 10;
constexpr int kFive = 5;
constexpr int kForty = 40;
constexpr int kNine = 9;

List<int> Of(int count) {
  List<int> list;
  for (int i = 1; i <= count; ++i) { list.Add(i * kTens); }
  return list;
}

/// EVERY INDEX COUNTS FROM ONE. `list-indexof-method.md` says "the one-based index of the first
/// occurrence", and a list that answered for index 0 would be off by one everywhere.
void EveryIndexCountsFromOne() {
  const List<int> list = Of(3);
  CHECK_TRUE("the first element is at index 1", list.Get(1) == 10);
  CHECK_TRUE("and the last at Count()", list.Get(list.Count()) == 30);
  CHECK_TRUE("IndexOf answers one-based", list.IndexOf(20) == 2);
  CHECK_TRUE("and 0 for something not in the list", list.IndexOf(99) == 0);

  // THE NEGATIVE CONTROL: index 0 is outside the list, not the first element.
  std::string said;
  try {
    (void)list.Get(0);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("index 0 is refused rather than answered", !said.empty());
  said.clear();
  try {
    (void)list.Get(list.Count() + 1);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("and so is one past the end", !said.empty());
}

/// The `Ok :=` form answers instead of raising, which is the pair AL gives most of these.
void TheReportingFormAnswersInsteadOfRaising() {
  const List<int> list = Of(2);
  int held = -1;
  CHECK_TRUE("Get with an out parameter reports success", list.Get(2, held));
  CHECK_TRUE("and writes the element", held == 20);
  held = -1;
  CHECK_TRUE("out of range it reports failure", !list.Get(kFive, held));
  CHECK_TRUE("and leaves the value alone", held == -1);
}

void InsertRemoveAndReverseKeepTheOrder() {
  List<int> list = Of(3);
  list.Insert(1, kFive);
  CHECK_TRUE("Insert puts the element at that position", list.Get(1) == kFive);
  CHECK_TRUE("and pushes the rest along", list.Get(2) == 10 && list.Count() == 4);
  list.Insert(list.Count() + 1, kForty);
  CHECK_TRUE("inserting one past the end appends", list.Get(5) == kForty);

  list.RemoveAt(1);
  CHECK_TRUE("RemoveAt takes the element there", list.Get(1) == 10);
  CHECK_TRUE("Remove takes the first occurrence of a value", list.Remove(20));
  CHECK_TRUE("and reports when there is none", !list.Remove(999));

  List<int> three = Of(3);
  three.Reverse();
  CHECK_TRUE("Reverse turns it around", three.Get(1) == 30 && three.Get(3) == 10);
}

void RangesAreTakenAndRemovedByPositionAndCount() {
  const List<int> list = Of(kFive);
  const List<int> middle = list.GetRange(2, 3);
  CHECK_TRUE("GetRange takes that many from that position", middle.Count() == 3);
  CHECK_TRUE("beginning at the one-based index", middle.Get(1) == 20);

  List<int> shortened = Of(kFive);
  shortened.RemoveRange(2, 2);
  CHECK_TRUE("RemoveRange takes that many out", shortened.Count() == 3);
  CHECK_TRUE("and closes the gap", shortened.Get(2) == 40);

  std::string said;
  try {
    (void)list.GetRange(4, kFive);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a range past the end is refused", !said.empty());
}

/// A DICTIONARY IS WALKED IN KEY ORDER, and that is a decision. Keys() and Values() hand out lists
/// AL walks with `foreach`, and a run has to give the same answer twice for the same data.
void ADictionaryIsOrderedByItsKeys() {
  Dictionary<std::string, int> map;
  map.Add("zebra", 1);
  map.Add("apple", 2);
  map.Add("mango", 3);

  const List<std::string> keys = map.Keys();
  CHECK_TEXT("the keys come out in key order and not insertion order", keys.Get(1), "apple");
  CHECK_TEXT("second", keys.Get(2), "mango");
  CHECK_TEXT("third", keys.Get(3), "zebra");
  CHECK_TRUE("and the values follow their keys", map.Values().Get(1) == 2);

  std::string said;
  try {
    map.Add("apple", kNine);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("Add refuses a key that is already there", !said.empty());
  CHECK_TRUE("while Set replaces it", (map.Set("apple", kNine), map.Get("apple") == kNine));

  int held = -1;
  CHECK_TRUE("the reporting Get answers for a key that is there", map.Get("mango", held));
  CHECK_TRUE("and writes the value", held == 3);
  CHECK_TRUE("for one that is not, it reports failure", !map.Get("pear", held));
  CHECK_TRUE("and leaves the value alone", held == 3);
  CHECK_TRUE("Remove reports what it removed", map.Remove("mango"));
  CHECK_TRUE("and says so when there is nothing", !map.Remove("mango"));
  CHECK_TRUE("Count follows", map.Count() == 2);
}

} // namespace

int main() {
  return gate::Run("List", [] {
    EveryIndexCountsFromOne();
    TheReportingFormAnswersInsteadOfRaising();
    InsertRemoveAndReverseKeepTheOrder();
    RangesAreTakenAndRemovedByPositionAndCount();
    ADictionaryIsOrderedByItsKeys();
  });
}
