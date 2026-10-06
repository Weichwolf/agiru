#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/Decimal.h"
#include "type/Option.h"
#include "type/Text.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>

namespace {

struct Kind {
  static constexpr std::array kValues{
      agiru::EnumValueDef{.ordinal = 0, .name = "Plain", .caption = "Translated"},
      agiru::EnumValueDef{.ordinal = 1, .name = "Group(Resource)", .caption = "Other caption"}};
};

}

template <> struct agiru::OptionTraits<Kind> : Kind {};

namespace {

struct Row : agiru::Table<Row> {
  static constexpr agiru::TableId kId{62433};
  static constexpr std::string_view kName = "Record Position Gate";
  agiru::detail::StateHandle State_Block;
  agiru::Option<Kind> Type;
  agiru::Text<100> Key;
  agiru::Decimal Amount;
  agiru::BigInteger Large{};
  agiru::Boolean Truth{};
  agiru::Text<100> Extra;
  agiru::Date Day;
};

}

template <> struct agiru::TableTraits<Row> {
  static constexpr std::array kFields{
      agiru::Declare<&Row::Type>(agiru::FieldNo{1}, "Type", "Type caption", offsetof(Row, Type)),
      agiru::Declare<&Row::Key>(agiru::FieldNo{2}, "Key", "Key=\"quoted\"", offsetof(Row, Key)),
      agiru::Declare<&Row::Amount>(agiru::FieldNo{3},
                                   "Amount",
                                   "Amount",
                                   offsetof(Row, Amount),
                                   agiru::Declared{.decimalPlaces = "2"}),
      agiru::Declare<&Row::Large>(agiru::FieldNo{4}, "Large", "", offsetof(Row, Large)),
      agiru::Declare<&Row::Truth>(agiru::FieldNo{5}, "Truth", "Truth", offsetof(Row, Truth)),
      agiru::Declare<&Row::Extra>(agiru::FieldNo{6}, "Extra", "Extra", offsetof(Row, Extra)),
      agiru::Declare<&Row::Day>(agiru::FieldNo{7}, "Day", "Day", offsetof(Row, Day))};
  static constexpr std::array kPrimary{agiru::FieldNo{1},
                                       agiru::FieldNo{2},
                                       agiru::FieldNo{3},
                                       agiru::FieldNo{4},
                                       agiru::FieldNo{5}};
  static constexpr std::array kKeys{agiru::KeyDef{.name = "Primary", .fields = kPrimary}};
  static constexpr agiru::TableDef kTable{
      .id = Row::kId, .name = Row::kName, .caption = Row::kName, .fields = kFields, .keys = kKeys};
};

