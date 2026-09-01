#pragma once

#include "agiru/Decimal.h"
#include "agiru/Error.h"
#include "agiru/Ids.h"
#include "agiru/Option.h"
#include "agiru/TableDef.h"
#include "agiru/Text.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

namespace agiru {

/// Reads one field of a record as the text AL would put in a message.
///
/// The record is addressed as bytes plus the field's offset, which is what makes this work for
/// every table without a virtual call and without the runtime knowing a single AL object -- the
/// invariant in CLAUDE.md that the runtime names no AL object.
[[nodiscard]] std::string FieldText(const void *record, const FieldDef &def);

/// True when the field holds the blank of its type: an empty string, a zero number, ordinal zero.
/// `record-testfield-joker-method.md`: "If you omit this parameter and the contents of Field is
/// zero or blank (empty string), then an error message is displayed."
[[nodiscard]] bool IsBlank(const void *record, const FieldDef &def);

/// AL `Record.FieldCaption(Field)` -- `record-fieldcaption-method.md`.
[[nodiscard]] std::string_view FieldCaption(const TableDef &table, FieldNo no);

/// AL `Record.FieldError(Field [, Text])` -- `record-fielderror-joker-string-method.md`.
///
/// The three forms are the documentation's own three examples, and the trailing period is added by
/// the platform ("Note that a period is automatically inserted at the end of a FieldError"):
///
///     FieldError("No.")            empty  -> You must specify No. in Customer No.=''.
///     FieldError("No.")            valued -> No. must not be NEW 3500 in Customer No.='NEW 3500'.
///     FieldError("No.", 'is not valid')   -> No. is not valid in Customer No.='NEW 3500'.
[[noreturn]] void
FieldError(const void *record, const TableDef &table, FieldNo no, std::string_view text = {});

/// AL `Record.TestField(Field)` -- raises when the field holds its type's blank.
void TestField(const void *record, const TableDef &table, FieldNo no);

namespace detail {

[[noreturn]] void RaiseTestFieldMismatch(const void *record,
                                         const TableDef &table,
                                         const FieldDef &def,
                                         std::string_view expected,
                                         std::string_view actual);

/// An option is rendered by its member NAME, and the names live in the field's metadata rather than
/// in the value -- so rendering an expected option needs the field it is compared against.
[[nodiscard]] std::string MemberText(const FieldDef &def, std::int32_t ordinal);

[[nodiscard]] std::string SubstituteInto(std::string_view pattern,
                                         std::span<const std::string_view> values);

/// The text of a value that is about to appear in a message, chosen by what the value IS.
template <typename T> [[nodiscard]] std::string TextOf(const T &value, const FieldDef &def) {
  if constexpr (std::is_base_of_v<OptionValue, T>) {
    return MemberText(def, value.AsInteger());
  } else if constexpr (std::is_base_of_v<StringValue, T>) {
    return std::string(value.Value());
  } else if constexpr (std::is_same_v<T, Decimal>) {
    return value.ToInvariantString();
  } else {
    return std::string(value);
  }
}

} // namespace detail

/// AL `Record.TestField(Field, Value)` -- `record-testfield-joker-joker-method.md`.
///
/// The value is typed to the FIELD's type at the call site, which is what lets the comparison be
/// the type's own: a Code literal is Code-normalised before it is compared, because handing it to a
/// `Code<N>` parameter is what AL does.
template <typename T>
void TestField(const void *record, const TableDef &table, FieldNo no, const T &expected) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("TestField: the table declares no such field"); }
  const auto *actual =
      reinterpret_cast<const T *>(static_cast<const std::byte *>(record) + def->offset);
  if (*actual == expected) { return; }
  detail::RaiseTestFieldMismatch(
      record, table, *def, detail::TextOf(expected, *def), FieldText(record, *def));
}

/// AL `StrSubstNo(Text [, Any,...])` -- `text-strsubstno-method.md`: "Replaces %1, %2, %3... and
/// #1, #2, #3... fields in a string with the values you provide as optional parameters."
template <typename... Args>
[[nodiscard]] std::string StrSubstNo(std::string_view pattern, const Args &...args) {
  const std::initializer_list<std::string_view> values{std::string_view(args)...};
  return detail::SubstituteInto(pattern, std::span<const std::string_view>(values));
}

} // namespace agiru
