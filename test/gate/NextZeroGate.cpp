#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Integer.h"

#include "Check.h"
#include "LineNumberBuffer.h"

#include <array>
#include <limits>

namespace {

using agiru::app::tables::LineNumberBuffer;

template <typename Record> agiru::Integer CurrentKey(const Record &row) {
  return row.OldLineNumber;
}

agiru::Integer CurrentKey(agiru::RecordRef &reference) {
  LineNumberBuffer observed;
  reference.SetTable(observed);
  return observed.OldLineNumber;
}

template <typename Record> void Fill(Record &row) {
  constexpr std::array<agiru::Integer, 3> kKeys{1, 2, 3};
  for (const agiru::Integer key : kKeys) {
    row.OldLineNumber = key;
    row.NewLineNumber = key;
    row.Insert();
  }
}

template <typename Record> void TypedZeroPreservesPosition(Record &row) {
  CHECK_TRUE("the typed fixture has a first row", row.FindFirst());
  CHECK_TRUE("typed Next(0) takes no steps", row.Next(0) == 0);
  CHECK_TRUE("typed Next(0) preserves the current row", row.OldLineNumber == 1);
  CHECK_TRUE("typed Next() still takes one step", row.Next() == 1);
  CHECK_TRUE("the cursor continues immediately after the preserved row", row.OldLineNumber == 2);
  CHECK_TRUE("the typed fixture has a last row", row.FindLast());
  constexpr agiru::Integer kPendingValue = 99;
  row.NewLineNumber = kPendingValue;
  CHECK_TRUE("typed Next(0) at the last row takes no steps", row.Next(0) == 0);
  CHECK_TRUE("typed Next(0) does not reload pending field changes",
             row.NewLineNumber == kPendingValue);
  CHECK_TRUE("typed Next(0) does not exhaust the position", row.Next(-1) == -1);
  CHECK_TRUE("backward navigation starts at the preserved last row", row.OldLineNumber == 2);
}

template <typename Record> void ReferenceZeroPreservesPosition(Record &row) {
  CHECK_TRUE("the reference fixture has a first row", row.FindFirst());
  agiru::RecordRef reference;
  reference.GetTable(row);
  CHECK_TRUE("the reference positions its own cursor", reference.FindFirst());
  CHECK_TRUE("RecordRef.Next(0) takes no steps", reference.Next(0) == 0);
  LineNumberBuffer observed;
  reference.SetTable(observed);
  CHECK_TRUE("RecordRef.Next(0) preserves the current row", observed.OldLineNumber == 1);
  CHECK_TRUE("RecordRef.Next() still takes one step", reference.Next() == 1);
  reference.SetTable(observed);
  CHECK_TRUE("RecordRef.Next() continues after the preserved row", observed.OldLineNumber == 2);
}

template <typename Record> void PartialStepsPreserveLastReachedRow(Record &row) {
  CHECK_TRUE("partial-step fixture starts at the first row", row.FindFirst());
  CHECK_TRUE("forward overshoot returns the actual steps", row.Next(5) == 2);
  CHECK_TRUE("forward overshoot retains the last reached row", CurrentKey(row) == 3);
  CHECK_TRUE("forward endpoint remains exhausted", row.Next() == 0);
  CHECK_TRUE("reverse navigation works after forward exhaustion", row.Next(-1) == -1);
  CHECK_TRUE("reverse navigation starts from the last reached row", CurrentKey(row) == 2);
  CHECK_TRUE("backward overshoot returns the actual signed steps", row.Next(-5) == -1);
  CHECK_TRUE("backward overshoot retains the first reached row", CurrentKey(row) == 1);
  CHECK_TRUE("forward navigation works after backward exhaustion", row.Next() == 1);
  CHECK_TRUE("forward navigation starts from the first reached row", CurrentKey(row) == 2);
  CHECK_TRUE("extreme-step fixture starts at the last row", row.FindLast());
  CHECK_TRUE("minimum Integer steps have a representable magnitude",
             row.Next(std::numeric_limits<agiru::Integer>::min()) == -2);
  CHECK_TRUE("minimum Integer steps retain the first reached row", CurrentKey(row) == 1);
  CHECK_TRUE("maximum Integer steps stop at the actual endpoint",
             row.Next(std::numeric_limits<agiru::Integer>::max()) == 2);
  CHECK_TRUE("maximum Integer steps retain the last reached row", CurrentKey(row) == 3);
}

template <typename Record> void ReferencePartialSteps(Record &row) {
  agiru::RecordRef reference;
  reference.GetTable(row);
  PartialStepsPreserveLastReachedRow(reference);
}

template <typename Record> void MultipleBlocksPreserveLastReachedRow(Record &row) {
  CHECK_TRUE("multi-block fixture starts at the first row", row.FindFirst());
  CHECK_TRUE("multi-block overshoot returns the actual steps", row.Next(200) == 129);
  CHECK_TRUE("multi-block overshoot retains the last reached row", CurrentKey(row) == 130);
  CHECK_TRUE("multi-block reversal returns all actual steps", row.Next(-200) == -129);
  CHECK_TRUE("multi-block reversal retains the first reached row", CurrentKey(row) == 1);
}

template <typename Record> void MultipleBlocks(Record &row) {
  constexpr agiru::Integer kMultipleBlockRows = 130;
  for (agiru::Integer key = 4; key <= kMultipleBlockRows; ++key) {
    row.OldLineNumber = key;
    row.NewLineNumber = key;
    row.Insert();
  }
  MultipleBlocksPreserveLastReachedRow(row);
  agiru::RecordRef reference;
  reference.GetTable(row);
  MultipleBlocksPreserveLastReachedRow(reference);
}

}

int main() {
  return gate::Run("NextZero", [] {
    const agiru::Session session(AGIRU_TEST_DSN);
    const auto &table = agiru::TableTraits<LineNumberBuffer>::kTable;
    agiru::DropTable(agiru::Session::Current().Database(), table);
    agiru::CreateTable(agiru::Session::Current().Database(), table);
    LineNumberBuffer stored;
    Fill(stored);
    TypedZeroPreservesPosition(stored);
    ReferenceZeroPreservesPosition(stored);
    PartialStepsPreserveLastReachedRow(stored);
    ReferencePartialSteps(stored);
    MultipleBlocks(stored);
    agiru::Temporary<LineNumberBuffer> temporary;
    Fill(temporary);
    TypedZeroPreservesPosition(temporary);
    ReferenceZeroPreservesPosition(temporary);
    PartialStepsPreserveLastReachedRow(temporary);
    ReferencePartialSteps(temporary);
    MultipleBlocks(temporary);
  });
}
