#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Enum.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/RecordId.h"
#include "type/Text.h"
#include "type/Time.h"

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
/// \brief One system field, as `devenv-table-system-fields.md` tabulates it.
struct SystemFieldDecl {
  FieldNo no;              ///< The reserved field number.
  std::string_view name;   ///< The AL name, which is also the caption.
  std::string_view alType; ///< The AL data type, which the generated member is declared with.
};

/// \brief The system fields, in field-number order.
///
/// \note THIS IS THE ONLY PLACE IN THE TREE THAT SPELLS THEM. The generator writes the members from
///       it, `SystemFieldNumbers` takes its constants from it, and `WithSystemFields` builds their
///       declarations from it -- so a name, a number and a type are each said once.
inline constexpr std::array<SystemFieldDecl, kSystemFieldCount> kSystemFields{{
    {.no = FieldNo{2000000000}, .name = "SystemId", .alType = "Guid"},
    {.no = FieldNo{2000000001}, .name = "SystemCreatedAt", .alType = "DateTime"},
    {.no = FieldNo{2000000002}, .name = "SystemCreatedBy", .alType = "Guid"},
    {.no = FieldNo{2000000003}, .name = "SystemModifiedAt", .alType = "DateTime"},
    {.no = FieldNo{2000000004}, .name = "SystemModifiedBy", .alType = "Guid"},
}};

/// \brief The field numbers the platform gives every table.
///
/// A generated table's `FieldNumber` struct DERIVES from this, so the five numbers are said once in
/// the door instead of once per table. They are static constants, so inheriting them costs no
/// layout -- which the storage they belong to cannot say for itself (see `WithSystemFields`).
struct SystemFieldNumbers {
  static constexpr FieldNo SystemId = kSystemFields[0].no;        ///< The row's immutable identity.
  static constexpr FieldNo SystemCreatedAt = kSystemFields[1].no; ///< The instant it was written.
  static constexpr FieldNo SystemCreatedBy = kSystemFields[2].no; ///< The SID that wrote it.
  static constexpr FieldNo SystemModifiedAt = kSystemFields[3].no; ///< The instant it last changed.
  static constexpr FieldNo SystemModifiedBy = kSystemFields[4].no; ///< The SID that changed it.
};

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

/// \brief `DateFormula` -- a date calculation written down.
template <> struct FieldTypeOf<DateFormula> {
  static constexpr FieldType kType = FieldType::DateFormula; ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;                ///< A formula declares no length.
  static constexpr std::span<const EnumValueDef> kValues{};  ///< Not an enumeration.
};

/// \brief `RecordId` -- which table, and which row of it.
template <> struct FieldTypeOf<RecordId> {
  static constexpr FieldType kType = FieldType::RecordId;   ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A RecordId declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `Duration` -- how long, in milliseconds.
template <> struct FieldTypeOf<Duration> {
  static constexpr FieldType kType = FieldType::Duration;   ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A duration declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `Time` -- a time of day, in milliseconds since midnight.
template <> struct FieldTypeOf<Time> {
  static constexpr FieldType kType = FieldType::Time;       ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A time declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `DateTime` -- an instant in UTC, in milliseconds.
template <> struct FieldTypeOf<DateTime> {
  static constexpr FieldType kType = FieldType::DateTime;   ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A DateTime declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `Guid` -- sixteen bytes.
template <> struct FieldTypeOf<Guid> {
  static constexpr FieldType kType = FieldType::Guid;       ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A GUID declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< Not an enumeration.
};

/// \brief `Blob` -- bytes of no declared length.
template <> struct FieldTypeOf<Blob> {
  static constexpr FieldType kType = FieldType::Blob;       ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A BLOB declares no length.
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

/// \brief The declared field table with the platform's own five appended.
///
/// \tparam T The generated table class.
/// \tparam N How many fields the `.al` file declares.
/// \param declared The fields the AL source names, in ascending field number.
/// \return All of them, followed by the system fields.
///
/// \note THE FIVE `Declare` CALLS LIVE HERE AND NOT IN 1 767 GENERATED FILES. Their numbers, names,
///       captions and offsets are the same in every table, so a generated file states them nowhere
///       and this one call carries them.
///
/// \note THE STORAGE CANNOT FOLLOW THEM HERE, and that is a language rule rather than a decision:
///       a standard-layout class has all its non-static data members in ONE class of its hierarchy,
///       so system fields in `Table<Derived>` and AL fields in the generated class would leave the
///       record non-standard-layout -- and `offsetof`, which is how the field table reaches every
///       field, is only conditionally supported there. The five members therefore stay in the
///       generated class, and everything about them that is not storage stays here.
///
/// \note The result stays SORTED, which `FieldsAreSorted` asserts beside every table: the reserved
///       range starts at 2000000000 and the largest field number in the BaseApp is 99 008 500.
template <typename T, std::size_t N>
[[nodiscard]] constexpr std::array<FieldDef, N + kSystemFieldCount>
WithSystemFields(const std::array<FieldDef, N> &declared) {
  std::array<FieldDef, N + kSystemFieldCount> all{};
  for (std::size_t i = 0; i < N; ++i) { all[i] = declared[i]; }
  all[N] = Declare<&T::SystemId>(
      kSystemFields[0].no, kSystemFields[0].name, kSystemFields[0].name, offsetof(T, SystemId));
  all[N + 1] = Declare<&T::SystemCreatedAt>(kSystemFields[1].no,
                                            kSystemFields[1].name,
                                            kSystemFields[1].name,
                                            offsetof(T, SystemCreatedAt));
  all[N + 2] = Declare<&T::SystemCreatedBy>(kSystemFields[2].no,
                                            kSystemFields[2].name,
                                            kSystemFields[2].name,
                                            offsetof(T, SystemCreatedBy));
  all[N + 3] = Declare<&T::SystemModifiedAt>(kSystemFields[3].no,
                                             kSystemFields[3].name,
                                             kSystemFields[3].name,
                                             offsetof(T, SystemModifiedAt));
  all[N + 4] = Declare<&T::SystemModifiedBy>(kSystemFields[4].no,
                                             kSystemFields[4].name,
                                             kSystemFields[4].name,
                                             offsetof(T, SystemModifiedBy));
  return all;
}

} // namespace agiru
