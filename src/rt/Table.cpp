#include "runtime/Table.h"

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Relation.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Media.h"
#include "type/MediaSet.h"
#include "type/RecordId.h"
#include "type/Refusal.h"
#include "type/StringValue.h"
#include "type/Time.h"

#include "BuiltinsWritten.h"
#include "Filter.h"
#include "Rows.h"
#include "Selection.h"
#include "Temporary.h"
#include "Where.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <expected>
#include <format>
#include <map>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

Found::~Found() noexcept(false) {
  if (read_ || found_ || std::uncaught_exceptions() != 0) { return; }
  if (!key_.empty()) {
    throw Error("The " + std::string(table_) +
                " does not exist. Identification fields and values: " + key_);
  }
  throw Error("There is no " + std::string(table_) + " within the filter.");
}

class ValueAccess {
public:
  static void Store(StringValue &target, std::string value) { target.Set(std::move(value)); }

  static void Store(OrdinalValue &target, std::int32_t ordinal) { target.SetOrdinal(ordinal); }
};

namespace {

std::byte *At(void *record, const FieldDef &def) {
  return static_cast<std::byte *>(record) + def.offset;
}

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

DateTime DateTimeFromStorageText(std::string_view text) {
  constexpr std::size_t kNarrowest = 19;
  constexpr std::size_t kTimeAt = 11;
  if (text.size() < kNarrowest) { return DateTime{}; }
  return DateTime::Create(
      Date::FromYmd(std::stoi(std::string(text.substr(IsoDate::kYear, IsoDate::kYearDigits))),
                    static_cast<unsigned>(
                        std::stoi(std::string(text.substr(IsoDate::kMonth, IsoDate::kDigits)))),
                    static_cast<unsigned>(
                        std::stoi(std::string(text.substr(IsoDate::kDay, IsoDate::kDigits))))),
      TimeFromStorageText(text.substr(kTimeAt)));
}

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
    case FieldType::DateTime:
      return DateTimeStorageText(
          *reinterpret_cast<const DateTime *>(static_cast<const std::byte *>(record) + def.offset));
    case FieldType::Guid:
      return reinterpret_cast<const Guid *>(static_cast<const std::byte *>(record) + def.offset)
          ->ToStorageText();
    case FieldType::RecordId:
      return reinterpret_cast<const RecordId *>(static_cast<const std::byte *>(record) + def.offset)
          ->ToStorageText();
    default: return FieldText(record, def);
  }
}

namespace {

FieldValues ValuesOf(const void *record, const TableDef &table) {
  FieldValues values;
  values.reserve(table.fields.size());
  for (const FieldDef &def : table.fields) {
    if (!Stored(def)) { continue; }
    values.emplace_back(StorageText(record, def));
  }
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

}

constexpr int kHexBase = 16;
constexpr int kDecimalBase = 10;
constexpr int kMostPlaces = 18;

namespace {

template <typename T> T Read(std::string_view text, const FieldDef &def) {
  if (text.empty()) { return T{}; }
  const std::expected<T, Refusal> got = T::FromText(text);
  if (got.has_value()) { return *got; }
  if constexpr (std::is_same_v<T, DateFormula>) {
    static_cast<void>(def);
    return T{};
  } else {
    throw Error("the column of " + std::string(def.name) + " holds " + std::string(text) +
                ", and " + std::string(got.error().what));
  }
}

}

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
      *reinterpret_cast<Decimal *>(At(record, def)) =
          text.find_first_not_of(' ') == std::string_view::npos
              ? Decimal{}
              : Decimal::FromInvariantString(text);
      return;
    case FieldType::Boolean:
      *reinterpret_cast<Boolean *>(At(record, def)) =
          text == "t" || text == "true" || text == "1" || text == "Yes" || text == "yes";
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
                         std::stoi(detail::MemberOrdinal(def, text)));
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
    case FieldType::Guid:
      *reinterpret_cast<Guid *>(At(record, def)) = Read<Guid>(text, def);
      return;
    case FieldType::Duration:
      *reinterpret_cast<Duration *>(At(record, def)) = Duration{std::stoll(std::string(text))};
      return;
    case FieldType::DateFormula:
      *reinterpret_cast<DateFormula *>(At(record, def)) = Read<DateFormula>(text, def);
      return;
    case FieldType::Media:
      *reinterpret_cast<Media *>(At(record, def)) = Media{Read<Guid>(text, def)};
      return;
    case FieldType::MediaSet:
      *reinterpret_cast<MediaSet *>(At(record, def)) = MediaSet{Read<Guid>(text, def)};
      return;
    case FieldType::Blob: {
      std::vector<std::uint8_t> bytes;
      const std::string_view hex = text.starts_with("\\x") ? text.substr(2) : std::string_view{};
      bytes.reserve(hex.size() / 2);
      for (std::size_t at = 0; at + 1 < hex.size(); at += 2) {
        bytes.push_back(static_cast<std::uint8_t>(
            std::stoul(std::string(hex.substr(at, 2)), nullptr, kHexBase)));
      }
      reinterpret_cast<Blob *>(At(record, def))->Set(std::move(bytes));
      return;
    }
    case FieldType::RecordId: {
      const std::expected<RecordId, Refusal> read = RecordId::FromStorageText(text);
      if (!read.has_value()) {
        throw Error("the column of " + std::string(def.name) + " holds " + std::string(text) +
                    ", and " + std::string(read.error().what));
      }
      *reinterpret_cast<RecordId *>(At(record, def)) = *read;
      return;
    }
    case FieldType::TableFilter:
    default: throw Error("no reader for the field type of " + std::string(def.name) + " yet");
  }
}

