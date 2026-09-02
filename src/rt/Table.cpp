#include "runtime/Table.h"

#include "Rows.h"
#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Time.h"

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
// earliest date SQL accepts for the undefined one. The reason to follow it is not tidiness: a plain
// `date` column has nowhere to put the closing bit, so it would collapse a closing date onto its
// normal one and lose the ordering a fiscal-year close is built on.
std::string DateStorageText(const Date &date) {
  if (date.IsUndefined()) { return "1753-01-01 00:00:00"; }
  return std::format("{:04}-{:02}-{:02} {}",
                     date.Year(),
                     date.Month(),
                     date.Day(),
                     date.IsClosing() ? "23:59:59" : "00:00:00");
}

struct IsoDate {
  static constexpr std::size_t kYear = 0;
  static constexpr std::size_t kMonth = 5;
  static constexpr std::size_t kDay = 8;
  static constexpr std::size_t kYearDigits = 4;
  static constexpr std::size_t kDigits = 2;
  static constexpr std::size_t kWidth = 10;
};

Date DateFromStorageText(std::string_view text) {
  // 1753-01-01 IS THE UNDEFINED DATE, ALL OF IT. A date column carries only two instants per day,
  // 00:00:00 and 23:59:59, and both of that day's are spent on the sentinel the platform writes for
  // 0D. So the whole day reads back as undefined here -- unlike a datetime column, where only the
  // one instant is reserved.
  if (text.size() < IsoDate::kWidth) { return Date{}; }
  const int year = std::stoi(std::string(text.substr(IsoDate::kYear, IsoDate::kYearDigits)));
  const auto month =
      static_cast<unsigned>(std::stoi(std::string(text.substr(IsoDate::kMonth, IsoDate::kDigits))));
  const auto day =
      static_cast<unsigned>(std::stoi(std::string(text.substr(IsoDate::kDay, IsoDate::kDigits))));
  if (year == Date::kFirstYear && month == 1 && day == 1) { return Date{}; }
  const Date normal = Date::FromYmd(year, month, day);
  return text.find("23:59:59") != std::string_view::npos ? normal.Closing() : normal;
}

// `hh:mm:ss[.fff]`, which is what a `time` column hands back. The offsets are the ISO layout and
// are named rather than counted at each use.
struct IsoTime {
  static constexpr std::size_t kHour = 0;
  static constexpr std::size_t kMinute = 3;
  static constexpr std::size_t kSecond = 6;
  static constexpr std::size_t kPoint = 8;
  static constexpr std::size_t kFraction = 9;
  static constexpr std::size_t kWidth = 8;
  static constexpr std::size_t kDigits = 2;
  static constexpr std::size_t kFractionDigits = 3;
};

Time TimeFromStorageText(std::string_view text) {
  if (text.size() < IsoTime::kWidth) { return Time{}; }
  const int hour = std::stoi(std::string(text.substr(IsoTime::kHour, IsoTime::kDigits)));
  const int minute = std::stoi(std::string(text.substr(IsoTime::kMinute, IsoTime::kDigits)));
  const int second = std::stoi(std::string(text.substr(IsoTime::kSecond, IsoTime::kDigits)));
  int millisecond = 0;
  if (text.size() > IsoTime::kFraction && text[IsoTime::kPoint] == '.') {
    std::string fraction(text.substr(IsoTime::kFraction));
    fraction.resize(IsoTime::kFractionDigits, '0');
    millisecond = std::stoi(fraction);
  }
  return Time::FromHms(hour, minute, second, millisecond);
}

std::string DateTimeStorageText(const DateTime &instant) {
  if (instant.IsUndefined()) { return "1753-01-01 00:00:00.000"; }
  return instant.Date().ToInvariantString() + " " + instant.Time().ToInvariantString();
}

