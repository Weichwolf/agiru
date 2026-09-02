#include "Check.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Integer.h"
#include "type/Option.h"

#include <string>

using agiru::FieldType;
using agiru::RecordRef;
using agiru::Temporary;
using agiru::platform::Field;
using agiru::platform::FieldClass;

namespace {

/// AL table 27 is `Item` and its field 1 is `"No."`. The numbers are AL's, so they are named.
constexpr agiru::Integer kItem = 27;
constexpr agiru::Integer kItemNo = 1;
constexpr agiru::Integer kItemDescription = 3;
constexpr agiru::Integer kNoLength = 20;
constexpr agiru::Integer kDescriptionLength = 100;

/// A TEMPORARY VIRTUAL TABLE NEEDS NOTHING BUT ITS DECLARATION, and that is what makes declaring
/// these tables the first step rather than computing their rows. Measured over BCApps: 282 of the
/// 1 069 `Record Field` declarations carry `temporary`, and `Temporary<T>` keeps its own store and
/// touches no database at all.
void ATemporaryFieldIsAContainerAndNeedsNoPlatform() {
  Temporary<Field> rows;
  CHECK_TRUE("a fresh store is empty", rows.IsEmpty());

  rows.TableNo = kItem;
  rows.No = kItemNo;
  rows.FieldName = "No.";
  rows.Type = FieldType::Code;
  rows.Len = kNoLength;
  rows.Insert();

  rows.TableNo = kItem;
  rows.No = kItemDescription;
  rows.FieldName = "Description";
  rows.Type = FieldType::Text;
  rows.Len = kDescriptionLength;
  rows.Insert();

  CHECK_TRUE("two rows went in", rows.Count() == 2);

  Temporary<Field> read;
  read.Copy(rows, true);
  CHECK_TRUE("a row is found by its primary key", read.Get(kItem, kItemNo));
  CHECK_TEXT("carrying its name", std::string(read.FieldName.Value()), "No.");
  CHECK_TRUE("and its type", read.Type == FieldType::Code);
  CHECK_TRUE("a key that matches nothing answers false", !read.Get(kItem, agiru::Integer{2}));
}

/// THE TABLE IS THE PLATFORM'S, AND IT SAYS SO. 2000000041 is in the reserved range, and the fields
/// carry the AL names the BaseApp writes -- `Field."No."` 437 times, `Field."Field Caption"` 219.
void ItIsTheTableTheBaseAppReadsFrom() {
  Field one;
  RecordRef ref;
  ref.GetTable(one);

  CHECK_TRUE("the AL table number", ref.Number() == 2000000041);
  CHECK_TEXT("the AL name", std::string(ref.Name()), "Field");
  CHECK_TRUE("fifteen fields and no system fields, because nothing stores a virtual row",
             ref.FieldCount() == 15);
  CHECK_TEXT("the field AL calls \"No.\" keeps its dot", std::string(ref.Field(2).Name()), "No.");
  CHECK_TEXT(
      "and the caption field keeps its space", std::string(ref.Field(20).Name()), "Field Caption");
}

/// AL'S `FieldType` IS SPARSE AND ITS OPTION IS DENSE, both at once: NAV writes the gaps as empty
/// members of the OptionString. `option-data-type.md` promises "zero-based ... sequential numbers,
/// starting with 0" and the values run 3, 5, 7 ... 40, and the blanks are what reconciles them.
void TheSparseTypeIsADenseOptionWithBlanks() {
  const agiru::Option<FieldType> code{FieldType::Code};
  CHECK_TRUE("Code is the platform's 33", code.AsInteger() == 33);
  CHECK_TEXT("and it names itself", std::string(code.Name()), "Code");

  const agiru::Option<FieldType> guid{FieldType::Guid};
  CHECK_TEXT("GUID is spelled as AL spells it", std::string(guid.Name()), "GUID");

  // THE GAPS ARE DECLARED AND BLANK, not absent: an option that simply skipped them would not be
  // dense, and `Option` asserts density at compile time precisely so this cannot drift.
  const agiru::Option<FieldType> gap{4};
  CHECK_TRUE("a gap is a declared member", gap.IsDeclared());
  CHECK_TEXT("with no name", std::string(gap.Name()), "");
  CHECK_TRUE("and one past the last is not declared", !agiru::Option<FieldType>{41}.IsDeclared());

  const agiru::Option<FieldClass> flow{FieldClass::FlowField};
  CHECK_TEXT("the field class names itself too", std::string(flow.Name()), "FlowField");
}

} // namespace

int main() {
  return gate::Run("PlatformField", [] {
    ATemporaryFieldIsAContainerAndNeedsNoPlatform();
    ItIsTheTableTheBaseAppReadsFrom();
    TheSparseTypeIsADenseOptionWithBlanks();
  });
}