namespace {

void ClearField(void *record, const FieldDef &def) {
  switch (def.type) {
    case FieldType::Code:
    case FieldType::Text:
      ValueAccess::Store(*reinterpret_cast<StringValue *>(At(record, def)), std::string{});
      return;
    case FieldType::Decimal: *reinterpret_cast<Decimal *>(At(record, def)) = Decimal{}; return;
    case FieldType::Boolean: *reinterpret_cast<Boolean *>(At(record, def)) = false; return;
    case FieldType::Integer: *reinterpret_cast<Integer *>(At(record, def)) = 0; return;
    case FieldType::BigInteger: *reinterpret_cast<BigInteger *>(At(record, def)) = 0; return;
    case FieldType::Option:
    case FieldType::Enum:
      ValueAccess::Store(*reinterpret_cast<OrdinalValue *>(At(record, def)), 0);
      return;
    case FieldType::Date: *reinterpret_cast<Date *>(At(record, def)) = Date{}; return;
    case FieldType::Time: *reinterpret_cast<Time *>(At(record, def)) = Time{}; return;
    case FieldType::DateTime: *reinterpret_cast<DateTime *>(At(record, def)) = DateTime{}; return;
    case FieldType::Duration: *reinterpret_cast<Duration *>(At(record, def)) = Duration{}; return;
    case FieldType::Guid: *reinterpret_cast<Guid *>(At(record, def)) = Guid{}; return;
    case FieldType::DateFormula:
      *reinterpret_cast<DateFormula *>(At(record, def)) = DateFormula{};
      return;
    case FieldType::Blob: *reinterpret_cast<Blob *>(At(record, def)) = Blob{}; return;
    case FieldType::Media: *reinterpret_cast<Media *>(At(record, def)) = Media{}; return;
    case FieldType::MediaSet: *reinterpret_cast<MediaSet *>(At(record, def)) = MediaSet{}; return;
    case FieldType::RecordId: *reinterpret_cast<RecordId *>(At(record, def)) = RecordId{}; return;
    default:
      throw Error("Init: field " + std::string(def.name) +
                  " has a type this runtime cannot return to its default yet");
  }
}

}

std::int32_t &Validating() {
  static thread_local std::int32_t field = 0;
  return field;
}

ValidatingField::ValidatingField(::agiru::FieldNo no) : was_(Validating()) {
  Validating() = no.Value();
}

ValidatingField::~ValidatingField() {
  Validating() = was_;
}

void ClearKeyField(void *record, const TableDef &table, std::size_t position) {
  const FieldDef *def = Field(table, table.keys[0].fields[position]);
  if (def == nullptr) { throw Error("Get: the primary key names a field the table lacks"); }
  ClearField(record, *def);
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

}

namespace {

void InitValuesOnly(void *record, const TableDef &table) {
  for (const FieldDef &def : table.fields) {
    if (def.initValue.has_value()) { SetFieldText(record, def, *def.initValue); }
  }
}

void Defaulted(void *record, const TableDef &table, bool sparePrimaryKey) {
  const std::span<const FieldNo> key =
      table.keys.empty() ? std::span<const FieldNo>{} : table.keys[0].fields;
  for (const FieldDef &def : table.fields) {
    if (sparePrimaryKey && std::ranges::find(key, def.no) != key.end()) { continue; }
    if (def.initValue.has_value()) {
      SetFieldText(record, def, *def.initValue);
    } else {
      ClearField(record, def);
    }
  }
}

}

namespace {

bool Convertible(FieldType into, FieldType from) {
  if (into == from) { return true; }
  const auto textLike = [](FieldType type) {
    return type == FieldType::Text || type == FieldType::Code;
  };
  const auto ordinal = [](FieldType type) {
    return type == FieldType::Option || type == FieldType::Enum;
  };
  return (textLike(into) && textLike(from)) || (ordinal(into) && ordinal(from));
}
}

namespace {

bool InPrimaryKey(const TableDef &table, FieldNo no) {
  return !table.keys.empty() &&
         std::ranges::any_of(table.keys[0].fields, [no](FieldNo held) { return held == no; });
}
}

std::string PositionText(const void *record, const TableDef &table, bool useNames) {
  std::string out;
  if (table.keys.empty()) { return out; }
  for (const FieldNo no : table.keys[0].fields) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { continue; }
    std::string value = StorageText(record, *def);
    std::string quoted;
    for (const char c : value) {
      quoted += c;
      if (c == '\'') { quoted += c; }
    }
    if (!out.empty()) { out += ','; }
    out += useNames ? std::string(def->name) : std::to_string(no.Value());
    out += "='" + quoted + "'";
  }
  return out;
}

void TakePosition(void *record, const TableDef &table, std::string_view position) {
  std::size_t at = 0;
  while (at < position.size()) {
    const std::size_t equals = position.find('=', at);
    if (equals == std::string_view::npos) { break; }
    const std::string_view key = position.substr(at, equals - at);
    std::size_t i = equals + 1;
    std::string value;
    if (i < position.size() && position[i] == '\'') {
      ++i;
      while (i < position.size()) {
        if (position[i] == '\'') {
          if (i + 1 < position.size() && position[i + 1] == '\'') {
            value += '\'';
            i += 2;
            continue;
          }
          ++i;
          break;
        }
        value += position[i];
        ++i;
      }
    } else {
      while (i < position.size() && position[i] != ',') {
        value += position[i];
        ++i;
      }
    }
    while (i < position.size() && position[i] != ',') { ++i; }
    at = i + 1;
    const FieldDef *def = nullptr;
    if (!key.empty() && std::isdigit(static_cast<unsigned char>(key.front())) != 0) {
      def = Field(table, FieldNo{static_cast<std::int32_t>(std::stol(std::string(key)))});
    } else {
      for (const FieldDef &one : table.fields) {
        if (one.name == key) { def = &one; }
      }
    }
    if (def == nullptr) {
      throw Error("Record.SetPosition: " + std::string(table.name) + " carries no field " +
                  std::string(key));
    }
    SetFieldText(record, *def, value);
  }
}

