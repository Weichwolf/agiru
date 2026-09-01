#include "agiru/Table.h"

#include "agiru/BigInteger.h"
#include "agiru/Boolean.h"
#include "agiru/Date.h"
#include "agiru/Decimal.h"
#include "agiru/EnumDef.h"
#include "agiru/Error.h"
#include "agiru/Ids.h"
#include "agiru/Integer.h"
#include "agiru/Record.h"
#include "agiru/Session.h"
#include "agiru/StringValue.h"
#include "agiru/TableDef.h"

#include "Rows.h"

#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace agiru::detail {

class ValueAccess {
public:
  static void Store(StringValue &target, std::string value) { target.Set(std::move(value)); }

  static void Store(OrdinalValue &target, std::int32_t ordinal) { target.SetOrdinal(ordinal); }
};

namespace {

std::byte *At(void *record, const FieldDef &def) {
  return static_cast<std::byte *>(record) + def.offset;
}

// THE COLUMN IS A TIMESTAMP AND THE TIME CARRIES THE CLOSING FLAG, which is what
// `date-data-type.md` describes: 00:00:00 for a normal date, 23:59:59 for a closing one, and the
// earliest date SQL accepts for the undefined one. Written that way rather than a way of our own so
// that a CRONUS database restored from a .bak is READ rather than converted.
std::string DateStorageText(const Date &date) {
  if (date.IsUndefined()) { return "1753-01-01 00:00:00"; }
  return std::format("{:04}-{:02}-{:02} {}",
                     date.Year(),
                     date.Month(),
                     date.Day(),
                     date.IsClosing() ? "23:59:59" : "00:00:00");
}

Date DateFromStorageText(std::string_view text) {
  // `1753-01-01 00:00:00` is the undefined date and nothing else: it is the sentinel the platform
  // itself writes, so a column holding it means 0D rather than that day.
  if (text.size() < 10) { return Date{}; }
  const int year = std::stoi(std::string(text.substr(0, 4)));
  const unsigned month = static_cast<unsigned>(std::stoi(std::string(text.substr(5, 2))));
  const unsigned day = static_cast<unsigned>(std::stoi(std::string(text.substr(8, 2))));
  if (year == Date::kFirstYear && month == 1 && day == 1) { return Date{}; }
  const Date normal = Date::FromYmd(year, month, day);
  return text.find("23:59:59") != std::string_view::npos ? normal.Closing() : normal;
}

std::string StorageText(const void *record, const FieldDef &def) {
  switch (def.type) {
    case FieldType::Option:
    case FieldType::Enum:
      return std::to_string(reinterpret_cast<const OrdinalValue *>(
                                static_cast<const std::byte *>(record) + def.offset)
                                ->AsInteger());
    case FieldType::Date:
      return DateStorageText(
          *reinterpret_cast<const Date *>(static_cast<const std::byte *>(record) + def.offset));
    default: return FieldText(record, def);
  }
}

FieldValues ValuesOf(const void *record, const TableDef &table) {
  FieldValues values;
  values.reserve(table.fields.size());
  for (const FieldDef &def : table.fields) { values.emplace_back(StorageText(record, def)); }
  return values;
}

FieldValues KeyOf(const void *record, const TableDef &table) {
  if (table.keys.empty()) { throw Error("the table declares no key"); }
  FieldValues values;
  for (const FieldNo no : table.keys[0].fields) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { throw Error("the primary key names a field the table lacks"); }
    values.emplace_back(StorageText(record, *def));
  }
  return values;
}

} // namespace

void SetFieldText(void *record, const FieldDef &def, std::string_view text) {
  switch (def.type) {
    case FieldType::Code: {
      std::string normalised = NormaliseCode(text);
      CheckLength(normalised, def.length);
      ValueAccess::Store(*reinterpret_cast<StringValue *>(At(record, def)), std::move(normalised));
      return;
    }
    case FieldType::Text: {
      CheckLength(text, def.length);
      ValueAccess::Store(*reinterpret_cast<StringValue *>(At(record, def)), std::string(text));
      return;
    }
    case FieldType::Decimal:
      *reinterpret_cast<Decimal *>(At(record, def)) = Decimal::FromInvariantString(text);
      return;
    case FieldType::Boolean:
      *reinterpret_cast<Boolean *>(At(record, def)) = text == "t" || text == "true";
      return;
    case FieldType::Integer:
      *reinterpret_cast<Integer *>(At(record, def)) =
          static_cast<Integer>(std::stol(std::string(text)));
      return;
    case FieldType::BigInteger:
      *reinterpret_cast<BigInteger *>(At(record, def)) = std::stoll(std::string(text));
      return;
    case FieldType::Option:
    case FieldType::Enum:
      ValueAccess::Store(*reinterpret_cast<OrdinalValue *>(At(record, def)),
                         std::stoi(std::string(text)));
      return;
    case FieldType::Date:
      *reinterpret_cast<Date *>(At(record, def)) = DateFromStorageText(text);
      return;
    default: throw Error("no reader for this field type yet");
  }
}

void RuntimeInsert(const void *record, const TableDef &table) {
  const FieldValues values = ValuesOf(record, table);
  InsertRow(Session::Current().Database(), table, values);
}

bool RuntimeModify(const void *record, const TableDef &table) {
  const FieldValues values = ValuesOf(record, table);
  return ModifyRow(Session::Current().Database(), table, values);
}

bool RuntimeDelete(const void *record, const TableDef &table) {
  const FieldValues key = KeyOf(record, table);
  return DeleteRow(Session::Current().Database(), table, key);
}

bool RuntimeGet(void *record, const TableDef &table) {
  const FieldValues key = KeyOf(record, table);
  const std::optional<FieldValues> row = GetRow(Session::Current().Database(), table, key);
  if (!row.has_value()) { return false; }
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    SetFieldText(record, table.fields[i], Required((*row)[i], table.fields[i]));
  }
  return true;
}

} // namespace agiru::detail
