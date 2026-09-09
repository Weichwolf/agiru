#include "runtime/Error.h"
#include "type/AlArray.h"
#include "type/Integer.h"

#include "Check.h"

#include <string>

using agiru::AlArray;
using agiru::Error;
using agiru::Integer;

namespace {

/// A PARAMETER TAKES THE SHAPE OF ITS ARGUMENT AND NOT OF ITS DECLARATION (board:0633):
/// `array[10, 10]` given an `array[10, 100]` is walked to column 100 in the BaseApp, and BC runs it.
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

} // namespace

int main() {
  return gate::Run("AlArray", [] {
    AParameterKeepsTheArgumentsLength();
    TwoDimensionsConvertRowByRow();
  });
}
