#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/Decimal.h"
#include "type/Enum.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

/// \file
/// \brief What a generated table declares, and what is derived rather than repeated.

namespace agiru {

/// \brief What a field's C++ type says about the field.
///
/// \tparam T The member's type.
///
/// AL's `field(2; "Code"; Code[20])` states a type, and everything else about the storage follows
/// from it: the type tag, the declared length, and an enumeration's values. Those are derived here
/// rather than repeated in the declaration, so that a table says each thing once.
template <typename T> struct FieldTypeOf;

/// \brief `Code[N]` -- a code field of declared length N.
template <std::size_t N> struct FieldTypeOf<Code<N>> {
  static constexpr FieldType kType = FieldType::Code;                     ///< The AL type tag.
  static constexpr std::uint16_t kLength = static_cast<std::uint16_t>(N); ///< The declared length.
  static constexpr std::span<const EnumValueDef> kValues{};               ///< Not an enumeration.
};

/// \brief `Text[N]` -- a text field of declared length N.
template <std::size_t N> struct FieldTypeOf<Text<N>> {
  static constexpr FieldType kType = FieldType::Text;                     ///< The AL type tag.
  static constexpr std::uint16_t kLength = static_cast<std::uint16_t>(N); ///< The declared length.
  static constexpr std::span<const EnumValueDef> kValues{};               ///< Not an enumeration.
};

/// \brief `Decimal` -- a decimal field.
template <> struct FieldTypeOf<Decimal> {
  static constexpr FieldType kType = FieldType::Decimal;    ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A decimal declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief A whole number, a truth value or a duration -- a field with no length and no members.
template <typename T>
  requires std::is_arithmetic_v<T>
struct FieldTypeOf<T> {
  /// \brief The AL type tag, chosen by what the C++ type is.
  static constexpr FieldType kType = std::is_same_v<T, Boolean>      ? FieldType::Boolean
                                     : std::is_same_v<T, Integer>    ? FieldType::Integer
                                     : std::is_same_v<T, BigInteger> ? FieldType::BigInteger
                                                                     : FieldType::Integer;
  static constexpr std::uint16_t kLength = 0;               ///< A number declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `Date` -- a calendar day, its undefined value and its closing twin in four bytes.
template <> struct FieldTypeOf<Date> {
  static constexpr FieldType kType = FieldType::Date;       ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A date declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `Option` -- an option field, whose members come from its enumeration.
template <typename E> struct FieldTypeOf<Option<E>> {
  static constexpr FieldType kType = FieldType::Option; ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;           ///< An option declares no length.

  /// The declared members, taken from the option's own declaration rather than repeated here.
  static constexpr std::span<const EnumValueDef> kValues{OptionTraits<E>::kValues};
};

/// \brief `Enum` -- an enum field, whose values come from the enum object it names.
template <typename E> struct FieldTypeOf<Enum<E>> {
  static constexpr FieldType kType = FieldType::Enum; ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;         ///< An enum declares no length.

  /// The declared values, taken from the enum object rather than repeated per field.
  static constexpr std::span<const EnumValueDef> kValues{EnumTraits<E>::kValues};
};

/// \brief The class a member pointer points into.
template <typename T> struct MemberOwnerOf;

/// \brief The class a member pointer points into.
template <typename Class, typename Value> struct MemberOwnerOf<Value Class::*> {
  using Type = Class;  ///< The class.
  using Field = Value; ///< The member's type.
};

/// \brief Builds one field's runtime declaration from its member.
///
/// \tparam Member A pointer to the field's member, which is where its TYPE comes from.
/// \param no      The AL field number.
/// \param name    The AL name, spaces and all.
/// \param caption The `Caption` property.
/// \param offset  `offsetof` the member within the record.
/// \return The field's declaration.
///
/// The type tag, the declared length and an enumeration's values are DERIVED from the member's
/// type rather than repeated, so a table states each of them once. What is left is the four things
/// AL states that no C++ type carries -- the number, the name, the caption -- plus the offset.
///
/// \note The identifier appears twice, once as the member and once inside `offsetof`, and no
///       standard C++ removes that: no member pointer yields a `constexpr` offset. It is not a
///       defect here, because this file is written by a generator from one AST node and the
///       compiler checks the pair -- a repetition a machine emits and a compiler verifies is a
///       checksum rather than a duplication. With C++26 reflection (P2996) it would go; measured
///       2026-09-01, neither clang-19 nor gcc-14 has it (board:0015).
template <auto Member>
constexpr FieldDef
Declare(FieldNo no, std::string_view name, std::string_view caption, std::size_t offset) {
  using Value = typename MemberOwnerOf<decltype(Member)>::Field;
  return FieldDef{
      .no = no,
      .name = name,
      .caption = caption,
      .type = FieldTypeOf<Value>::kType,
      .length = FieldTypeOf<Value>::kLength,
      .offset = offset,
      .values = FieldTypeOf<Value>::kValues,
  };
}

} // namespace agiru
