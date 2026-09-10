#include "runtime/Record.h"

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Media.h"
#include "type/MediaSet.h"
#include "type/RecordId.h"
#include "type/TableFilter.h"
#include "type/Time.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"

#include <algorithm>
#include <cctype>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {
namespace {
const std::byte *At(const void *record, const FieldDef &def) {
  return static_cast<const std::byte *>(record) + def.offset;
}

std::string PrimaryKeyText(const void *record, const TableDef &table, std::string_view separator) {
  if (table.keys.empty()) { return {}; }
  std::string out;
  for (const FieldNo no : table.keys[0].fields) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { continue; }
    if (!out.empty()) { out += separator; }
    out += std::string(def->caption) + "='" + FieldText(record, *def) + "'";
  }
  return out;
}

}

namespace detail {

std::string VariantText(const Variant &value) {
  return std::string(std::string_view(::agiru::Format(value)));
}

}

std::string FieldText(const void *record, const FieldDef &def) {
  switch (def.type) {
    case FieldType::Code:
    case FieldType::Text:
      return std::string(reinterpret_cast<const StringValue *>(At(record, def))->Value());
    case FieldType::Decimal:
      return reinterpret_cast<const Decimal *>(At(record, def))->ToInvariantString();
    case FieldType::Date:
      return reinterpret_cast<const Date *>(At(record, def))->ToInvariantString();
    case FieldType::Time:
      return reinterpret_cast<const Time *>(At(record, def))->ToInvariantString();
    case FieldType::DateTime:
      return reinterpret_cast<const DateTime *>(At(record, def))->ToInvariantString();
    case FieldType::Guid: return reinterpret_cast<const Guid *>(At(record, def))->ToText();
    case FieldType::RecordId: return reinterpret_cast<const RecordId *>(At(record, def))->ToText();
    case FieldType::DateFormula:
      return reinterpret_cast<const DateFormula *>(At(record, def))->ToText();
    case FieldType::Integer: return ToText(*reinterpret_cast<const Integer *>(At(record, def)));
    case FieldType::BigInteger:
      return ToText(*reinterpret_cast<const BigInteger *>(At(record, def)));
    case FieldType::Boolean: return ToText(*reinterpret_cast<const Boolean *>(At(record, def)));
    case FieldType::Option:
    case FieldType::Enum:
      return detail::MemberText(
          def, reinterpret_cast<const OrdinalValue *>(At(record, def))->AsInteger());
    case FieldType::Blob: {
      const std::vector<std::uint8_t> &bytes =
          reinterpret_cast<const Blob *>(At(record, def))->Bytes();
      if (bytes.empty()) { return "\\x"; }
      std::string out = "\\x";
      for (const std::uint8_t byte : bytes) { out += std::format("{:02x}", byte); }
      return out;
    }
    case FieldType::Duration:
      return reinterpret_cast<const Duration *>(At(record, def))->ToInvariantString();
    case FieldType::TableFilter:
      return std::string(reinterpret_cast<const TableFilter *>(At(record, def))->Value());
    case FieldType::Media:
      return reinterpret_cast<const Media *>(At(record, def))->MediaId().ToText();
    case FieldType::MediaSet:
      return reinterpret_cast<const MediaSet *>(At(record, def))->MediaId().ToText();
    default: throw Error("FieldText: no rendering for this field type yet");
  }
}

bool IsBlank(const void *record, const FieldDef &def) {
  switch (def.type) {
    case FieldType::Code:
    case FieldType::Text: return reinterpret_cast<const StringValue *>(At(record, def))->IsEmpty();
    case FieldType::Decimal: return reinterpret_cast<const Decimal *>(At(record, def))->IsZero();
    case FieldType::Date: return reinterpret_cast<const Date *>(At(record, def))->IsUndefined();
    case FieldType::Time: return reinterpret_cast<const Time *>(At(record, def))->IsUndefined();
    case FieldType::DateTime:
      return reinterpret_cast<const DateTime *>(At(record, def))->IsUndefined();
    case FieldType::Guid: return reinterpret_cast<const Guid *>(At(record, def))->IsNull();
    case FieldType::RecordId: return reinterpret_cast<const RecordId *>(At(record, def))->IsEmpty();
    case FieldType::DateFormula:
      return reinterpret_cast<const DateFormula *>(At(record, def))->IsEmpty();
    case FieldType::Integer: return *reinterpret_cast<const Integer *>(At(record, def)) == 0;
    case FieldType::BigInteger: return *reinterpret_cast<const BigInteger *>(At(record, def)) == 0;
    case FieldType::Boolean: return !*reinterpret_cast<const Boolean *>(At(record, def));
    case FieldType::Blob: return !reinterpret_cast<const Blob *>(At(record, def))->HasValue();
    case FieldType::Media: return !reinterpret_cast<const Media *>(At(record, def))->HasValue();
    case FieldType::MediaSet:
      return reinterpret_cast<const MediaSet *>(At(record, def))->MediaId().IsNull();
    case FieldType::Option:
    case FieldType::Enum:
      return reinterpret_cast<const OrdinalValue *>(At(record, def))->AsInteger() == 0;
    default: throw Error("IsBlank: no blank test for this field type yet");
  }
}

std::string_view FieldCaption(const TableDef &table, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("FieldCaption: the table declares no such field"); }
  return def->caption;
}

