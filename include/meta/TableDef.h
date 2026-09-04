#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

/// \file
/// \brief The static declaration of an AL table: its fields, its keys, and how to reach them.

namespace agiru {

/// \brief An AL field's data type, as far as the generator can emit it.
///
/// This list grows as the generator learns types. A type it cannot emit is a translation error
/// rather than a silent fallback to Text.
///
/// \warning THE NUMBERS ARE THE PLATFORM'S OWN AND NOT A COUNTER. `FieldRef.Type()` and
///          `Field.Type` return this, and AL compares the result against `Field.Type::Code`
///          directly -- so a dense 0, 1, 2 ... of this tree's own invention would make every such
///          comparison quietly false. The sparse values are BC's: measured from
///          `~/Git/openerp/openerp/runtime/al_system_enums.py`, which mirrors AL's `FieldType`
///          system type, and cross-checked against `fieldtype-option.md`, whose eighteen documented
///          members are exactly a subset of them.
enum class FieldType : std::uint8_t {
  Boolean = 3,      ///< AL `Boolean`.
  Option = 5,       ///< AL `Option`, and what an Enum field reports too.
  Integer = 7,      ///< AL `Integer`.
  Decimal = 9,      ///< AL `Decimal`.
  Date = 11,        ///< AL `Date`.
  Time = 12,        ///< AL `Time`.
  Blob = 14,        ///< AL `Blob`.
  DateFormula = 15, ///< AL `DateFormula`.
  BigInteger = 18,  ///< AL `BigInteger`.
  Duration = 20,    ///< AL `Duration`.
  Guid = 21,        ///< AL `Guid`.
  DateTime = 22,    ///< AL `DateTime`.
  RecordId = 23,    ///< AL `RecordId`.
  TableFilter = 24, ///< AL `TableFilter`, which only the Permission table uses.
  Text = 31,        ///< AL `Text`.
  Code = 33,        ///< AL `Code`.
  MediaSet = 39,    ///< AL `MediaSet`.
  Media = 40,       ///< AL `Media`.

  /// \brief An enum field, which the PLATFORM does not distinguish and this metadata does.
  ///
  /// It is deliberately outside the platform's own range: `FieldRef::Type()` reports `Option` for
  /// a field declared this way, because `fieldtype-option.md` has no `Enum` member at all, and
  /// `IsEnum()` is the one place BC puts the difference. So this value never leaves the metadata,
  /// and giving it a platform number would let it escape looking like one.
  Enum = 200,
};

/// \brief One field's declaration, as static const data.
///
/// Every `FieldDef` the generator writes is a `constexpr` aggregate in `.rodata`: demand-paged by
/// the kernel, shared between processes, costing nothing until it is touched and nothing at
/// startup. The predecessor built the equivalent as heap objects while loading and paid about a
/// gigabyte per process for it (CLAUDE.md, board:0006).
///
/// \note `offset` is what lets the runtime reach a field by number without a virtual call and
///       without a map, and it is why a generated record must be standard-layout.
struct FieldDef {
  FieldNo no;               ///< The AL field number.
  std::string_view name;    ///< The AL name, spaces and all: `"Work Type Code"`.
  std::string_view caption; ///< The `Caption` property, which AL error messages quote.
  FieldType type;           ///< The AL data type.
  std::uint16_t length;     ///< Declared length for Code and Text, 0 otherwise.
  std::size_t offset;       ///< `offsetof` within the generated record.

  /// \brief The declared values of an Option or Enum field, empty otherwise.
  ///
  /// They live in the metadata rather than in the value because they are the same for every record
  /// of the table: one `.rodata` run for the whole table instead of a copy per instance. An error
  /// message needs them, since AL renders either enumeration by its value name and never by its
  /// ordinal.
  std::span<const EnumValueDef> values;

