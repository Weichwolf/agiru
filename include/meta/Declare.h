#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
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
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Media.h"
#include "type/MediaSet.h"
#include "type/Option.h"
#include "type/RecordId.h"
#include "type/TableFilter.h"
#include "type/Text.h"
#include "type/Time.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
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

/// \brief `DateFormula` -- a date calculation written down.
template <> struct FieldTypeOf<DateFormula> {
  static constexpr FieldType kType = FieldType::DateFormula; ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;                ///< A formula declares no length.
  static constexpr std::span<const EnumValueDef> kValues{};  ///< Not an enumeration.
};

/// \brief `TableFilter` -- a filter stored in a field and applied to another table.
template <> struct FieldTypeOf<TableFilter> {
  static constexpr FieldType kType = FieldType::TableFilter; ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;                ///< A filter declares no length.
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

/// \brief `Media` -- one media object, stored as the identifier that finds it.
template <> struct FieldTypeOf<Media> {
  static constexpr FieldType kType = FieldType::Media;      ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A Media declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< It names no members.
};

/// \brief `MediaSet` -- a collection of media objects, stored as the set's identifier.
template <> struct FieldTypeOf<MediaSet> {
  static constexpr FieldType kType = FieldType::MediaSet;   ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< A MediaSet declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< It names no members.
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

/// \brief `Option<>` -- an option field that names no members at all.
///
/// \note AN OPTION WITH NO VOCABULARY IS AL'S OWN DECLARATION AND NOT A GAP. `procedure P(Type:
///       Option)` takes any ordinal, and a platform table whose member ORDER nothing states carries
///       the ordinal without claiming names it cannot check (board:0032).
template <> struct FieldTypeOf<Option<void>> {
  static constexpr FieldType kType = FieldType::Option;     ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< An option declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< No members are claimed.
};

/// \brief `Enum` -- an enum field, whose values come from the enum object it names.
template <typename E> struct FieldTypeOf<Enum<E>> {
  static constexpr FieldType kType = FieldType::Enum; ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;         ///< An enum declares no length.

  /// The declared values, taken from the enum object rather than repeated per field.
  static constexpr std::span<const EnumValueDef> kValues{EnumTraits<E>::kValues};
};

/// \brief `Enum<>` -- an enum field whose declaration this run never read.
///
/// It names no member, which is what `Enum<void>` says: the ordinal is carried and nothing claims
/// to know what it means. The transpiler names every unresolved enumeration in its summary.
template <> struct FieldTypeOf<Enum<void>> {
  static constexpr FieldType kType = FieldType::Enum;       ///< The AL type tag.
  static constexpr std::uint16_t kLength = 0;               ///< An enum declares no length.
  static constexpr std::span<const EnumValueDef> kValues{}; ///< No declaration was in reach.
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
/// \param declared What the `.al` file's `properties` block said about the field.
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
/// \brief What a field's `properties` block declares, beyond what its type carries.
///
/// \note IT IS ONE VALUE AND NOT FOURTEEN PARAMETERS. AL writes these as a block of named
///       assignments and the generator emits them the same way, so a reader compares the two side
///       by side -- and a field that declares none costs one defaulted argument.
struct Declared {
  std::optional<std::string_view> initValue{};                  ///< `InitValue`, column spelling.
  ::agiru::FieldClass fieldClass = ::agiru::FieldClass::Normal; ///< `FieldClass`.
  std::string_view calcFormula{};                               ///< `CalcFormula`, as AL wrote it.
  bool notBlank = false;                                        ///< `NotBlank`.
  bool autoIncrement = false;                                   ///< `AutoIncrement`.
  bool editable = true;                                         ///< `Editable`.
  bool validateTableRelation = true;                            ///< `ValidateTableRelation`.
  std::string_view relationTable{}; ///< `TableRelation`'s target table, empty when the declaration
                                    ///< is conditional or filtered (board:0043).
  std::string_view relationField{}; ///< Its target field, empty when the relation names the table's
                                    ///< own primary key.
  std::string_view relation{};      ///< The whole `TableRelation`, for the conditional and filtered
                                    ///< forms (board:0658).
  bool blankZero = false;           ///< `BlankZero`.
  std::string_view minValue{};      ///< `MinValue`, as AL wrote it.
  std::string_view maxValue{};      ///< `MaxValue`, as AL wrote it.
  std::string_view decimalPlaces{}; ///< `DecimalPlaces`.
  std::string_view blankNumbers{};  ///< `BlankNumbers`.
  bool compressed = true;           ///< `Compressed`, BLOB only.
  bool numeric = false;             ///< `Numeric`.
  std::string_view charAllowed{};   ///< `CharAllowed`, as AL wrote it.
  std::string_view valuesAllowed{}; ///< `ValuesAllowed`, as AL wrote it.
  bool closingDates = false;        ///< `ClosingDates`.
  std::string_view extendedDataType{};      ///< `ExtendedDataType`, as AL wrote it.
  std::string_view maskType{};              ///< `MaskType`, as AL wrote it.
  std::string_view toolTip{};               ///< `ToolTip`, which a table field may declare.
  std::string_view accessByPermission{};    ///< `AccessByPermission`.
  PageId lookupPageId{};                    ///< `LookupPageId`.
  PageId drillDownPageId{};                 ///< `DrillDownPageId`.
  bool optimizeForTextSearch = false;       ///< `OptimizeForTextSearch`.
  std::string_view captionClass{};          ///< `CaptionClass`.
  std::uint16_t width = 0;                  ///< `Width`, 0 when none is declared.
  std::string_view autoFormatType{};        ///< `AutoFormatType`.
  std::string_view autoFormatExpression{};  ///< `AutoFormatExpression`.
  std::string_view allowInCustomizations{}; ///< Effective declaring-owner `AllowInCustomizations`.
  std::string_view access{};                ///< `Access`, as AL wrote it.
  std::string_view subtype{};               ///< `Subtype`, on a Blob or Media field.
  bool enabled = true;                      ///< `Enabled`, on a field.
  std::string_view movedFrom{};             ///< `MovedFrom`, the app the field came from.
  std::string_view movedTo{};               ///< `MovedTo`, the app the field went to.
  std::string_view description{};           ///< `Description`, which nothing reads.
  std::string_view obsoleteState{};         ///< `ObsoleteState`.
  std::string_view obsoleteReason{};        ///< `ObsoleteReason`.
  std::string_view obsoleteTag{};           ///< `ObsoleteTag`.
  std::string_view externalName{};          ///< `ExternalName`, on a field of an external table.
  std::string_view optionOrdinalValues{};   ///< `OptionOrdinalValues`, a member's foreign number.
  bool sqlTimestamp = false;                ///< `SqlTimestamp`.
};

template <auto Member>
constexpr FieldDef Declare(FieldNo no,
                           std::string_view name,
                           std::string_view caption,
                           std::size_t offset,
                           const Declared &declared = {}) {
  using Value = typename MemberOwnerOf<decltype(Member)>::Field;
  return FieldDef{
      .offset = offset,
      .name = name,
      .caption = caption,
      .values = FieldTypeOf<Value>::kValues,
      .calcFormula = declared.calcFormula,
      .relationTable = declared.relationTable,
      .relationField = declared.relationField,
      .relation = declared.relation,
      .minValue = declared.minValue,
      .maxValue = declared.maxValue,
      .decimalPlaces = declared.decimalPlaces,
      .blankNumbers = declared.blankNumbers,
      .charAllowed = declared.charAllowed,
      .valuesAllowed = declared.valuesAllowed,
      .extendedDataType = declared.extendedDataType,
      .maskType = declared.maskType,
      .toolTip = declared.toolTip,
      .accessByPermission = declared.accessByPermission,
      .captionClass = declared.captionClass,
      .autoFormatType = declared.autoFormatType,
      .autoFormatExpression = declared.autoFormatExpression,
      .allowInCustomizations = declared.allowInCustomizations,
      .access = declared.access,
      .subtype = declared.subtype,
      .movedFrom = declared.movedFrom,
      .movedTo = declared.movedTo,
      .description = declared.description,
      .obsoleteState = declared.obsoleteState,
      .obsoleteReason = declared.obsoleteReason,
      .obsoleteTag = declared.obsoleteTag,
      .externalName = declared.externalName,
      .optionOrdinalValues = declared.optionOrdinalValues,
      .initValue = declared.initValue,
      .no = no,
      .fieldClass = declared.fieldClass,
      .lookupPageId = declared.lookupPageId,
      .drillDownPageId = declared.drillDownPageId,
      .length = FieldTypeOf<Value>::kLength,
      .width = declared.width,
      .type = FieldTypeOf<Value>::kType,
      .notBlank = declared.notBlank,
      .autoIncrement = declared.autoIncrement,
      .editable = declared.editable,
      .validateTableRelation = declared.validateTableRelation,
      .blankZero = declared.blankZero,
      .compressed = declared.compressed,
      .numeric = declared.numeric,
      .closingDates = declared.closingDates,
      .optimizeForTextSearch = declared.optimizeForTextSearch,
      .sqlTimestamp = declared.sqlTimestamp,
      .enabled = declared.enabled,
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

namespace detail {

/// \brief Original storage type of a known implicit declaration; unknown types refuse.
/// \param field Canonical source-owned declaration. \return Its runtime type tag.
constexpr FieldType ImplicitFieldType(const SystemFieldDecl &field) {
  if (field.alType == "Guid") { return FieldType::Guid; }
  if (field.alType == "DateTime") { return FieldType::DateTime; }
  if (field.alType == "BigInteger") { return FieldType::BigInteger; }
  if (field.alType == "Text") { return FieldType::Text; }
  throw Error("unknown implicit field type");
}

/// \brief Binds one canonical declaration to actual typed storage; mismatches refuse.
/// \tparam Member Typed record member. \param no Original field number.
/// \param offset Actual standard-layout offset. \return Validated immutable metadata.
template <auto Member> constexpr FieldDef ImplicitField(FieldNo no, std::size_t offset) {
  for (const auto &field : kImplicitSystemFields) {
    if (field.no != no) { continue; }
    const auto declaration = Declare<Member>(
        no,
        field.name,
        field.name,
        offset,
        Declared{.fieldClass = field.role == SystemFieldRole::AuditLookup ? FieldClass::FlowField
                                                                          : FieldClass::Normal,
                 .calcFormula = field.calcFormula,
                 .editable = field.role != SystemFieldRole::Timestamp &&
                             field.role != SystemFieldRole::AuditLookup,
                 .sqlTimestamp = field.role == SystemFieldRole::Timestamp});
    if (declaration.type != ImplicitFieldType(field) || declaration.length != field.length) {
      throw Error("implicit field storage violates the original type or capacity");
    }
    return declaration;
  }
  throw Error("unknown implicit field identity");
}

}

/// \brief Materializes a complete host-selected profile using actual record member offsets.
/// \tparam T Standard-layout record with storage for every selected implicit member.
/// \tparam Profile Explicit host capability, not an app's version/minimum runtime.
/// \tparam Type AL source table kind.
/// \tparam Linked Original LinkedObject property.
/// \tparam N Source-declared field count.
/// \param declared Sorted positive source field numbers, below the reserved range.
/// \return Timestamp, source declarations and selected reserved fields, sorted by number.
/// \note User-name fields are nonstored FlowFields over the shared lookup evaluator.
///       Metadata allocation does not provide durable rowversion/audit or a live catalogue.
template <typename T, SystemFieldProfile Profile, TableType Type, bool Linked, std::size_t N>
[[nodiscard]] constexpr auto WithImplicitFields(const std::array<FieldDef, N> &declared) {
  static_assert(std::is_standard_layout_v<T>);
  for (std::size_t i = 0; i < N; ++i) {
    if (declared[i].no.Value() <= 0 || IsReservedSystemField(declared[i].no) ||
        (i > 0 && declared[i - 1].no.Value() >= declared[i].no.Value())) {
      throw Error("implicit profile requires sorted, distinct, nonreserved source fields");
    }
  }
  std::array<FieldDef, N + ImplicitFieldCount(Profile, Type, Linked)> all{};
  all[0] = detail::ImplicitField<&T::SystemRowVersion>(SystemFieldNumbers::SystemRowVersion,
                                                       offsetof(T, SystemRowVersion));
  for (std::size_t i = 0; i < N; ++i) { all[i + 1] = declared[i]; }
  std::size_t next = N + 1;
  all[next++] =
      detail::ImplicitField<&T::SystemId>(SystemFieldNumbers::SystemId, offsetof(T, SystemId));
  if constexpr (CarriesAuditFields(Type, Linked)) {
    all[next++] = detail::ImplicitField<&T::SystemCreatedAt>(SystemFieldNumbers::SystemCreatedAt,
                                                             offsetof(T, SystemCreatedAt));
    all[next++] = detail::ImplicitField<&T::SystemCreatedBy>(SystemFieldNumbers::SystemCreatedBy,
                                                             offsetof(T, SystemCreatedBy));
    all[next++] = detail::ImplicitField<&T::SystemModifiedAt>(SystemFieldNumbers::SystemModifiedAt,
                                                              offsetof(T, SystemModifiedAt));
    all[next++] = detail::ImplicitField<&T::SystemModifiedBy>(SystemFieldNumbers::SystemModifiedBy,
                                                              offsetof(T, SystemModifiedBy));
    if constexpr (Profile == SystemFieldProfile::Runtime18) {
      all[next++] = detail::ImplicitField<&T::SystemCreatedByUserName>(
          SystemFieldNumbers::SystemCreatedByUserName, offsetof(T, SystemCreatedByUserName));
      all[next++] = detail::ImplicitField<&T::SystemCreatedByFullName>(
          SystemFieldNumbers::SystemCreatedByFullName, offsetof(T, SystemCreatedByFullName));
      all[next++] = detail::ImplicitField<&T::SystemModifiedByUserName>(
          SystemFieldNumbers::SystemModifiedByUserName, offsetof(T, SystemModifiedByUserName));
      all[next++] = detail::ImplicitField<&T::SystemModifiedByFullName>(
          SystemFieldNumbers::SystemModifiedByFullName, offsetof(T, SystemModifiedByFullName));
    }
  }
  return all;
}

}
