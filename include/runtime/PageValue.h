#pragma once

#include <string>
#include <string_view>

/// \file
/// \brief Lossless scalar page values, separate from localized display text.

namespace agiru {

struct FieldDef;
struct TableDef;

/// \brief A version-one semantic-HTML scalar. Unknown values always refuse.
/// All numbers travel as strings. Date/Time/DateTime use invariant text with an
/// explicit undefined flag; a Date also carries its closing flag. Duration is
/// signed milliseconds. Option/Enum carry an ordinal and a declared storage domain,
/// not a caption-derived value or a claim to a globally resolved Enum object ID.
/// RecordId is "base64:" plus RFC 4648 encoding of its round-tripping storage bytes.
struct PageValue {
  std::string type{};  ///< AL scalar tag: Decimal, BigInteger, Boolean, etc.
  std::string value{}; ///< Exact invariant value, never a localized number.
  /// \brief Option/Enum domain: table/<id>/field/<number> or <object-kind>/<id>/control/<name>.
  std::string domain{};
  std::string member{};   ///< Declared Option/Enum member name; empty if undeclared.
  bool undefined = false; ///< Undefined Date/Time/DateTime, distinct from a blank display.
  bool closing = false;   ///< Closing Date, distinct from its normal twin.
};

/// \brief Reads a bound scalar from typed record storage, without parsing its display.
/// \param record Non-null storage matching the supplied immutable table declaration.
/// \param table The owning declaration and stable Option/Enum field domain.
/// \param field The field belonging to that table, including its storage offset.
/// \return An owned lossless value; Decimal scale and Int64 digits remain unchanged.
/// \throws Error for null/mismatched declarations or unsupported Blob/Media/filter values.
[[nodiscard]] PageValue
ReadPageValue(const void *record, const TableDef &table, const FieldDef &field);

/// \brief Reads standalone typed storage using the same lossless scalar transport as fields.
/// \param value Non-null storage matching the declaration's type, length and members.
/// \param declaration Scalar declaration with offset zero and no FlowFilter class.
/// \param domain Nonempty stable declaration identity, not a localized caption.
/// \return An owned exact value; no display-text parsing or numeric narrowing occurs.
/// \throws Error for null storage, invalid declarations or unsupported scalar types.
[[nodiscard]] PageValue
ReadPageScalar(const void *value, const FieldDef &declaration, std::string_view domain);

}
