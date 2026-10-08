#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageValue.h"
#include "runtime/PageVariableValue.h"
#include "type/Action.h"
#include "type/Base64.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Enum.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/RecordId.h"
#include "type/StringValue.h"
#include "type/Time.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <limits>
#include <string>

namespace {

constexpr std::size_t kCodeCharacters = 20;
constexpr agiru::Integer kUndeclaredOrdinal = 99;
constexpr std::size_t kBlobIndex = 9;
constexpr std::size_t kFilterIndex = 19;

struct Row {
  agiru::Decimal amount = agiru::Decimal::FromInvariantString("1.2300");
  agiru::BigInteger large = std::numeric_limits<agiru::BigInteger>::max();
  agiru::Boolean flag = true;
  agiru::Text<0> text{"Grüezi <script> & \"quoted\"\r\n"};
  agiru::Enum<> choice{10};
  agiru::Date date = agiru::Date::FromYmd(2026, 10, 6).Closing();
  agiru::Time time = agiru::Time::FromHms(12, 34, 56, 789);
  agiru::DateTime instant = agiru::DateTime::Create(date.Normal(), time);
  agiru::Duration duration{std::numeric_limits<agiru::BigInteger>::min()};
  agiru::Code<kCodeCharacters> code{" abc "};
  agiru::Integer integer = -123;
  agiru::Option<> option{10};
  agiru::Guid guid{"{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"};
  agiru::DateFormula formula = agiru::DateFormula::FromText("<1M>").value();
  agiru::RecordId record{agiru::TableId{50399}, "caption", {"one", "two"}};
};

constexpr std::array kChoices{
    agiru::EnumValueDef{.ordinal = 0, .name = "Before", .caption = "Vorher"},
    agiru::EnumValueDef{.ordinal = 10, .name = "After", .caption = "Nachher"}};
constexpr std::array kFields{
    agiru::FieldDef{.offset = offsetof(Row, amount),
                    .no = agiru::FieldNo{1},
                    .type = agiru::FieldType::Decimal},
    agiru::FieldDef{.offset = offsetof(Row, large),
                    .no = agiru::FieldNo{2},
                    .type = agiru::FieldType::BigInteger},
    agiru::FieldDef{
        .offset = offsetof(Row, flag), .no = agiru::FieldNo{3}, .type = agiru::FieldType::Boolean},
    agiru::FieldDef{
        .offset = offsetof(Row, text), .no = agiru::FieldNo{4}, .type = agiru::FieldType::Text},
    agiru::FieldDef{.offset = offsetof(Row, choice),
                    .values = kChoices,
                    .no = agiru::FieldNo{5},
                    .type = agiru::FieldType::Enum},
    agiru::FieldDef{
        .offset = offsetof(Row, date), .no = agiru::FieldNo{6}, .type = agiru::FieldType::Date},
    agiru::FieldDef{
        .offset = offsetof(Row, time), .no = agiru::FieldNo{7}, .type = agiru::FieldType::Time},
    agiru::FieldDef{.offset = offsetof(Row, instant),
                    .no = agiru::FieldNo{8},
                    .type = agiru::FieldType::DateTime},
    agiru::FieldDef{.offset = offsetof(Row, duration),
                    .no = agiru::FieldNo{9},
                    .type = agiru::FieldType::Duration},
    agiru::FieldDef{.no = agiru::FieldNo{10}, .type = agiru::FieldType::Blob},
    agiru::FieldDef{
        .offset = offsetof(Row, code), .no = agiru::FieldNo{11}, .type = agiru::FieldType::Code},
    agiru::FieldDef{.offset = offsetof(Row, integer),
                    .no = agiru::FieldNo{12},
                    .type = agiru::FieldType::Integer},
    agiru::FieldDef{.offset = offsetof(Row, option),
                    .values = kChoices,
                    .no = agiru::FieldNo{13},
                    .type = agiru::FieldType::Option},
    agiru::FieldDef{
        .offset = offsetof(Row, guid), .no = agiru::FieldNo{14}, .type = agiru::FieldType::Guid},
    agiru::FieldDef{.offset = offsetof(Row, formula),
                    .no = agiru::FieldNo{15},
                    .type = agiru::FieldType::DateFormula},
    agiru::FieldDef{.offset = offsetof(Row, record),
                    .no = agiru::FieldNo{16},
                    .type = agiru::FieldType::RecordId},
    agiru::FieldDef{.no = agiru::FieldNo{17}, .type = agiru::FieldType::Media},
    agiru::FieldDef{.no = agiru::FieldNo{18}, .type = agiru::FieldType::MediaSet},
    agiru::FieldDef{.no = agiru::FieldNo{19}, .type = agiru::FieldType::TableFilter},
    agiru::FieldDef{.no = agiru::FieldNo{20},
                    .fieldClass = agiru::FieldClass::FlowFilter,
                    .type = agiru::FieldType::Decimal},
};
constexpr agiru::TableDef kTable{.id = agiru::TableId{50399}, .fields = kFields};

agiru::PageValue Value(const Row &row, std::size_t index) {
  return agiru::ReadPageValue(&row, kTable, kFields.at(index));
}

void ExactValues() {
  Row row;
  CHECK_TEXT(
      "decimal carries its type, not a numeric-looking caption", Value(row, 0).type, "Decimal");
  CHECK_TEXT("decimal scale survives the machine boundary", Value(row, 0).value, "1.2300");
  row.amount = agiru::Decimal::MaxValue();
  CHECK_TEXT("full CLR mantissa survives", Value(row, 0).value, "79228162514264337593543950335");
  CHECK_TEXT(
      "Int64 maximum does not traverse a double", Value(row, 1).value, "9223372036854775807");
  row.large = std::numeric_limits<agiru::BigInteger>::min();
  CHECK_TEXT("Int64 minimum survives", Value(row, 1).value, "-9223372036854775808");
  CHECK_TEXT("Boolean has a culture-independent machine token", Value(row, 2).value, "true");
  row.flag = false;
  CHECK_TEXT("false is not the display caption No", Value(row, 2).value, "false");
  CHECK_TEXT(
      "Unicode and markup-looking text remain exact data", Value(row, 3).value, row.text.Value());
  const auto choice = Value(row, 4);
  CHECK_TEXT("sparse Enum carries its declared ordinal", choice.value, "10");
  CHECK_TEXT(
      "Enum domain is a stable qualified field identity", choice.domain, "table/50399/field/5");
  CHECK_TEXT("Enum member identity is not its translated caption", choice.member, "After");
  row.choice = agiru::Enum<>{kUndeclaredOrdinal};
  CHECK_TRUE("undeclared extensible ordinals are not mapped to a fabricated member",
             Value(row, 4).value == "99" && Value(row, 4).member.empty());
  CHECK_TEXT("date retains its invariant calendar value", Value(row, 5).value, "2026-10-06");
  CHECK_TRUE("closing date is distinct from its normal twin", Value(row, 5).closing);
  row.date = row.date.Normal();
  CHECK_TRUE("normal date does not acquire a closing marker", !Value(row, 5).closing);
  row.date = {};
  CHECK_TRUE("undefined date remains distinct from blank display", Value(row, 5).undefined);
  CHECK_TEXT("time retains milliseconds", Value(row, 6).value, "12:34:56.789");
  CHECK_TEXT(
      "DateTime retains UTC and milliseconds", Value(row, 7).value, "2026-10-06T12:34:56.789Z");
  CHECK_TEXT(
      "Duration carries signed Int64 milliseconds", Value(row, 8).value, "-9223372036854775808");
  row.time = {};
  row.instant = {};
  CHECK_TRUE("undefined temporal values have explicit flags",
             Value(row, 6).undefined && Value(row, 7).undefined);
}

void Refusals() {
  const Row row;
  bool refused = false;
  try {
    static_cast<void>(Value(row, kBlobIndex));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueUnsupported"; }
  CHECK_TRUE("binary payload is not silently serialized as a scalar", refused);
  refused = false;
  const auto copied = kFields.front();
  try {
    static_cast<void>(agiru::ReadPageValue(&row, kTable, copied));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueDeclaration"; }
  CHECK_TRUE("foreign declarations do not access guessed storage", refused);
  refused = false;
  try {
    static_cast<void>(agiru::ReadPageValue(nullptr, kTable, kFields.front()));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueDeclaration"; }
  CHECK_TRUE("null storage refuses", refused);
  refused = false;
  try {
    static_cast<void>(Value(row, kFilterIndex));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueUnsupported"; }
  CHECK_TRUE("FlowFilter does not fabricate a scalar from unused record storage", refused);
}

void RemainingScalars() {
  const Row row;
  CHECK_TRUE("Code keeps its normalized value and distinct type",
             Value(row, 10).type == "Code" && Value(row, 10).value == "ABC");
  CHECK_TRUE("Integer remains a distinct integral type",
             Value(row, 11).type == "Integer" && Value(row, 11).value == "-123");
  CHECK_TRUE("Option preserves its domain and declared member independently of captions",
             Value(row, 12).type == "Option" && Value(row, 12).domain == "table/50399/field/13" &&
                 Value(row, 12).value == "10" && Value(row, 12).member == "After");
  CHECK_TEXT("Guid uses canonical UUID storage text",
             Value(row, 13).value,
             "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE");
  CHECK_TEXT("DateFormula preserves language-independent storage text",
             Value(row, 14).value,
             row.formula.ToStorageText());
  const auto record = Value(row, 15);
  CHECK_TRUE("RecordId has explicit printable opaque encoding",
             record.type == "RecordId" && record.value.starts_with("base64:"));
  CHECK_TEXT("opaque identity retains exact round-tripping bytes",
             agiru::DecodeBase64(record.value.substr(7)),
             row.record.ToStorageText());
  for (const std::size_t index : {16U, 17U, 18U}) {
    bool refused = false;
    try {
      static_cast<void>(Value(row, index));
    } catch (const agiru::Error &error) { refused = error.Code() == "PageValueUnsupported"; }
    CHECK_TRUE("non-scalar media and table filters refuse explicitly", refused);
  }
}

void VariableValues() {
  const Row row;
  constexpr std::string_view kDomain = "page/50352/control/Scalar";
  CHECK_TRUE("page Decimal preserves its declared type and scale",
             agiru::ReadPageVariable(row.amount, kDomain).type == "Decimal" &&
                 agiru::ReadPageVariable(row.amount, kDomain).value == "1.2300");
  CHECK_TEXT("page Int64 keeps all digits",
             agiru::ReadPageVariable(row.large, kDomain).value,
             "9223372036854775807");
  CHECK_TEXT("page Unicode text is not reparsed",
             agiru::ReadPageVariable(row.text, kDomain).value,
             row.text.Value());
  CHECK_TEXT("page Code retains its AL normalization",
             agiru::ReadPageVariable(row.code, kDomain).value,
             "ABC");
  CHECK_TRUE("page closing Date retains both flags",
             agiru::ReadPageVariable(row.date, kDomain).closing &&
                 !agiru::ReadPageVariable(row.date, kDomain).undefined);
  CHECK_TRUE("page sparse Enum uses its declared storage identity",
             agiru::ReadPageVariable(row.choice, kDomain).type == "Enum" &&
                 agiru::ReadPageVariable(row.choice, kDomain).value == "10" &&
                 agiru::ReadPageVariable(row.choice, kDomain).domain == kDomain);
  bool refused = false;
  try {
    static_cast<void>(agiru::ReadPageVariable(agiru::Action::OK, kDomain));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueUnsupported"; }
  CHECK_TRUE("undeclared platform enum transport refuses rather than guessing", refused);
  refused = false;
  try {
    static_cast<void>(agiru::ReadPageVariable('x', kDomain));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueUnsupported"; }
  CHECK_TRUE("character storage cannot be read as a wider Integer", refused);
  refused = false;
  try {
    static_cast<void>(agiru::ReadPageVariable(row.integer, {}));
  } catch (const agiru::Error &error) { refused = error.Code() == "PageValueDeclaration"; }
  CHECK_TRUE("standalone scalar requires its declaration identity", refused);
}

}

int main() {
  return gate::Run("PageValue", [] {
    ExactValues();
    Refusals();
    RemainingScalars();
    VariableValues();
  });
}