namespace {

const agiru::RegisterTable<Row> kRegister;
constexpr std::string_view kNumericTail =
    ",Field3=0(1.2300),Field4=0(9223372036854775807),Field5=0(1)";

Row Value() {
  Row row;
  row.Type = agiru::Option<Kind>::FromInteger(1);
  row.Key = "A,B=O'Reilly)\"Z";
  row.Amount = agiru::Decimal::FromInvariantString("1.2300");
  row.Large = std::numeric_limits<agiru::BigInteger>::max();
  row.Truth = true;
  row.Extra = "untouched";
  return row;
}

bool Refuses(Row &row, std::string_view input) {
  try {
    row.SetPosition(input);
  } catch (const agiru::Error &) { return true; }
  return false;
}

void IndependentSyntax() {
  Row row = Value();
  constexpr std::string_view named =
      R"pos(Type caption=CONST("Group(Resource)"),"Key=""quoted"""=CONST("A,B=O'Reilly)""Z"),Amount=CONST(1.2300),Large=CONST(9223372036854775807),Truth=CONST(Yes))pos";
  const std::string numeric =
      "Field1=0(1),Field2=0(\"A,B=O'Reilly)\"\"Z\")" + std::string(kNumericTail);
  CHECK_TEXT("Record defaults to captions and nonlocalized option names", row.GetPosition(), named);
  CHECK_TEXT("numeric positions carry FieldN, zero and exact ordinal values",
             row.GetPosition(false),
             numeric);
  agiru::RecordRef ref;
  ref.GetTable(row);
  CHECK_TEXT("RecordRef has the same default position contract", ref.GetPosition(), named);
  CHECK_TEXT("RecordRef has the same numeric position contract", ref.GetPosition(false), numeric);
  Row back;
  back.Extra = "not fetched";
  back.SetPosition(named);
  CHECK_TEXT(
      "constant assignment preserves typed primary values", back.GetPosition(false), numeric);
  CHECK_TEXT("SetPosition neither fetches nor overwrites nonkeys",
             std::string_view(back.Extra),
             "not fetched");
  CHECK_TEXT("DecimalPlaces does not round a primary-key assignment",
             back.Amount.ToInvariantString(),
             "1.2300");
  ref.SetPosition(
      "Field1=0(0),Field2=0(FromRef),Field3=0(2.3456),Field4=0(-9223372036854775808),Field5=0(0)");
  ref.SetTable(back);
  CHECK_TEXT(
      "RecordRef reads an independent literal position", std::string_view(back.Key), "FromRef");
  CHECK_TEXT("RecordRef retains an exact negative Int64 boundary",
             std::to_string(back.Large),
             "-9223372036854775808");
  CHECK_TEXT(
      "RecordRef preserves the source nonkey buffer", std::string_view(back.Extra), "untouched");
}

void LiteralValues() {
  struct Case {
    std::string_view value;
    std::string_view literal;
  };

  constexpr std::array cases{Case{.value = "", .literal = ""},
                             Case{.value = "a,b=c", .literal = "a,b=c"},
                             Case{.value = "(a", .literal = "(a"},
                             Case{.value = "a)", .literal = "\"a)\""},
                             Case{.value = "a\"b", .literal = "a\"b"},
                             Case{.value = "O'Reilly", .literal = "O'Reilly"},
                             Case{.value = " a ", .literal = "\" a \""},
                             Case{.value = "\u00a0x\u2003", .literal = "\"\u00a0x\u2003\""},
                             Case{.value = "@*|&..", .literal = "@*|&.."},
                             Case{.value = "雪🙂", .literal = "雪🙂"},
                             Case{.value = "\u200bx\ufeff", .literal = "\u200bx\ufeff"}};
  for (const auto &one : cases) {
    Row row = Value();
    row.Key = one.value;
    const std::string expected =
        "Field1=0(1),Field2=0(" + std::string(one.literal) + ")" + std::string(kNumericTail);
    CHECK_TEXT("literal serialization follows CONST quotation, not filter quotation",
               row.GetPosition(false),
               expected);
    Row back;
    back.SetPosition(expected);
    CHECK_TEXT("literal contents survive the shared parser", std::string_view(back.Key), one.value);
  }
}

void InvalidInputs() {
  constexpr std::array inputs{
      "",
      "Field1=0(1)",
      "Field1=0(1),",
      "9999='x'",
      "Field1=0(1) garbage",
      "Field1=FILTER(1),Field2=0(A),Field3=0(1),Field4=0(2),Field5=0(1)",
      "Field1=1(1),Field2=0(A),Field3=0(1),Field4=0(2),Field5=0(1)",
      "Field1=FIELD(Type),Field2=0(A),Field3=0(1),Field4=0(2),Field5=0(1)",
      "Field2=0(A),Field1=0(1),Field3=0(1),Field4=0(2),Field5=0(1)",
      "Field1=0(1),Field1=0(1),Field3=0(1),Field4=0(2),Field5=0(1)",
      "Field1=0(1),Field2=0(A),Field3=0(1),Field4=0(2),Field6=0(changed)",
      "Field1=0(1),Field2=0(\"unterminated),Field3=0(1),Field4=0(2),Field5=0(1)",
      "Field1=0(1),Field2=0(A),Field3=0(1),Field4=0(2),Field5=0(1),Field6=0(extra)",
      "Field1=0(1),Field2=0(A),Field3=0(1),Field4=0(9223372036854775808),Field5=0(1)",
      "Field1=0(1),Field2=0(A),Field3=0(1),Field4=0(2junk),Field5=0(1)",
      "Field1=0(1),Field2=0(A),Field3=0(1),=0(2),Field5=0(1)",
      "Field1=0(1),Field2=0(A),Field3=0(1),Field4=0(2),Field5=0(not-bool)"};
  for (const auto *input : inputs) {
    Row row;
    CHECK_TRUE("malformed, partial, reordered and nonkey positions refuse", Refuses(row, input));
  }
  Row row;
  row.SetPosition(" type caption = cOnSt( Plain ) , Key=0( X ) , Amount=CONST(1.2345), Large=0(2), "
                  "Truth=CONST(No) ");
  CHECK_TEXT("keywords, captions, names and boundary spaces parse", std::string_view(row.Key), "X");
  CHECK_TEXT("key evaluation does not apply UI decimal rounding",
             row.Amount.ToInvariantString(),
             "1.2345");
}

void CurrentKeyNavigation() {
  agiru::Temporary<Row> row;
  for (const auto *key : {"A", "C", "E"}) {
    row.Key = key;
    CHECK_TRUE("temporary rows establish an independent navigation oracle", row.Insert(false));
  }
  row.SetRange(row.Type, agiru::Option<Kind>::FromInteger(0));
  CHECK_TRUE("the old view is positioned on the first row", row.FindFirst());
  Row anchor;
  anchor.Key = "C";
  row.Extra = "kept";
  const std::string filters = std::string(row.GetFilters());
  row.SetPosition(anchor.GetPosition(false));
  CHECK_TEXT("SetPosition preserves the active filters", std::string(row.GetFilters()), filters);
  CHECK_TEXT("the new key does not read the stored row", std::string_view(row.Extra), "kept");
  CHECK_TRUE("SetPosition invalidates the old cursor", row.State_Block.Peek()->viewDirty);
  CHECK_TRUE("Next uses the assigned key rather than the old view index", row.Next() == 1);
  CHECK_TEXT("Next reaches the row after the explicit anchor", std::string_view(row.Key), "E");
}

void ClosingDateValues() {
  constexpr std::array fields{agiru::FieldNo{7}};
  const std::array keys{agiru::KeyDef{.name = "Date key", .fields = fields}};
  auto table = agiru::TableTraits<Row>::kTable;
  table.keys = keys;
  Row row;
  constexpr int kExampleYear = 2028;
  constexpr unsigned kExampleDay = 25;
  row.Day = agiru::Date::FromYmd(kExampleYear, 1, kExampleDay).Closing();
  CHECK_TEXT("current date foundation retains the closing marker, not the XML date alone",
             agiru::detail::PositionText(&row, table, true),
             "Day=CONST(C2028-01-25)");
  Row back;
  agiru::detail::TakePosition(&back, table, "Field7=0(C2028-01-25)");
  CHECK_TRUE("closing dates do not silently become normal dates", back.Day == row.Day);
  agiru::detail::TakePosition(&back, table, "Field7=0()");
  CHECK_TRUE("an empty date constant retains the undefined date", back.Day.IsUndefined());
}

void BorrowedInputsAndExactPrecision() {
  Row row;
  row.Key = "Field1=0(0),Field2=0(x),Field3=0(0),Field4=0(0),Field5=0(0)";
  row.SetPosition(std::string_view(row.Key));
  CHECK_TEXT("position text borrowed from a key survives assigning that key",
             std::string_view(row.Key),
             "x");
  constexpr std::string_view exact = "0.1234567890123456789012345678";
  row.SetPosition("Field1=0(0),Field2=0(x),Field3=0(" + std::string(exact) +
                  "),Field4=0(0),Field5=0(0)");
  CHECK_TEXT(
      "position parsing retains all 28 decimal places", row.Amount.ToInvariantString(), exact);
  const std::string before = row.GetPosition(false);
  CHECK_TRUE("a truncated position refuses", Refuses(row, "Field1=0(1),Field2=0(changed)"));
  CHECK_TEXT("incomplete keys refuse before altering any key", row.GetPosition(false), before);
}

}

int main() {
  return gate::Run("Record Position", [] {
    IndependentSyntax();
    LiteralValues();
    InvalidInputs();
    CurrentKeyNavigation();
    ClosingDateValues();
    BorrowedInputsAndExactPrecision();
  });
}
