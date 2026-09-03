#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Table.h"
#include "type/Integer.h"

#include "Check.h"
#include "LineNumberBuffer.h"

#include <string>
#include <vector>

using agiru::Error;
using agiru::Temporary;
using agiru::app::tables::LineNumberBuffer;

namespace {

constexpr agiru::Integer kTens = 10;

Temporary<LineNumberBuffer> With(const std::vector<agiru::Integer> &numbers) {
  Temporary<LineNumberBuffer> buffer;
  for (const agiru::Integer n : numbers) {
    buffer.OldLineNumber = n;
    buffer.NewLineNumber = n * kTens;
    buffer.Insert();
  }
  return buffer;
}

/// A KEY IS ORDERED BY ITS TYPE AND NEVER BY ITS TEXT, and this case is the one that catches the
/// difference: rendered as strings, 10 sorts before 9. Every buffer keyed on an entry number walks
/// in this order, and both orders look right until the numbers reach ten.
void RowsWalkInPrimaryKeyOrder() {
  // Out of order, and crossing ten twice: rendered as strings, 10 sorts before 9 and 100 before
  // 2. Nothing smaller than this catches a lexical comparison.
  constexpr agiru::Integer kNine = 9;
  constexpr agiru::Integer kHundred = 100;
  Temporary<LineNumberBuffer> buffer = With({kNine, kTens, 1, kHundred, 2});

  std::string walked;
  for (bool more = buffer.FindSet(); more; more = buffer.Next()) {
    walked += std::to_string(buffer.OldLineNumber) + " ";
  }
  CHECK_TEXT("the walk is numeric and not lexical", walked, "1 2 9 10 100 ");
  CHECK_TRUE("and every row came back", buffer.Count() == 5);
}

/// AL refuses a duplicate primary key on a temporary record exactly as on a real one.
void ADuplicateKeyIsRefused() {
  constexpr agiru::Integer kOther = 999;
  Temporary<LineNumberBuffer> buffer = With({1});
  buffer.OldLineNumber = 1;
  buffer.NewLineNumber = kOther;

  std::string said;
  try {
    buffer.Insert();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("inserting the same key twice refuses", !said.empty());
  CHECK_TRUE("and the store still holds one row", buffer.Count() == 1);

  // THE NEGATIVE CONTROL. A store that refused every insert would pass the check above.
  buffer.OldLineNumber = 2;
  said.clear();
  try {
    buffer.Insert();
  } catch (const Error &e) { said = e.what(); }
  CHECK_SILENT("a different key inserts", said);
  CHECK_TRUE("and the store grows", buffer.Count() == 2);
}

constexpr agiru::Integer kReplacement = 222;

void GetsRowFinds() {
  Temporary<LineNumberBuffer> buffer = With({1, 2, 3});

  CHECK_TRUE("Get finds a row that is there", buffer.Get(2));
  CHECK_TRUE("and brings its other fields with it", buffer.NewLineNumber == 20);
  CHECK_TRUE("Get on a key that is not there answers false", !buffer.Get(4));

  (void)buffer.Get(2);
  buffer.NewLineNumber = kReplacement;
  CHECK_TRUE("Modify reports the row it replaced", buffer.Modify());
  (void)buffer.Get(2);
  CHECK_TRUE("and the replacement is what comes back", buffer.NewLineNumber == kReplacement);
  CHECK_TRUE("while the rows around it are untouched", buffer.Count() == 3);

  (void)buffer.Get(2);
  CHECK_TRUE("Delete reports the row it removed", buffer.Delete());
  CHECK_TRUE("the store is one shorter", buffer.Count() == 2);
  CHECK_TRUE("and the row is gone", !buffer.Get(2));
  CHECK_TRUE("Delete on a key that is not there answers false", !buffer.Delete());
}

/// AL `Copy(From, true)` makes two variables share ONE set of rows. It is why the version rides on
/// the store rather than on the record, and it is how a codeunit hands its buffer out.
constexpr agiru::Integer kSeven = 7;

void CopyingWithShareGivesOneStoreAndWithoutGivesTwo() {
  Temporary<LineNumberBuffer> owner = With({1, 2});

  Temporary<LineNumberBuffer> shared;
  shared.Copy(owner, true);
  CHECK_TRUE("a shared copy sees the rows", shared.Count() == 2);
  owner.OldLineNumber = 3;
  owner.NewLineNumber = 3 * kTens;
  owner.Insert();
  CHECK_TRUE("and sees a row added through the other variable", shared.Count() == 3);
  shared.DeleteAll();
  CHECK_TRUE("and a clear through either empties both", owner.Count() == 0);

  // THE NEGATIVE CONTROL, and it is the whole meaning of the second argument.
  const Temporary<LineNumberBuffer> apart = With({kSeven});
  Temporary<LineNumberBuffer> separate;
  separate.Copy(apart, false);
  CHECK_TRUE("a copy WITHOUT sharing has its own empty store", separate.Count() == 0);
  CHECK_TRUE("while the one it copied from keeps its rows", apart.Count() == 1);
  CHECK_TRUE("though the current row came across", separate.OldLineNumber == kSeven);
}

/// A temporary record reaches no database, which is what lets a test build a result set with no
/// session open at all.
void ATemporaryRecordNeedsNoSession() {
  CHECK_TRUE("no session is open", !agiru::Session::HasCurrent());
  std::string said;
  try {
    Temporary<LineNumberBuffer> buffer = With({1, 2, 3});
    (void)buffer.Get(1);
  } catch (const Error &e) { said = e.what(); }
  CHECK_SILENT("and the whole store works anyway", said);
}

} // namespace

int main() {
  return gate::Run("Temporary", [] {
    RowsWalkInPrimaryKeyOrder();
    ADuplicateKeyIsRefused();
    GetsRowFinds();
    CopyingWithShareGivesOneStoreAndWithoutGivesTwo();
    ATemporaryRecordNeedsNoSession();
  });
}
