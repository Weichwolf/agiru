#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Integer.h"
#include "type/Time.h"

#include "BuiltinsWritten.h"
#include "MetadataText.h"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace agiru::detail {
namespace {

[[noreturn]] void InvalidPosition() {
  throw Error("The record position is invalid.");
}

bool Same(std::string_view left, std::string_view right) {
  const auto fold = [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; };
  return std::ranges::equal(
      left, right, [&](unsigned char a, unsigned char b) { return fold(a) == fold(b); });
}

std::string DoubleQuoted(std::string_view value) {
  std::string result = "\"";
  for (const char c : value) {
    result += c;
    if (c == '"') { result += c; }
  }
  result += '"';
  return result;
}

std::string ConstantLiteral(std::string_view value) {
  if (value == "''") { return {}; }
  if (value.find(')') != std::string_view::npos || TrimWhitespace(value) != value) {
    return DoubleQuoted(value);
  }
  return std::string(value);
}

std::string FieldName(const FieldDef &field, bool useNames) {
  if (!useNames) { return "Field" + std::to_string(field.no.Value()); }
  const std::string_view caption = field.caption.empty() ? field.name : field.caption;
  return caption.find('=') == std::string_view::npos ? std::string(caption) : DoubleQuoted(caption);
}

const FieldDef &KeyField(const TableDef &table, FieldNo number) {
  const FieldDef *field = Field(table, number);
  if (field == nullptr) { throw Error("The primary key names an undeclared field."); }
  return *field;
}

template <typename T> const T &ValueAt(const void *record, const FieldDef &field) {
  return *reinterpret_cast<const T *>(static_cast<const std::byte *>(record) + field.offset);
}

std::string
PositionValue(const void *record, const TableDef &table, const FieldDef &field, bool useNames) {
  if (field.type == FieldType::Option || field.type == FieldType::Enum) {
    const std::int32_t ordinal = ValueAt<OrdinalValue>(record, field).AsInteger();
    const EnumValueDef *member = ValueOf(field.values, ordinal);
    if (!useNames || member == nullptr || member->name.empty()) { return std::to_string(ordinal); }
    return std::string(member->name);
  }
  if (field.type == FieldType::Boolean && !useNames) {
    return ValueAt<Boolean>(record, field) ? "1" : "0";
  }
  if (field.type == FieldType::Decimal) { return FieldText(record, field); }
  if (field.type == FieldType::Date && ValueAt<Date>(record, field).IsClosing()) {
    return "C" + FieldFormat(record, table, field.no);
  }
  return FieldFormat(record, table, field.no);
}

class PositionReader {
public:
  explicit PositionReader(std::string_view input) : remaining_(input) {}

  std::string Assignment(const FieldDef &field) {
    SkipSpace();
    const std::string name = Token('=');
    if (!Same(name, field.name) && (field.caption.empty() || !Same(name, field.caption)) &&
        !Same(name, "Field" + std::to_string(field.no.Value()))) {
      InvalidPosition();
    }
    Expect('=');
    const std::string kind = Token('(');
    if (!Same(kind, "CONST") && kind != "0") { InvalidPosition(); }
    Expect('(');
    const std::string value = Token(')');
    Expect(')');
    return value;
  }

  void Separator() { Expect(','); }

  void End() {
    SkipSpace();
    if (!remaining_.empty()) { InvalidPosition(); }
  }

private:
  void SkipSpace() {
    while (const std::size_t size = WhitespacePrefix(remaining_)) {
      remaining_.remove_prefix(size);
    }
  }

  void Expect(char c) {
    SkipSpace();
    if (remaining_.empty() || remaining_.front() != c) { InvalidPosition(); }
    remaining_.remove_prefix(1);
  }

  std::string Quoted() {
    remaining_.remove_prefix(1);
    std::string result;
    while (!remaining_.empty()) {
      const char c = remaining_.front();
      remaining_.remove_prefix(1);
      if (c != '"') {
        result += c;
        continue;
      }
      if (remaining_.empty() || remaining_.front() != '"') { return result; }
      remaining_.remove_prefix(1);
      result += '"';
    }
    InvalidPosition();
  }