void FieldError(const void *record, const TableDef &table, FieldNo no, std::string_view text) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("FieldError: the table declares no such field"); }

  const std::string key = PrimaryKeyText(record, table, ",");
  const std::string where =
      " in " + std::string(table.caption) + (key.empty() ? std::string{} : " " + key);

  if (!text.empty()) {
    throw Error(std::string(def->caption) + " " + std::string(text) + where + ".");
  }
  if (IsBlank(record, *def)) {
    throw Error("You must specify " + std::string(def->caption) + where + ".");
  }
  throw Error(std::string(def->caption) + " must not be " + FieldText(record, *def) + where + ".",
              "TableErrorStr");
}

namespace detail {

void TestField(const void *record, const TableDef &table, FieldNo no) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("TestField: the table declares no such field"); }
  if (!IsBlank(record, *def)) { return; }
  const std::string key = PrimaryKeyText(record, table, ", ");
  const std::string where =
      " in " + std::string(table.caption) + (key.empty() ? std::string{} : ": " + key);
  if (def->type == FieldType::Option || def->type == FieldType::Enum) {
    const std::string member = FieldText(record, *def);
    if (!member.empty()) {
      throw Error(std::string(def->caption) + " must not be " + member + where + ".", "TestField");
    }
  }
  throw Error(std::string(def->caption) + " must have a value" + where +
                  ". It cannot be zero or empty.",
              "TestField");
}

}

namespace detail {
std::string MemberText(const FieldDef &def, std::int32_t ordinal) {
  const EnumValueDef *value = ValueOf(def.values, ordinal);
  return value != nullptr ? std::string(value->name) : std::to_string(ordinal);
}

std::string MemberOrdinal(const FieldDef &def, std::string_view text) {
  if (text.find_first_not_of(' ') == std::string_view::npos) {
    for (const EnumValueDef &value : def.values) {
      if (value.name.find_first_not_of(' ') == std::string_view::npos) {
        return std::to_string(value.ordinal);
      }
    }
    return "0";
  }
  if (text.find_first_not_of("-0123456789") == std::string_view::npos) { return std::string(text); }
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    text = text.substr(1, text.size() - 2);
  }
  const auto same = [](std::string_view a, std::string_view b) {
    return std::ranges::equal(
        a, b, [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
  };
  for (const EnumValueDef &value : def.values) {
    if (same(value.name, text) || same(value.caption, text)) {
      return std::to_string(value.ordinal);
    }
  }
  return std::string(text);
}

void RaiseTestFieldMismatch(const void *record,
                            const TableDef &table,
                            const FieldDef &def,
                            std::string_view expected,
                            std::string_view actual) {
  const std::string key = PrimaryKeyText(record, table, ", ");
  throw Error(std::string(def.caption) + " must be equal to '" + std::string(expected) + "'  in " +
                  std::string(table.caption) + (key.empty() ? std::string{} : ": " + key) +
                  ". Current value is '" + std::string(actual) + "'.",
              "TestField");
}

std::string SubstituteInto(std::string_view pattern, std::span<const std::string_view> values) {
  std::string out;
  out.reserve(pattern.size());
  for (std::size_t i = 0; i < pattern.size(); ++i) {
    const char c = pattern[i];
    const bool marker = (c == '%' || c == '#') && i + 1 < pattern.size();
    if (!marker) {
      out += c;
      continue;
    }
    const char digit = pattern[i + 1];
    if (digit < '1' || digit > '9') {
      out += c;
      continue;
    }
    const auto index = static_cast<std::size_t>(digit - '1');
    if (index >= values.size()) {
      out += c;
      continue;
    }
    out += values[index];
    ++i;
  }
  return out;
}

}

std::strong_ordering CompareField(const void *a, const void *b, const FieldDef &def) {
  const auto compare = [](const auto &left, const auto &right) {
    return left < right   ? std::strong_ordering::less
           : right < left ? std::strong_ordering::greater
                          : std::strong_ordering::equal;
  };
  switch (def.type) {
    case FieldType::Integer:
      return compare(*reinterpret_cast<const Integer *>(At(a, def)),
                     *reinterpret_cast<const Integer *>(At(b, def)));
    case FieldType::BigInteger:
      return compare(*reinterpret_cast<const BigInteger *>(At(a, def)),
                     *reinterpret_cast<const BigInteger *>(At(b, def)));
    case FieldType::Boolean:
      return compare(*reinterpret_cast<const Boolean *>(At(a, def)),
                     *reinterpret_cast<const Boolean *>(At(b, def)));
    case FieldType::Option:
    case FieldType::Enum:
      return compare(reinterpret_cast<const OrdinalValue *>(At(a, def))->AsInteger(),
                     reinterpret_cast<const OrdinalValue *>(At(b, def))->AsInteger());
    case FieldType::Date:
      return *reinterpret_cast<const Date *>(At(a, def)) <=>
             *reinterpret_cast<const Date *>(At(b, def));
    case FieldType::Time:
      return *reinterpret_cast<const Time *>(At(a, def)) <=>
             *reinterpret_cast<const Time *>(At(b, def));
    case FieldType::DateTime:
      return *reinterpret_cast<const DateTime *>(At(a, def)) <=>
             *reinterpret_cast<const DateTime *>(At(b, def));
    case FieldType::Decimal:
      return compare(*reinterpret_cast<const Decimal *>(At(a, def)),
                     *reinterpret_cast<const Decimal *>(At(b, def)));
    default: return compare(FieldText(a, def), FieldText(b, def));
  }
}

}