// `yyyy-mm-dd hh:mm:ss[.fff]`, which is what a `timestamp` column hands back. The undefined
// DateTime and the earliest instant AL accepts are the same value, so the sentinel needs no
// special case: 1753-01-01 00:00:00 lands on zero on its own.
DateTime DateTimeFromStorageText(std::string_view text) {
  constexpr std::size_t kNarrowest = 19;
  constexpr std::size_t kTimeAt = 11;
  if (text.size() < kNarrowest) { return DateTime{}; }
  // NOT DateFromStorageText(), and the difference is a whole day. A DATE column carries the
  // sentinel across the whole of 1753-01-01, because that day's normal and closing timestamps are
  // both reserved for the undefined date. A DATETIME column reserves one INSTANT --
  // 1753-01-01 00:00:00.000 -- and 1753-01-01 12:00:00 on that same day is an ordinary value.
  // DateTime::Create() lands it on zero of its own accord, so there is no sentinel test here.
  return DateTime::Create(
      Date::FromYmd(std::stoi(std::string(text.substr(IsoDate::kYear, IsoDate::kYearDigits))),
                    static_cast<unsigned>(
                        std::stoi(std::string(text.substr(IsoDate::kMonth, IsoDate::kDigits)))),
                    static_cast<unsigned>(
                        std::stoi(std::string(text.substr(IsoDate::kDay, IsoDate::kDigits))))),
      TimeFromStorageText(text.substr(kTimeAt)));
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
    // A `timestamp` column takes the ISO form with a space rather than the `T` and without the `Z`
    // the XML format carries: the column has no time zone, and handing it one would have the server
    // read a zone it then discards.
    case FieldType::DateTime:
      return DateTimeStorageText(
          *reinterpret_cast<const DateTime *>(static_cast<const std::byte *>(record) + def.offset));
    // A `uuid` column writes and reads the hyphenated form WITHOUT braces. The braces belong to
    // AL's text and not to the value, so they go on and come off at this boundary.
    case FieldType::Guid:
      return reinterpret_cast<const Guid *>(static_cast<const std::byte *>(record) + def.offset)
          ->ToStorageText();
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
    case FieldType::Time:
      *reinterpret_cast<Time *>(At(record, def)) = TimeFromStorageText(text);
      return;
    case FieldType::DateTime:
      *reinterpret_cast<DateTime *>(At(record, def)) = DateTimeFromStorageText(text);
      return;
    case FieldType::Guid: *reinterpret_cast<Guid *>(At(record, def)) = Guid::FromText(text); return;
    default: throw Error("no reader for this field type yet");
  }
}

namespace {

template <typename T> T *SystemField(void *record, const TableDef &table, FieldNo no) {
  const FieldDef *def = Field(table, no);
  return def != nullptr ? reinterpret_cast<T *>(static_cast<std::byte *>(record) + def->offset)
                        : nullptr;
}

void StampModified(void *record, const TableDef &table, const DateTime &now, const Guid &user) {
  if (auto *at = SystemField<DateTime>(record, table, SystemFieldNumbers::SystemModifiedAt);
      at != nullptr) {
    *at = now;
  }
  if (auto *by = SystemField<Guid>(record, table, SystemFieldNumbers::SystemModifiedBy);
      by != nullptr) {
    *by = user;
  }
}

void StampInserted(void *record, const TableDef &table) {
  const DateTime now = CurrentDateTime();
  const Guid &user = Session::Current().UserSecurityId();
  if (auto *id = SystemField<Guid>(record, table, SystemFieldNumbers::SystemId); id != nullptr) {
    *id = Guid::Create();
  }
  if (auto *at = SystemField<DateTime>(record, table, SystemFieldNumbers::SystemCreatedAt);
      at != nullptr) {
    *at = now;
  }
  if (auto *by = SystemField<Guid>(record, table, SystemFieldNumbers::SystemCreatedBy);
      by != nullptr) {
    *by = user;
  }
  StampModified(record, table, now, user);
}

} // namespace

void RuntimeInsert(void *record, const TableDef &table) {
  StampInserted(record, table);
  const FieldValues values = ValuesOf(record, table);
  InsertRow(Session::Current().Database(), table, values);
}

bool RuntimeModify(void *record, const TableDef &table) {
  StampModified(record, table, CurrentDateTime(), Session::Current().UserSecurityId());
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
