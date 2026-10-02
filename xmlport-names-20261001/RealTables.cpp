#include "core/table/TestTableC_132512.h"
#include "core/table/TestTableC_139063.h"
#include "Check.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Table.h"
#include "type/Code.h"
#include "type/Text.h"
#include <type_traits>

int main() {
  return gate::Run("RealTableIdentity", [] {
    using First = agiru::TestTableC_132512_Table;
    using Second = agiru::TestTableC_139063_Table;
    static_assert(std::is_same_v<decltype(Second::TextField), agiru::Text<2048>>);
    static_assert(std::is_same_v<decltype(Second::CodeField), agiru::Code<50>>);
    CHECK_TRUE("distinct original declarations have distinct types", (!std::is_same_v<First, Second>));
    CHECK_TRUE("first ID survives", First::kId == agiru::TableId{132512});
    CHECK_TRUE("second ID survives", Second::kId == agiru::TableId{139063});
    CHECK_TEXT("first original name survives", First::kName, "TestTableC");
    CHECK_TEXT("second original name survives", Second::kName, "Test Table C");
    const auto &first = agiru::TableTraits<First>::kTable;
    const auto &second = agiru::TableTraits<Second>::kTable;
    CHECK_TRUE("first source field count survives", first.fields.size() == 1 + agiru::kSystemFieldCount);
    CHECK_TRUE("second source field count survives", second.fields.size() == 11 + agiru::kSystemFieldCount);
    CHECK_TRUE("first IntegerField retains field number one", First::Field_No::IntegerField == agiru::FieldNo{1});
    CHECK_TRUE("second Integer Field retains field number two", Second::Field_No::IntegerField == agiru::FieldNo{2});
    CHECK_TRUE("first key retains its own field", first.keys[0].fields[0] == First::Field_No::IntegerField);
    CHECK_TRUE("second key retains its own field", second.keys[0].fields[0] == Second::Field_No::PrimaryField);
    CHECK_TRUE("first numeric registration survives", agiru::FindTable(First::kId)->table == &first);
    CHECK_TRUE("second numeric registration survives", agiru::FindTable(Second::kId)->table == &second);
    agiru::Temporary<First> firstRow;
    agiru::Temporary<Second> secondRow;
    firstRow.IntegerField = 17;
    secondRow.PrimaryField = 1;
    secondRow.IntegerField = 23;
    secondRow.TextField = "retained";
    secondRow.CodeField = "ABC";
    CHECK_TRUE("first declaration inserts into its temporary store", firstRow.Insert());
    CHECK_TRUE("second declaration inserts into its temporary store", secondRow.Insert());
    CHECK_TRUE("first row count is independent", firstRow.Count() == 1);
    CHECK_TRUE("second row count is independent", secondRow.Count() == 1);
    CHECK_TRUE("first original key retrieves its own row", firstRow.Get(17));
    CHECK_TRUE("second original key retrieves its own row", secondRow.Get(1));
    CHECK_TRUE("both original row layouts retain values", firstRow.IntegerField == 17 &&
        secondRow.IntegerField == 23 && secondRow.TextField == "retained" && secondRow.CodeField == "ABC");
  });
}