void RuntimeTransferFields(void *into,
                           const TableDef &table,
                           const void *from,
                           const TableDef &source,
                           bool withPrimaryKey,
                           bool skipMismatchingTypes) {
  for (const FieldDef &target : table.fields) {
    if (!withPrimaryKey && InPrimaryKey(table, target.no)) { continue; }
    const FieldDef *held = Field(source, target.no);
    if (held == nullptr) { continue; }
    if (!Convertible(target.type, held->type)) {
      if (skipMismatchingTypes) { continue; }
      throw Error("TransferFields: " + std::string(table.name) + "." + std::string(target.name) +
                  " and " + std::string(source.name) + "." + std::string(held->name) +
                  " are not the same data type");
    }
    SetFieldText(into, target, StorageText(from, *held));
  }
}

void MarkConsistent(const TableDef &table, bool consistent) {
  Session::Current().Transaction().MarkConsistent(table.name, consistent);
}

namespace {

std::string_view Entered(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
    text.remove_prefix(1);
  }
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
    text.remove_suffix(1);
  }
  return text;
}

std::optional<Decimal> Numeric(std::string_view text) {
  try {
    return Decimal::FromInvariantString(Entered(text));
  } catch (const DecimalError &) { return std::nullopt; }
}

}

void CheckEntryRange(std::string_view text, std::string_view minValue, std::string_view maxValue) {
  if (minValue.empty() && maxValue.empty()) { return; }
  const std::optional<Decimal> entered = Numeric(text);
  if (!entered.has_value()) { return; }
  if (const std::optional<Decimal> low = Numeric(minValue); low.has_value() && *entered < *low) {
    throw Error("The value must be greater than or equal to " + std::string(Entered(minValue)) +
                ". Value: " + std::string(Entered(text)) + ".")
        .Coded("TestValidation");
  }
  if (const std::optional<Decimal> high = Numeric(maxValue); high.has_value() && *entered > *high) {
    throw Error("The value must be less than or equal to " + std::string(Entered(maxValue)) +
                ". Value: " + std::string(Entered(text)) + ".")
        .Coded("TestValidation");
  }
}

std::string RuntimeCurrentCompany() {
  return std::string(Session::Current().CompanyName());
}

std::string RuntimeCurrentKey(const void *record, const TableDef &table) {
  std::string out;
  const auto add = [&out, &table](FieldNo no) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { return; }
    if (!out.empty()) { out += ','; }
    out += def->name;
  };
  const RecordState *state = reinterpret_cast<const StateHandle *>(record)->Peek();
  if (state != nullptr && !state->key.empty()) {
    for (const SortField &sorted : state->key) { add(sorted.field); }
    return out;
  }
  if (!table.keys.empty()) {
    for (const FieldNo no : table.keys.front().fields) { add(no); }
  }
  return out;
}

void RuntimeInitValues(void *record, const TableDef &table) {
  InitValuesOnly(record, table);
}

void RuntimeInit(void *record, const TableDef &table) {
  Defaulted(record, table, true);
}

void RuntimeClear(void *record, const TableDef &table) {
  Defaulted(record, table, false);
}

namespace {

void AutoIncrement(void *record, const TableDef &table) {
  for (const FieldDef &def : table.fields) {
    if (!def.autoIncrement || def.fieldClass != FieldClass::Normal) { continue; }
    if (def.type != FieldType::Integer && def.type != FieldType::BigInteger) { continue; }
    if (!IsBlank(record, def)) { continue; }
    const Result next = Session::Current().Database().Execute(
        "SELECT COALESCE(MAX(" + Quoted(def.name) + "), 0) + 1 FROM " + Name(table));
    const std::optional<std::string_view> value = next.Value(0, 0);
    detail::SetFieldText(record, def, value.has_value() ? std::string(*value) : "1");
  }
}

}

bool RuntimeInsert(void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempInsert(record, table); }

  StampInserted(record, table);
  AutoIncrement(record, table);
  const FieldValues values = ValuesOf(record, table);
  return InsertRow(Session::Current().Database(), table, values);
}

bool RuntimeModify(void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempModify(record, table); }

  StampModified(record, table, CurrentDateTime(), Session::Current().UserSecurityId());
  const FieldValues values = ValuesOf(record, table);
  return ModifyRow(Session::Current().Database(), table, values);
}

bool RuntimeRename(void *record, const void *before, const TableDef &table) {
  if (TempOf(record) != nullptr) {
    if (!TempDelete(const_cast<void *>(before), table)) { return false; }
    return TempInsert(record, table);
  }
  StampModified(record, table, CurrentDateTime(), Session::Current().UserSecurityId());
  const FieldValues values = ValuesOf(record, table);
  const FieldValues oldKey = KeyOf(before, table);
  return RenameRow(Session::Current().Database(), table, values, oldKey);
}

bool RuntimeDelete(const void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempDelete(const_cast<void *>(record), table); }

  const FieldValues key = KeyOf(record, table);
  return DeleteRow(Session::Current().Database(), table, key);
}

namespace {

void LoadRow(void *record, const TableDef &table, const FieldValues &row) {
  std::size_t column = 0;
  for (const FieldDef &def : table.fields) {
    if (!Stored(def)) { continue; }
    SetFieldText(record, def, Required(row[column], def));
    ++column;
  }
}

}