  /// \brief The `InitValue` property, as the COLUMN spells it, or nothing when AL declared none.
  ///
  /// `devenv-initvalue-property.md`: "Sets the initial value of this field when a user creates a
  /// new record", and it is what `Init`, `Clear` and `ClearAll` reach for. 815 fields declare one
  /// under `Layers/W1` (measured 2026-09-04), most of them `true` on a Boolean.
  ///
  /// \note THE MEMBER NAME IS RESOLVED TO ITS ORDINAL BY THE GENERATOR. AL writes
  ///       `InitValue = "Gen. Prod. Posting Group"` and the column holds a number, so the
  ///       translation happens where the enumeration is in scope and not at run time.
  ///
  /// \note EMPTY IS NOT ABSENT. `InitValue = ''` on a Code field is a declaration and an absent
  ///       property is not, and a bare `string_view` could not tell them apart.
  std::optional<std::string_view> initValue;
};

/// \brief One key's declaration. The first key of a table is its primary key.
struct KeyDef {
  std::string_view name;           ///< The AL key name, `Key1` by convention.
  std::span<const FieldNo> fields; ///< The key fields, in declaration order.
  bool clustered;                  ///< The `Clustered` property.
};

/// \brief How many system fields the platform adds to every table.
///
/// `devenv-table-system-fields.md` tabulates five with their numbers -- SystemId 2000000000,
/// SystemCreatedAt 2000000001, SystemCreatedBy 2000000002, SystemModifiedAt 2000000003,
/// SystemModifiedBy 2000000004 -- and says they are "automatically included in every table object
/// by the platform".
///
/// \note IT IS NAMED SO THAT A GENERATED ASSERTION CAN KEEP THE AL NUMBER VISIBLE. A table's field
///       count is asserted as `declared + kSystemFieldCount`, so a reader checks the first number
///       against the `.al` file and the second against this page. Folding them into one total would
///       leave nothing that either source can be compared with.
///
/// \note SystemRowVersion is NOT among them. AL exposes the SQL rowversion under that name and the
///       page gives it no field number, unlike the five it tabulates (board:0013).
inline constexpr std::size_t kSystemFieldCount = 5;

/// \brief One table's declaration.
struct TableDef {
  TableId id;                       ///< The AL table number.
  std::string_view name;            ///< The AL name: `"Resource Cost"`.
  std::string_view caption;         ///< The `Caption` property.
  std::span<const FieldDef> fields; ///< Every declared field, in declaration order.
  std::span<const KeyDef> keys;     ///< Every declared key; `keys[0]` is the primary key.
};

/// \brief Finds a field by its AL number.
///
/// \param table The table to search.
/// \param no    The AL field number.
/// \return The field, or `nullptr` when the table declares no such number.
///
/// \pre The field table is sorted by field number. The generator emits it that way and asserts it
///      beside every table, so the precondition is checked at compile time rather than trusted.
///
/// A binary search rather than a walk, and the numbers say why (measured over the BaseApp,
/// 2026-09-01): the median table has 9 fields but the widest has 240, and a `FieldDef` is some 70
/// bytes -- a walk over the widest touches around 17 KB, on a path `Validate` will take for every
/// field of every record. Sorting costs nothing: AL already declares 1 526 of 1 545 tables in
/// ascending order.
///
/// \note Field numbers are NOT dense and no index array can replace this: the median highest
///       number is 14 and the largest in the BaseApp is 99 008 500.
[[nodiscard]] constexpr const FieldDef *Field(const TableDef &table, FieldNo no) {
  std::size_t low = 0;
  std::size_t high = table.fields.size();
  while (low < high) {
    const std::size_t mid = low + ((high - low) / 2);
    if (table.fields[mid].no == no) { return &table.fields[mid]; }
    if (table.fields[mid].no < no) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return nullptr;
}

/// \brief Whether a field table is sorted by field number.
///
/// \param table The table to check.
/// \return True when Field() may binary-search it.
///
/// The generator asserts this beside every table it writes, so a mis-sorted table is a translation
/// error rather than a lookup that quietly finds nothing.
[[nodiscard]] constexpr bool FieldsAreSorted(const TableDef &table) {
  for (std::size_t i = 1; i < table.fields.size(); ++i) {
    if (!(table.fields[i - 1].no < table.fields[i].no)) { return false; }
  }
  return true;
}

/// \brief Finds a field by where it sits in the record.
///
/// \param table  The table to search.
/// \param offset The field's offset within the record.
/// \return The field, or `nullptr` when no field sits there.
///
/// This is what lets generated code name a FIELD where AL names a field -- `FieldError(Code)`
/// rather than `FieldError(Field_No::Code)` -- because the address of a member is enough to find
/// its declaration.
///
/// \note A walk rather than a search, because offsets have no useful order and this is only ever
///       reached on an error path, where one pass over a few hundred entries costs nothing.
[[nodiscard]] constexpr const FieldDef *FieldAtOffset(const TableDef &table, std::size_t offset) {
  for (const FieldDef &f : table.fields) {
    if (f.offset == offset) { return &f; }
  }
  return nullptr;
}

}
