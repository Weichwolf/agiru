#include "runtime/Error.h"
#include "type/AlArray.h"
#include "type/Integer.h"
#include "type/Text.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <string>

using agiru::AlArray;
using agiru::Error;
using agiru::Integer;

namespace {

/// A PARAMETER TAKES THE SHAPE OF ITS ARGUMENT AND NOT OF ITS DECLARATION (board:0633):
/// `array[10, 10]` given an `array[10, 100]` is walked to column 100 in the BaseApp, and BC runs
/// it.
Integer FourthOf(AlArray<Integer, 2> narrow) {
  return narrow[4];
}

Integer FifthOf(AlArray<Integer, 2> narrow) {
  return narrow[5];
}

void AParameterKeepsTheArgumentsLength() {
  AlArray<Integer, 4> wide;
  wide[4] = 44;
  CHECK_TRUE("element 4 is read through a parameter declared with 2", FourthOf(wide) == 44);
  CHECK_TRUE("and ArrayLen answers the argument's length",
             ArrayLen(AlArray<Integer, 2>(wide)) == 4);

  // THE NEGATIVE CONTROL: the bound is the ARGUMENT'S, so 5 refuses. An array that stopped checking
  // would pass the first claim and not this one.
  std::string said;
  try {
    (void)FifthOf(wide);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("element 5 refuses", !said.empty());
}

void TwoDimensionsConvertRowByRow() {
  AlArray<AlArray<Integer, 3>, 2> wide;
  wide[2][3] = 23;
  AlArray<AlArray<Integer, 1>, 2> narrow = wide;
  CHECK_TRUE("the inner row keeps its three", narrow[2][3] == 23);
  AlArray<AlArray<Integer, 1>, 2> copied = narrow;
  CHECK_TRUE("and a copy of it keeps them too", copied[2][3] == 23);
  narrow[2][3] = 0;
  CHECK_TRUE("while the copy is its own storage", copied[2][3] == 23);
}

/// COMPRESSARRAY KEEPS THE LENGTH AND THE ORDER: `system-compressarray-method.md` says the
/// result "has the same number of elements as the input array, but empty entries appear at the
/// end", and the count it returns is where those begin.
void CompressArrayMovesTheFullEntriesForward() {
  AlArray<agiru::Text<30>, 5> spread;
  spread[1] = "one";
  spread[3] = "three";
  spread[5] = "five";
  const Integer kept = agiru::CompressArray(spread);
  CHECK_TRUE("the count is how many were not empty", kept == 3);
  CHECK_TRUE("and they keep their order at the front",
             std::string_view(spread[1]) == "one" && std::string_view(spread[2]) == "three" &&
                 std::string_view(spread[3]) == "five");
  CHECK_TRUE("the empty ones are at the end and the length is unchanged",
             std::string_view(spread[4]).empty() && std::string_view(spread[5]).empty() &&
                 spread.Length() == 5);
  // THE NEGATIVE CONTROL: an array with nothing empty is returned untouched, and one with
  // nothing full answers zero rather than moving anything.
  AlArray<agiru::Text<30>, 2> full;
  full[1] = "a";
  full[2] = "b";
  CHECK_TRUE("a full array is unchanged",
             agiru::CompressArray(full) == 2 && std::string_view(full[1]) == "a" &&
                 std::string_view(full[2]) == "b");
  AlArray<agiru::Text<30>, 2> empty;
  CHECK_TRUE("an empty one answers zero", agiru::CompressArray(empty) == 0);
}

Integer &AccessInteger(void *storage, std::size_t index) {
  return static_cast<Integer *>(storage)[index];
}

/// A stand-in for a view whose length no longer matches its storage -- the door keeps the
/// `(storage, count, accessor)` constructor protected so a client cannot build one, and only a
/// dead read (board:0718) produces one at run time, so the gate reaches it through a subclass.
struct View : AlArray<Integer, 0> {
  View(void *storage, std::size_t count) : AlArray<Integer, 0>(storage, count, &AccessInteger) {}
};

/// A VIEW THAT CLAIMS MORE ELEMENTS THAN ANY AL ARRAY DECLARES is a view over storage that is gone
/// (board:0633, board:0718): copying it is refused BY NAME, not left to die in
/// `std::bad_array_new_length` or to read the dead bytes. The largest declared array in the corpus
/// is `array[2000]`; the guard is 1 000 000. It is a diagnostic, not the fix -- the dead view
/// itself is board:0718.
void ADeadViewIsRefusedByName() {
  Integer buffer[3] = {1, 2, 3};
  const View deadView(buffer, 2'000'000);
  std::string said;
  try {
    const AlArray<Integer, 2> copy(deadView);
    (void)copy;
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a view claiming two million elements over a buffer of three is refused",
             said.find("view over storage that is gone") != std::string::npos);

  // THE NEGATIVE CONTROL: a view of a length an array really has copies without a word, so the
  // guard is refusing the impossible length and not every copy.
  const View liveView(buffer, 3);
  std::string ok;
  try {
    const AlArray<Integer, 3> copy(liveView);
    CHECK_TRUE("and a real view's elements come across", copy[1] == 1 && copy[3] == 3);
  } catch (const Error &e) { ok = e.what(); }
  CHECK_TRUE("a view of a real length copies without refusing", ok.empty());
}

} // namespace

int main() {
  return gate::Run("AlArray", [] {
    CompressArrayMovesTheFullEntriesForward();
    AParameterKeepsTheArgumentsLength();
    TwoDimensionsConvertRowByRow();
    ADeadViewIsRefusedByName();
  });
}