bool RuntimeGet(void *record, const TableDef &table) {
  if (TempOf(record) != nullptr) { return TempGet(record, table); }

  const FieldValues key = KeyOf(record, table);
  const std::optional<FieldValues> row = GetRow(Session::Current().Database(), table, key);
  if (!row.has_value()) { return false; }
  LoadRow(record, table, *row);
  RecordState &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  state.open.Forget();
  state.positioned = true;
  return true;
}

bool RuntimeGetBySystemId(void *record, const TableDef &table, const Guid &systemId) {
  const FieldDef *column = nullptr;
  for (const FieldDef &def : table.fields) {
    if (def.name == "SystemId") { column = &def; }
  }
  if (column == nullptr) {
    throw Error("GetBySystemId: " + std::string(table.name) + " carries no SystemId");
  }
  if (TempOf(record) != nullptr) {
    throw Error("GetBySystemId on a temporary record is not written yet (board:0035)");
  }
  const std::optional<FieldValues> row =
      GetRowWhere(Session::Current().Database(), table, *column, systemId.ToText());
  if (!row.has_value()) { return false; }
  LoadRow(record, table, *row);
  return true;
}

void RuntimeSetRecFilter(void *record, const TableDef &table) {
  RecordState &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  if (table.keys.empty()) { return; }
  for (const FieldNo no : table.keys.front().fields) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { continue; }
    Narrow(state, no, Literally(FieldText(record, *def)));
  }
}

std::string FiltersText(const RecordState *state, const TableDef &table) {
  if (state == nullptr) { return {}; }
  std::string out;
  for (const FieldFilter &one : state->filters) {
    if (one.group != state->group) { continue; }
    const FieldDef *def = Field(table, one.field);
    const std::string_view shown =
        def == nullptr ? std::string_view{} : (def->caption.empty() ? def->name : def->caption);
    out += (out.empty() ? "" : ", ") + std::string(shown) + ": " + one.text;
  }
  return out;
}

namespace {
struct BeforeEntry {
  const void *before;
  const void *owner;
};

std::vector<BeforeEntry> &BeforeStack() {
  static thread_local std::vector<BeforeEntry> stack;
  return stack;
}
}

std::string_view FieldNameOf(const TableDef &table, FieldNo no) {
  for (const FieldDef &def : table.fields) {
    if (def.no.Value() == no.Value()) { return def.name; }
  }
  throw Error("FieldName: the table declares no such field");
}

Decimal DeclaredPlaces(const Decimal &value, const TableDef &table, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr || def->decimalPlaces.empty()) { return value; }
  return DeclaredPlaces(value, def->decimalPlaces);
}

Decimal DeclaredPlaces(const Decimal &value, std::string_view decimalPlaces) {
  std::string_view places = decimalPlaces;
  if (const std::size_t colon = places.find(':'); colon != std::string_view::npos) {
    places = places.substr(colon + 1);
  }
  while (!places.empty() && places.front() == ' ') { places.remove_prefix(1); }
  while (!places.empty() && places.back() == ' ') { places.remove_suffix(1); }
  if (places.empty()) { return value; }
  int most = 0;
  for (const char c : places) {
    if (c < '0' || c > '9') { return value; }
    most = most * kDecimalBase + (c - '0');
    if (most > kMostPlaces) { return value; }
  }
  std::string precision = "0." + std::string(static_cast<std::size_t>(most), '0');
  precision.back() = '1';
  if (most == 0) { precision = "1"; }
  return Round(value, Decimal::FromInvariantString(precision));
}

void CheckRelation(const void *record, const TableDef &table, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr || !def->validateTableRelation) { return; }
  if (def->relationTable.empty() && def->relation.empty()) { return; }
  if (IsBlank(record, *def)) { return; }
  const std::optional<ResolvedRelation> resolved = ResolveRelation(record, table, *def);
  if (!resolved.has_value()) { return; }
  const TableEntry *target = FindTable(resolved->table);
  if (target == nullptr) { return; }
  const TableDef &other = *target->table;
  if (other.tableType == TableType::Temporary || IsPlatformTable(other.id)) { return; }
  const auto named = [&other](std::string_view name) -> const FieldDef * {
    for (const FieldDef &candidate : other.fields) {
      if (candidate.name.size() == name.size() &&
          std::ranges::equal(candidate.name, name, [](unsigned char x, unsigned char y) {
            return std::tolower(x) == std::tolower(y);
          })) {
        return &candidate;
      }
    }
    return nullptr;
  };
  const FieldDef *column = nullptr;
  if (!resolved->field.empty()) {
    column = named(resolved->field);
  } else if (!other.keys.empty() && !other.keys[0].fields.empty()) {
    column = Field(other, other.keys[0].fields.front());
  }
  if (column == nullptr) { return; }
  bool found = false;
  if (resolved->filters.empty()) {
    found = GetRowWhere(Session::Current().Database(), other, *column, StorageText(record, *def))
                .has_value();
  } else {
    void *probe = target->make();
    if (TempOf(probe) != nullptr) {
      target->free(probe);
      return;
    }
    RecordState &state = reinterpret_cast<StateHandle *>(probe)->Ensure();
    Narrow(state, column->no, Literally(FieldText(record, *def)));
    bool applicable = true;
    for (const RelationFilter &filter : resolved->filters) {
      const FieldDef *narrowed = named(filter.field);
      if (narrowed == nullptr) {
        applicable = false;
        break;
      }
      Narrow(state, narrowed->no, filter.text);
    }
    found = !applicable || !RuntimeIsEmpty(probe, other);
    target->free(probe);
  }
  if (found) { return; }
  throw Error("The field " + std::string(def->caption.empty() ? def->name : def->caption) +
              " of table " + std::string(table.caption.empty() ? table.name : table.caption) +
              " contains a value (" + FieldText(record, *def) +
              ") that cannot be found in the related table (" +
              std::string(other.caption.empty() ? other.name : other.caption) + ").");
}