  std::string Token(char delimiter) {
    SkipSpace();
    if (!remaining_.empty() && remaining_.front() == '"') { return Quoted(); }
    const std::size_t end = remaining_.find(delimiter);
    if (end == std::string_view::npos) { InvalidPosition(); }
    const std::string result(TrimWhitespace(remaining_.substr(0, end)));
    remaining_.remove_prefix(end);
    return result;
  }

  std::string_view remaining_;
};

template <typename T> T WholeValue(std::string_view text) {
  if (text.empty()) { return {}; }
  T value{};
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size()) { InvalidPosition(); }
  return value;
}

template <typename T>
void AssignEvaluated(void *record, const FieldDef &field, std::string_view text) {
  T value{};
  if (!Evaluated(value, text)) { InvalidPosition(); }
  *reinterpret_cast<T *>(static_cast<std::byte *>(record) + field.offset) = value;
}

template <typename T> void AssignWhole(void *record, const FieldDef &field, std::string_view text) {
  *reinterpret_cast<T *>(static_cast<std::byte *>(record) + field.offset) = WholeValue<T>(text);
}

void AssignDate(void *record, const FieldDef &field, std::string_view text) {
  const bool closing = text.starts_with('C');
  if (closing) { text.remove_prefix(1); }
  Date value{};
  if (!Evaluated(value, text)) { InvalidPosition(); }
  if (closing) { value = value.Closing(); }
  *reinterpret_cast<Date *>(static_cast<std::byte *>(record) + field.offset) = value;
}

std::int32_t PositionOrdinal(const FieldDef &field, std::string_view text) {
  if (!text.empty() && text.find_first_not_of("-+0123456789") == std::string_view::npos) {
    return WholeValue<std::int32_t>(text);
  }
  const auto value = MemberOrdinalOf(field.values, text);
  if (!value.has_value()) { InvalidPosition(); }
  return *value;
}

void AssignPosition(void *record, const FieldDef &field, std::string_view text) {
  switch (field.type) {
    case FieldType::Decimal: AssignEvaluated<Decimal>(record, field, text); return;
    case FieldType::Integer: AssignWhole<Integer>(record, field, text); return;
    case FieldType::BigInteger: AssignWhole<BigInteger>(record, field, text); return;
    case FieldType::Boolean: AssignEvaluated<Boolean>(record, field, text); return;
    case FieldType::Date: AssignDate(record, field, text); return;
    case FieldType::Time: AssignEvaluated<Time>(record, field, text); return;
    case FieldType::DateTime: AssignEvaluated<DateTime>(record, field, text); return;
    case FieldType::Duration: {
      *reinterpret_cast<Duration *>(static_cast<std::byte *>(record) + field.offset) =
          Duration{WholeValue<std::int64_t>(text)};
      return;
    }
    case FieldType::Option:
    case FieldType::Enum:
      SetFieldText(record, field, std::to_string(PositionOrdinal(field, text)));
      return;
    default: SetFieldText(record, field, text); return;
  }
}

}

std::string PositionText(const void *record, const TableDef &table, bool useNames) {
  std::string result;
  if (table.keys.empty()) { return result; }
  for (const FieldNo number : table.keys.front().fields) {
    const FieldDef &field = KeyField(table, number);
    if (!result.empty()) { result += ','; }
    result += FieldName(field, useNames);
    result += useNames ? "=CONST(" : "=0(";
    result += ConstantLiteral(PositionValue(record, table, field, useNames));
    result += ')';
  }
  return result;
}

void TakePosition(void *record, const TableDef &table, std::string_view position) {
  PositionReader reader(position);
  if (table.keys.empty()) {
    reader.End();
    return;
  }

  struct Assignment {
    const FieldDef *field;
    std::string value;
  };

  std::vector<Assignment> assignments;
  assignments.reserve(table.keys.front().fields.size());
  for (const FieldNo number : table.keys.front().fields) {
    if (!assignments.empty()) { reader.Separator(); }
    const FieldDef &field = KeyField(table, number);
    assignments.push_back({&field, reader.Assignment(field)});
  }
  reader.End();
  for (const auto &assignment : assignments) {
    AssignPosition(record, *assignment.field, assignment.value);
  }
  RecordState &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  SelectionChanged(state);
  state.positioned = true;
}

}
