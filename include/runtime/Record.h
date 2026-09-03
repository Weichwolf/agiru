#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Decimal.h"
#include "type/Option.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

/// \file
/// \brief The AL record operations that raise the platform's own error messages.

namespace agiru {

/// \brief Reads one field of a record as the text AL would put in a message.
///
/// \param record The record, addressed as raw storage.
/// \param def    The field to read.
/// \return The value as AL renders it: a string as itself, a decimal in invariant notation, an
///         option by its member name.
/// \throws Error when the field's type has no rendering yet. That is loud on purpose -- a silent
///         empty string here becomes an error message missing its value, which reads as a defect
///         somewhere else entirely.
///
/// Addressing the record as bytes plus an offset is what makes this work for every table without a
/// virtual call and without the runtime knowing a single AL object.
[[nodiscard]] std::string FieldText(const void *record, const FieldDef &def);

/// \brief Orders one field of two records of the same table.
///
/// \param a   One record.
/// \param b   The other.
/// \param def The field, which decides how the two values are compared.
/// \return The ordering.
///
/// \warning BY TYPE AND NEVER BY TEXT. Rendering both sides and comparing the strings orders "10"
///          before "9", which is the wrong walk order for every table keyed on an entry number --
///          and wrong silently, because both orders look plausible until the numbers reach ten.
[[nodiscard]] std::strong_ordering CompareField(const void *a, const void *b, const FieldDef &def);

/// \brief Tests whether a field holds the blank of its type.
///
/// \param record The record, addressed as raw storage.
/// \param def    The field to test.
/// \return True for an empty string, a zero number, or ordinal zero.
///
/// From `record-testfield-joker-method.md`: "If you omit this parameter and the contents of Field
/// is zero or blank (empty string), then an error message is displayed."
[[nodiscard]] bool IsBlank(const void *record, const FieldDef &def);

/// \brief AL `Record.FieldCaption(Field)`.
///
/// \param table The record's table.
/// \param no    The field number.
/// \return The field's `Caption` property.
/// \throws Error when the table declares no such field.
/// \see `record-fieldcaption-method.md`
[[nodiscard]] std::string_view FieldCaption(const TableDef &table, FieldNo no);

/// \brief AL `Record.FieldError(Field [, Text])` -- raises an error naming a field.
///
/// \param record The record whose primary key the message quotes.
/// \param table  The record's table.
/// \param no     The field the message is about.
/// \param text   Optional replacement for the default wording.
/// \throws Error always.
///
/// The three forms are the documentation's own worked examples, and the trailing full stop is the
/// platform's ("Note that a period is automatically inserted at the end of a FieldError"):
///
/// \verbatim
/// FieldError("No.")                  blank  -> You must specify No. in Customer No.=''.
/// FieldError("No.")                  valued -> No. must not be NEW 3500 in Customer No.='NEW
/// 3500'. FieldError("No.", 'is not valid')         -> No. is not valid in Customer No.='NEW 3500'.
/// \endverbatim
///
/// \warning The primary key is rendered with one leading space and commas with NO space after
///          them. TestField() renders it differently, and that difference is load-bearing rather
///          than a slip.
/// \see `record-fielderror-joker-string-method.md`
[[noreturn]] void
FieldError(const void *record, const TableDef &table, FieldNo no, std::string_view text = {});

/// \brief AL `Record.TestField(Field)` -- raises when the field holds its type's blank.
///
/// \param record The record whose primary key the message quotes.
/// \param table  The record's table.
/// \param no     The field to test.
/// \throws Error when the field is blank, with the message
///
/// \verbatim
/// Code must have a value in Resource Cost: Type='Resource', Code=''. It cannot be zero or empty.
/// \endverbatim
///
/// \warning The primary key is rendered after a COLON, with commas that DO carry a space. That is
///          not what FieldError() does. The form is in no document; it comes from the predecessor,
///          where it was verified against the official BC test suite, and it matters because BC
///          test code matches the message text.
/// \see `record-testfield-joker-method.md`
/// \note IT LIVES IN `detail` BECAUSE `TestField` IS ALSO AN AL TYPE. The test framework's
///       `TestField` is one control of a page, and a class and a function of the same name in one
///       namespace make every use of the class ambiguous. The door keeps the TYPE, because that is
///       what AL code writes; this is the platform half behind `Record::TestField`, which no
///       generated file ever names.
namespace detail {

/// \brief The platform half of `Record.TestField(Field)` -- raises when the field holds its blank.
/// \param record The record whose primary key the message quotes.
/// \param table  The record's table.
/// \param no     The field to test.
/// \throws Error with BC's own wording when the field holds its type's zero.
void TestField(const void *record, const TableDef &table, FieldNo no);

} // namespace detail

/// \brief Internals of the record operations. Not part of the door.
namespace detail {

/// \brief Raises TestField's mismatch message.
///
/// \param record   The record whose primary key the message quotes.
/// \param table    The record's table.
/// \param def      The field that did not match.
/// \param expected The expected value, already rendered.
/// \param actual   The stored value, already rendered.
/// \throws Error always.
///
/// \warning The DOUBLE SPACE before the word "in" is BC-faithful and not a typo. It comes from the
///          predecessor, verified there against the official test suite; removing it stops an
///          `Assert.ExpectedError` substring from matching.
[[noreturn]] void RaiseTestFieldMismatch(const void *record,
                                         const TableDef &table,
                                         const FieldDef &def,
                                         std::string_view expected,
                                         std::string_view actual);

/// \brief Renders an option ordinal through its field's member table.
///
/// \param def     The field, which carries the member names.
/// \param ordinal The zero-based member number.
/// \return The member name, or the ordinal as digits when it is undeclared.
[[nodiscard]] std::string MemberText(const FieldDef &def, std::int32_t ordinal);

/// \brief Replaces the numbered placeholders in a pattern.
///
/// \param pattern The text carrying the placeholders.
/// \param values  The replacements, in order.
/// \return The substituted text.
[[nodiscard]] std::string SubstituteInto(std::string_view pattern,
                                         std::span<const std::string_view> values);

/// \brief Renders a value about to appear in a message, choosing by what the value is.
///
/// \tparam T    The value's type.
/// \param value The value.
/// \param def   The field it is compared against, which carries an enumeration's value names.
/// \return The rendered text.
template <typename T> [[nodiscard]] std::string TextOf(const T &value, const FieldDef &def) {
  if constexpr (std::is_base_of_v<OrdinalValue, T>) {
    return MemberText(def, value.AsInteger());
  } else if constexpr (std::is_base_of_v<StringValue, T>) {
    return std::string(value.Value());
  } else if constexpr (std::is_same_v<T, Decimal>) {
    return value.ToInvariantString();
  } else if constexpr (std::is_same_v<T, Boolean>) {
    return ToText(value);
  } else {
    return std::string(value);
  }
}

} // namespace detail

/// \brief AL `Record.TestField(Field, Value)` -- raises when the field does not hold that value.
///
/// \tparam T       The field's own type, which is what makes the comparison the type's own: a Code
///                 literal is Code-normalised before it is compared, because handing it to a
///                 `Code<N>` parameter is what AL does with the argument.
/// \param record   The record whose primary key the message quotes.
/// \param table    The record's table.
/// \param no       The field to test.
/// \param expected The value it must hold.
/// \throws Error when the values differ, or when the table declares no such field.
/// \see `record-testfield-joker-joker-method.md`, detail::RaiseTestFieldMismatch
/// \note Named apart from the AL type `TestField`, for the reason the blank form gives above.
template <typename T>
void TestFieldValue(const void *record, const TableDef &table, FieldNo no, const T &expected) {
  const FieldDef *def = Field(table, no);
  if (def == nullptr) { throw Error("TestField: the table declares no such field"); }
  const auto *actual =
      reinterpret_cast<const T *>(static_cast<const std::byte *>(record) + def->offset);
  if (*actual == expected) { return; }
  detail::RaiseTestFieldMismatch(
      record, table, *def, detail::TextOf(expected, *def), FieldText(record, *def));
}

/// \brief AL `Format(Value)` for an option -- its caption.
///
/// \tparam E The option's enumeration.
/// \param value The option.
/// \return The caption of the member it holds.
///
/// \warning A first step only. AL's `Format` takes a length and a format number and is locale
///          dependent; the full function is board:0007. What is here covers `Format(<option>)`,
///          which is what an error message uses.
template <typename E> [[nodiscard]] std::string Format(const Option<E> &value) {
  return std::string(value.Caption());
}

/// \brief One argument of `StrSubstNo`, rendered the way AL renders it.
///
/// \tparam T The argument's type.
/// \param value The argument.
/// \return Its text.
///
/// \note AL's `StrSubstNo` TAKES `Any`, AND EVERY TYPE HAS A RENDERING. Requiring a string view
///       made `StrSubstNo(Msg, Amount)` -- a Decimal in a message, which the BaseApp writes
///       constantly -- fail to compile. What reads as text is used as it is; everything else goes
///       through `Format`, which is what AL does with it.
template <typename T> [[nodiscard]] std::string AsText(const T &value) {
  if constexpr (std::convertible_to<const T &, std::string_view>) {
    return std::string(std::string_view(value));
  } else if constexpr (requires { value.ToText(); }) {
    return value.ToText();
  } else if constexpr (std::is_arithmetic_v<T>) {
    return std::to_string(value);
  } else {
    return Format(value);
  }
}

/// \brief AL `Format(Any)` for a value that is not a Variant.
///
/// \tparam T The value's type.
/// \param value The value.
/// \return Its text.
///
/// \note THE DOCUMENTED SIGNATURE TAKES AN `Any` AND A VARIANT HOLDS ONLY SOME TYPES. A `Char`, an
///       array element, a generated option -- AL formats all of them and none of them is a Variant
///       alternative. This is the same rendering `StrSubstNo` uses on its arguments, under the name
///       AL calls it by.
template <typename T>
  requires(!std::convertible_to<const T &, const Variant &>)
[[nodiscard]] std::string Format(const T &value) {
  return AsText(value);
}

/// \brief AL `StrSubstNo(Text [, Any,...])`.
///
/// \tparam Args    The argument types, each convertible to a string view.
/// \param pattern  The text carrying `%1` to `%9` or `#1` to `#9`.
/// \param args     The replacements, in order.
/// \return The substituted text.
///
/// \note A placeholder with no argument is LEFT STANDING, so a message missing a parameter is
///       visible rather than merely wrong.
/// \see `text-strsubstno-method.md`
template <typename... Args>
[[nodiscard]] std::string StrSubstNo(std::string_view pattern, const Args &...args) {
  const std::initializer_list<std::string> rendered{AsText(args)...};
  std::vector<std::string_view> values;
  values.reserve(rendered.size());
  for (const std::string &one : rendered) { values.emplace_back(one); }
  return detail::SubstituteInto(pattern, std::span<const std::string_view>(values));
}

} // namespace agiru