void PushBefore(const void *record, const void *owner) {
  BeforeStack().push_back({record, owner});
}

void PopBefore() {
  if (!BeforeStack().empty()) { BeforeStack().pop_back(); }
}

const void *CurrentBefore() {
  return BeforeStack().empty() ? nullptr : BeforeStack().back().before;
}

const void *OutermostBefore(const void *owner) {
  for (const BeforeEntry &entry : BeforeStack()) {
    if (entry.owner == owner) { return entry.before; }
  }
  return nullptr;
}

}

namespace agiru {

::agiru::Integer CurrFieldNo() {
  return detail::Validating();
}

namespace {

bool SameName(std::string_view a, std::string_view b) {
  return std::ranges::equal(
      a, b, [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
}

const FieldDef *FieldNamed(const TableDef &table, std::string_view name) {
  for (const FieldDef &def : table.fields) {
    if (SameName(def.name, name)) { return &def; }
  }
  return nullptr;
}

const TableDef &TableNamed(std::string_view name, const FieldDef &asked) {
  static std::once_flag once;
  static std::map<std::string, const TableDef *> byName;
  std::call_once(once, [] {
    for (const TableEntry *entry : InstalledTables()) {
      std::string key(entry->table->name);
      for (char &c : key) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
      byName.emplace(std::move(key), entry->table);
    }
  });
  std::string key(name);
  for (char &c : key) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
  const auto found = byName.find(key);
  if (found == byName.end()) {
    throw Error("the CalcFormula of " + std::string(asked.name) + " names the table " +
                std::string(name) + ", which this build does not carry");
  }
  return *found->second;
}

struct FlowTerm {
  enum class How : std::uint8_t {
    Const,
    Filter,
    Field,
    FieldFilter,
    FieldUpperLimit,
    FieldUpperLimitFilter
  };
  std::string target;
  How how = How::Const;
  std::string value;
};

struct FlowFormula {
  enum class Kind : std::uint8_t { Sum, Average, Exist, Count, Min, Max, Lookup };
  Kind kind = Kind::Sum;
  bool reverseSign = false;
  std::string table;
  std::string field;
  std::vector<FlowTerm> terms;
};

class FormulaReader {
public:
  FormulaReader(std::string_view text, const FieldDef &asked) : text_(text), asked_(asked) {}

  FlowFormula Read() {
    FlowFormula made;
    Space();
    if (Take('-')) { made.reverseSign = true; }
    const std::string kind = Word();
    made.kind = KindOf(kind);
    Expect('(');
    made.table = Name();
    Space();
    if (Take('.')) { made.field = Name(); }
    Space();
    if (SameName(Peek(), "where")) {
      Word();
      Expect('(');
      ReadTerms(made);
    }
    Space();
    Expect(')');
    return made;
  }

private:
  void ReadTerms(FlowFormula &made) {
    {
      for (;;) {
        FlowTerm term;
        term.target = Name();
        Space();
        Expect('=');
        const std::string how = Word();
        Expect('(');
        if (SameName(how, "const")) {
          term.how = FlowTerm::How::Const;
          term.value = Unquoted(Balanced());
        } else if (SameName(how, "filter")) {
          term.how = FlowTerm::How::Filter;
          term.value = Trimmed(Balanced());
        } else if (SameName(how, "field")) {
          Space();
          if (SameName(Peek(), "upperlimit")) {
            Word();
            Expect('(');
            Space();
            if (SameName(Peek(), "filter")) {
              Word();
              Expect('(');
              term.how = FlowTerm::How::FieldUpperLimitFilter;
              term.value = Name();
              Expect(')');
            } else {
              term.how = FlowTerm::How::FieldUpperLimit;
              term.value = Name();
            }
            Expect(')');
          } else if (SameName(Peek(), "filter")) {
            Word();
            Expect('(');
            term.how = FlowTerm::How::FieldFilter;
            term.value = Name();
            Expect(')');
          } else {
            term.how = FlowTerm::How::Field;
            term.value = Name();
          }
          Expect(')');
        } else {
          Refuse("a filter of the kind " + how);
        }
        made.terms.push_back(std::move(term));
        Space();
        if (Take(',')) { continue; }
        Expect(')');
        break;
      }
    }
  }

  [[noreturn]] void Refuse(const std::string &what) const {
    throw Error("the CalcFormula of " + std::string(asked_.name) + " has " + what +
                " at position " + std::to_string(at_) + ": " + std::string(text_));
  }

  FlowFormula::Kind KindOf(std::string_view word) const {
    static constexpr std::array<std::pair<std::string_view, FlowFormula::Kind>, 7> kinds{
        {{"sum", FlowFormula::Kind::Sum},
         {"average", FlowFormula::Kind::Average},
         {"exist", FlowFormula::Kind::Exist},
         {"count", FlowFormula::Kind::Count},
         {"min", FlowFormula::Kind::Min},
         {"max", FlowFormula::Kind::Max},
         {"lookup", FlowFormula::Kind::Lookup}}};
    for (const auto &[name, kind] : kinds) {
      if (SameName(name, word)) { return kind; }
    }
    Refuse("the calculation " + std::string(word));
  }

  void Space() {
    while (at_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[at_])) != 0) {
      ++at_;
    }
  }

  bool Take(char c) {
    Space();
    if (at_ < text_.size() && text_[at_] == c) {
      ++at_;
      return true;
    }
    return false;
  }

  void Expect(char c) {
    if (!Take(c)) { Refuse(std::string("no '") + c + "'"); }
  }

  std::string_view Peek() {
    Space();
    std::size_t end = at_;
    while (end < text_.size() &&
           (std::isalnum(static_cast<unsigned char>(text_[end])) != 0 || text_[end] == '_')) {
      ++end;
    }
    return text_.substr(at_, end - at_);
  }

  std::string Word() {
    const std::string_view word = Peek();
    if (word.empty()) { Refuse("no word"); }
    at_ += word.size();
    return std::string(word);
  }

  std::string Name() {
    Space();
    if (at_ < text_.size() && text_[at_] == '"') {
      const std::size_t close = text_.find('"', at_ + 1);
      if (close == std::string_view::npos) { Refuse("an unclosed name"); }
      std::string name(text_.substr(at_ + 1, close - at_ - 1));
      at_ = close + 1;
      return name;
    }
    return Word();
  }

  std::string Balanced() {
    int depth = 1;
    const std::size_t from = at_;
    bool quoted = false;
    while (at_ < text_.size()) {
      const char c = text_[at_];
      if (c == '"') {
        quoted = !quoted;
      } else if (!quoted && c == '(') {
        ++depth;
      } else if (!quoted && c == ')') {
        if (--depth == 0) {
          std::string inner(text_.substr(from, at_ - from));
          ++at_;
          return inner;
        }
      }
      ++at_;
    }
    Refuse("an unclosed parenthesis");
  }

  static std::string Trimmed(std::string value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) { return {}; }
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
  }

