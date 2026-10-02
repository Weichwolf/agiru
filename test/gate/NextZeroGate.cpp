#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Integer.h"

#include "Check.h"
#include "LineNumberBuffer.h"

#include <array>

namespace {

using agiru::app::tables::LineNumberBuffer;

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
    agiru::Temporary<LineNumberBuffer> temporary;
    Fill(temporary);
    TypedZeroPreservesPosition(temporary);
    ReferenceZeroPreservesPosition(temporary);
  });
}
