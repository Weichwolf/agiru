#include "Check.h"
#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/RecordId.h"

#include <cstddef>
#include <string>

using agiru::Error;
using agiru::RecordId;
using agiru::TableId;

namespace {

/// The worked example: `Sales Header: Order,101001`. The table's caption, then `": "`, then the
/// primary key values separated by commas.
RecordId SalesOrder() {
  constexpr int kSalesHeader = 36;
  return RecordId{TableId{kSalesHeader}, "Sales Header", {"Order", "101001"}};
}

/// THE SEPARATOR IS LOAD-BEARING AND THE BASEAPP DEPENDS ON IT, twice:
/// `PrimaryKey := CopyStr(Format(RecordID), StrPos(Format(RecordID), ': ') + 2)`. A different
/// separator leaves that code with the whole string, or with nothing.
void TheTextFormIsCaptionColonSpaceThenTheKey() {
  CHECK_TEXT("the documented shape", SalesOrder().ToText(), "Sales Header: Order,101001");

  // What the BaseApp does with it, done here: everything after the first ": " is the key.
  const std::string text = SalesOrder().ToText();
  const std::size_t at = text.find(": ");
  CHECK_TRUE("the separator is there to be found", at != std::string::npos);
  CHECK_TEXT("and what follows it is the primary key", text.substr(at + 2), "Order,101001");
  CHECK_TEXT("while what precedes it is the caption", text.substr(0, at), "Sales Header");
}

/// A BLANK RecordId FORMATS TO NOTHING, which `CalcItemAvailability` and `ServiceConnection` both
/// test for: `if Format(RecordID) = '' then exit;`.
void ABlankRecordIdFormatsToTheEmptyString() {
  const RecordId blank;
  CHECK_TRUE("it says it is blank", blank.IsEmpty());
  CHECK_TRUE("and renders as nothing rather than as a colon", blank.ToText().empty());

  // THE NEGATIVE CONTROL: one that names a row is not blank and does render.
  CHECK_TRUE("while one that names a row is not blank", !SalesOrder().IsEmpty());
  CHECK_TRUE("and renders", !SalesOrder().ToText().empty());
}

/// `recordid-tableno-method.md`: "This function returns an error if the record is blank."
void TableNoRefusesOnABlankRecordId() {
  constexpr int kSalesHeader = 36;
  CHECK_TRUE("a RecordId that names a row answers its table",
             SalesOrder().TableNo() == kSalesHeader);

  std::string said;
  try {
    (void)RecordId{}.TableNo();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a blank one refuses rather than answering 0", !said.empty());
}

void TwoRecordIdsCompareByTableAndKey() {
  CHECK_TRUE("the same row is the same RecordId", SalesOrder() == SalesOrder());
  const RecordId other{TableId{36}, "Sales Header", {"Invoice", "101001"}};
  CHECK_TRUE("a different key is a different row", !(SalesOrder() == other));
  const RecordId elsewhere{TableId{37}, "Sales Line", {"Order", "101001"}};
  CHECK_TRUE("and so is the same key in another table", !(SalesOrder() == elsewhere));
}

} // namespace

int main() {
  return gate::Run("RecordId", [] {
    TheTextFormIsCaptionColonSpaceThenTheKey();
    ABlankRecordIdFormatsToTheEmptyString();
    TableNoRefusesOnABlankRecordId();
    TwoRecordIdsCompareByTableAndKey();
  });
}