  static std::string Unquoted(std::string value) {
    value = Trimmed(std::move(value));
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
      return value.substr(1, value.size() - 2);
    }
    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
      std::string inner;
      for (std::size_t i = 1; i + 1 < value.size(); ++i) {
        if (value[i] == '\'' && i + 2 < value.size() && value[i + 1] == '\'') { ++i; }
        inner += value[i];
      }
      return inner;
    }
    return value;
  }

  std::string_view text_;
  const FieldDef &asked_;
  std::size_t at_ = 0;
};

std::optional<std::string> FilterOn(const detail::RecordState *state, FieldNo no) {
  if (state == nullptr) { return std::nullopt; }
  std::string joined;
  for (const detail::FieldFilter &filter : state->filters) {
    if (filter.field != no || filter.text.empty()) { continue; }
    if (!joined.empty()) { joined += "&"; }
    joined += "(" + filter.text + ")";
  }
  if (joined.empty()) { return std::nullopt; }
  return joined;
}

std::string UpperOf(std::string_view text) {
  const std::size_t dots = text.find("..");
  return std::string(dots == std::string_view::npos ? text : text.substr(dots + 2));
}

struct Predicate {
  std::string sql;
  std::vector<std::optional<std::string>> binds;
};

void Add(Predicate &into, const detail::Clause &clause) {
  if (clause.sql.empty()) { return; }
  if (!into.sql.empty()) { into.sql += " AND "; }
  into.sql += "(" + clause.sql + ")";
  into.binds.insert(into.binds.end(), clause.binds.begin(), clause.binds.end());
}

detail::Expression Equal(std::string value) {
  detail::Atom atom;
  atom.compare = detail::Compare::Equal;
  atom.value = std::move(value);
  return detail::Expression{detail::All{atom}};
}

detail::Expression AtMost(std::string value) {
  detail::Atom atom;
  atom.compare = detail::Compare::LessOrEqual;
  atom.value = std::move(value);
  return detail::Expression{detail::All{atom}};
}

Predicate PredicateOf(const FlowFormula &formula,
                      const TableDef &target,
                      const void *record,
                      const TableDef &table,
                      const detail::RecordState *state,
                      const FieldDef &asked) {
  Predicate made;
  for (const FlowTerm &term : formula.terms) {
    const FieldDef *at = FieldNamed(target, term.target);
    if (at == nullptr) {
      throw Error("the CalcFormula of " + std::string(asked.name) + " filters " +
                  std::string(target.name) + " on " + term.target + ", which it does not declare");
    }
    const std::size_t first = made.binds.size() + 1;
    switch (term.how) {
      case FlowTerm::How::Const: Add(made, detail::Where(*at, Equal(term.value), first)); break;
      case FlowTerm::How::Filter:
        Add(made, detail::Where(*at, detail::ParseFilter(term.value), first));
        break;
      default: {
        const FieldDef *source = FieldNamed(table, term.value);
        if (source == nullptr) {
          throw Error("the CalcFormula of " + std::string(asked.name) + " reads " + term.value +
                      ", which " + std::string(table.name) + " does not declare");
        }
        const bool fromFilter = term.how == FlowTerm::How::FieldFilter ||
                                term.how == FlowTerm::How::FieldUpperLimitFilter ||
                                source->fieldClass == FieldClass::FlowFilter;
        std::optional<std::string> text = fromFilter
                                              ? FilterOn(state, source->no)
                                              : std::optional(detail::StorageText(record, *source));
        if (!text.has_value()) { break; }
        if (term.how == FlowTerm::How::FieldUpperLimit ||
            term.how == FlowTerm::How::FieldUpperLimitFilter) {
          const std::string upper = UpperOf(*text);
          if (!upper.empty()) { Add(made, detail::Where(*at, AtMost(upper), first)); }
        } else if (fromFilter) {
          Add(made, detail::Where(*at, detail::ParseFilter(*text), first));
        } else {
          Add(made, detail::Where(*at, Equal(*text), first));
        }
      }
    }
  }
  return made;
}

std::string ZeroText(const FieldDef &def) {
  switch (def.type) {
    case FieldType::Decimal:
    case FieldType::Integer:
    case FieldType::BigInteger:
    case FieldType::Duration:
    case FieldType::Option:
    case FieldType::Enum: return "0";
    case FieldType::Boolean: return "f";
    default: return {};
  }
}

std::string OrderByPrimaryKey(const TableDef &target) {
  if (target.keys.empty()) { return {}; }
  std::string out;
  for (const FieldNo no : target.keys[0].fields) {
    const FieldDef *def = Field(target, no);
    if (def == nullptr) { continue; }
    out += out.empty() ? " ORDER BY " : ", ";
    out += detail::Quoted(def->name);
  }
  return out;
}

std::string Aggregate(const FlowFormula &formula, const FieldDef &def, const std::string &column) {
  const bool whole = def.type == FieldType::Integer || def.type == FieldType::BigInteger;
  const std::string sign = formula.reverseSign ? "-" : "";
  switch (formula.kind) {
    case FlowFormula::Kind::Sum: return sign + "COALESCE(SUM(" + column + "), 0)";
    case FlowFormula::Kind::Average:
      return whole ? sign + "ROUND(COALESCE(AVG(" + column + "), 0))"
                   : sign + "COALESCE(AVG(" + column + "), 0)";
    case FlowFormula::Kind::Count: return "COUNT(*)";
    case FlowFormula::Kind::Min:
      if (def.type == FieldType::Boolean) { return "(MIN(" + column + "::int) = 1)"; }
      return sign + "MIN(" + column + ")";
    case FlowFormula::Kind::Max:
      if (def.type == FieldType::Boolean) { return "(MAX(" + column + "::int) = 1)"; }
      return sign + "MAX(" + column + ")";
    case FlowFormula::Kind::Lookup: return column;
    case FlowFormula::Kind::Exist: return "1";
  }
  return "1";
}

void Store(void *record, const FieldDef &def, const std::optional<std::string_view> &value) {
  detail::SetFieldText(record, def, value.has_value() ? std::string(*value) : ZeroText(def));
}

}

namespace detail {

void CalcField(void *record, const TableDef &table, const RecordState *state, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("CalcFields names a field the table lacks"); }
  if (def->fieldClass != FieldClass::FlowField) { return; }
  const FlowFormula formula = FormulaReader(def->calcFormula, *def).Read();
  const TableDef &target = TableNamed(formula.table, *def);
  std::string column;
  if (formula.kind != FlowFormula::Kind::Count && formula.kind != FlowFormula::Kind::Exist) {
    const FieldDef *of = FieldNamed(target, formula.field);
    if (of == nullptr) {
      throw Error("the CalcFormula of " + std::string(def->name) + " reads " +
                  std::string(target.name) + "." + formula.field + ", which it does not declare");
    }
    column = Quoted(of->name);
  }
  const Predicate predicate = PredicateOf(formula, target, record, table, state, *def);
  const std::string where = predicate.sql.empty() ? std::string{} : " WHERE " + predicate.sql;
  std::string sql;
  if (formula.kind == FlowFormula::Kind::Exist) {
    sql = "SELECT EXISTS(SELECT 1 FROM " + Name(target) + where + ")";
  } else if (formula.kind == FlowFormula::Kind::Lookup) {
    sql = "SELECT " + column + " FROM " + Name(target) + where + OrderByPrimaryKey(target) +
          " LIMIT 1";
  } else {
    sql = "SELECT " + Aggregate(formula, *def, column) + " FROM " + Name(target) + where;
  }
  const Result result = Session::Current().Database().Execute(sql, predicate.binds);
  Store(record, *def, result.Rows() == 0 ? std::nullopt : result.Value(0, 0));
}

Predicate CorrelatedPredicateOf(const FlowFormula &formula,
                                const TableDef &target,
                                const TableDef &table,
                                const detail::RecordState *state,
                                const FieldDef &asked,
                                std::size_t first) {
  Predicate made;
  for (const FlowTerm &term : formula.terms) {
    const FieldDef *at = FieldNamed(target, term.target);
    if (at == nullptr) {
      throw Error("the CalcFormula of " + std::string(asked.name) + " filters " +
                  std::string(target.name) + " on " + term.target + ", which it does not declare");
    }
    const std::size_t next = first + made.binds.size();
    switch (term.how) {
      case FlowTerm::How::Const: Add(made, detail::Where(*at, Equal(term.value), next)); break;
      case FlowTerm::How::Filter:
        Add(made, detail::Where(*at, detail::ParseFilter(term.value), next));
        break;
      default: {
        const FieldDef *source = FieldNamed(table, term.value);
        if (source == nullptr) {
          throw Error("the CalcFormula of " + std::string(asked.name) + " reads " + term.value +
                      ", which " + std::string(table.name) + " does not declare");
        }
        const bool fromFilter = term.how == FlowTerm::How::FieldFilter ||
                                term.how == FlowTerm::How::FieldUpperLimitFilter ||
                                source->fieldClass == FieldClass::FlowFilter;
        if (fromFilter) {
          const std::optional<std::string> text = FilterOn(state, source->no);
          if (!text.has_value()) { break; }
          if (term.how == FlowTerm::How::FieldUpperLimitFilter ||
              term.how == FlowTerm::How::FieldUpperLimit) {
            const std::string upper = UpperOf(*text);
            if (!upper.empty()) { Add(made, detail::Where(*at, AtMost(upper), next)); }
          } else {
            Add(made, detail::Where(*at, detail::ParseFilter(*text), next));
          }
          break;
        }
        const std::string outer = detail::Name(table) + "." + detail::Quoted(source->name);
        const bool upper = term.how == FlowTerm::How::FieldUpperLimit;
        Add(made,
            detail::Clause{.sql = detail::Quoted(at->name) + (upper ? " <= " : " = ") + outer,
                           .binds = {}});
      }
    }
  }
  return made;
}

Clause FlowFieldColumn(const TableDef &table,
                       const FieldDef &def,
                       const RecordState *state,
                       std::size_t first) {
  if (def.fieldClass != FieldClass::FlowField || def.calcFormula.empty()) { return {}; }
  const FlowFormula formula = FormulaReader(def.calcFormula, def).Read();
  const TableDef &target = TableNamed(formula.table, def);
  std::string column;
  if (formula.kind != FlowFormula::Kind::Count && formula.kind != FlowFormula::Kind::Exist) {
    const FieldDef *of = FieldNamed(target, formula.field);
    if (of == nullptr) {
      throw Error("the CalcFormula of " + std::string(def.name) + " reads " +
                  std::string(target.name) + "." + formula.field + ", which it does not declare");
    }
    column = Quoted(of->name);
  }
  const Predicate predicate = CorrelatedPredicateOf(formula, target, table, state, def, first);
  const std::string where = predicate.sql.empty() ? std::string{} : " WHERE " + predicate.sql;
  const std::string from = " FROM " + Name(target);
  Clause made;
  if (formula.kind == FlowFormula::Kind::Exist) {
    made.sql = "EXISTS(SELECT 1" + from + where + ")";
  } else if (formula.kind == FlowFormula::Kind::Lookup) {
    made.sql = "(SELECT " + column + from + where + OrderByPrimaryKey(target) + " LIMIT 1)";
  } else {
    made.sql = "(SELECT " + Aggregate(formula, def, column) + from + where + ")";
  }
  made.binds = predicate.binds;
  return made;
}

std::string FieldFormat(const void *record, const TableDef &table, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("the table lacks the field a control reads"); }
  switch (def->type) {
    case FieldType::Decimal:
      return std::string(::agiru::Format(
          *reinterpret_cast<const Decimal *>(At(const_cast<void *>(record), *def))));
    case FieldType::Integer:
      return std::string(::agiru::Format(
          *reinterpret_cast<const Integer *>(At(const_cast<void *>(record), *def))));
    case FieldType::BigInteger:
      return std::string(::agiru::Format(
          *reinterpret_cast<const BigInteger *>(At(const_cast<void *>(record), *def))));
    case FieldType::Boolean:
      return std::string(::agiru::Format(
          *reinterpret_cast<const Boolean *>(At(const_cast<void *>(record), *def))));
    case FieldType::Date:
      return std::string(
          ::agiru::Format(*reinterpret_cast<const Date *>(At(const_cast<void *>(record), *def))));
    case FieldType::Time:
      return std::string(
          ::agiru::Format(*reinterpret_cast<const Time *>(At(const_cast<void *>(record), *def))));
    case FieldType::DateTime:
      return std::string(::agiru::Format(
          *reinterpret_cast<const DateTime *>(At(const_cast<void *>(record), *def))));
    case FieldType::Option:
    case FieldType::Enum: {
      const std::int32_t ordinal =
          reinterpret_cast<const OrdinalValue *>(At(const_cast<void *>(record), *def))->AsInteger();
      const EnumValueDef *value = ValueOf(def->values, ordinal);
      if (value == nullptr) { return std::to_string(ordinal); }
      return std::string(value->caption.empty() ? value->name : value->caption);
    }
    default: return FieldText(record, *def);
  }
}

void EvaluateInto(void *record, const TableDef &table, FieldNo no, std::string_view text) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("the table lacks the field a control writes"); }
  const auto refuse = [&] {
    throw Error("The value \"" + std::string(text) + "\" can't be evaluated into type " +
                std::string(def->name) + ".");
  };
  switch (def->type) {
    case FieldType::Decimal: {
      Decimal value{};
      if (!::agiru::detail::Evaluated(value, text)) { refuse(); }
      *reinterpret_cast<Decimal *>(At(record, *def)) = value;
      return;
    }
    case FieldType::Integer: {
      Integer value{};
      if (!::agiru::detail::Evaluated(value, text)) { refuse(); }
      *reinterpret_cast<Integer *>(At(record, *def)) = value;
      return;
    }
    case FieldType::BigInteger: {
      BigInteger value{};
      if (!::agiru::detail::Evaluated(value, text)) { refuse(); }
      *reinterpret_cast<BigInteger *>(At(record, *def)) = value;
      return;
    }
    case FieldType::Boolean: {
      Boolean value{};
      if (!::agiru::detail::Evaluated(value, text)) { refuse(); }
      *reinterpret_cast<Boolean *>(At(record, *def)) = value;
      return;
    }
    case FieldType::Date: {
      Date value{};
      if (!::agiru::detail::Evaluated(value, text)) { refuse(); }
      *reinterpret_cast<Date *>(At(record, *def)) = value;
      return;
    }
    case FieldType::Option:
    case FieldType::Enum: {
      const std::string ordinal = MemberOrdinal(*def, text);
      if (ordinal.find_first_not_of("-0123456789") != std::string::npos) { refuse(); }
      SetFieldText(record, *def, ordinal);
      return;
    }
    default: SetFieldText(record, *def, text); return;
  }
}

void CalcSum(void *record, const TableDef &table, const RecordState *state, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("CalcSums names a field the table lacks"); }
  if (def->fieldClass != FieldClass::Normal) {
    throw Error("CalcSums over " + std::string(def->name) + " needs a stored field; " +
                "a FlowField is calculated with CalcFields");
  }
  if (def->type != FieldType::Decimal && def->type != FieldType::Integer &&
      def->type != FieldType::BigInteger && def->type != FieldType::Duration) {
    throw Error("CalcSums over " + std::string(def->name) + " needs a numeric field");
  }
  const Selection selection = Select(state, table);
  const std::string sql = "SELECT COALESCE(SUM(" + Quoted(def->name) + "), 0) FROM " + Name(table) +
                          (selection.where.empty() ? std::string{} : " WHERE " + selection.where);
  const Result result = Session::Current().Database().Execute(sql, selection.binds);
  Store(record, *def, result.Rows() == 0 ? std::nullopt : result.Value(0, 0));
}

}

}
