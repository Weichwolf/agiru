#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief The static declaration of an AL table: its fields, its keys, and how to reach them.

namespace agiru {

/// \brief An AL field's data type, as far as the generator can emit it.
///
/// This list grows as the generator learns types. A type it cannot emit is a translation error
/// rather than a silent fallback to Text.
enum class FieldType : std::uint8_t {
  Boolean,
  Integer,
  BigInteger,
  Decimal,
  Code,
  Text,
  Date,
  Time,
  DateTime,
  Option,
  Enum,
  Guid,
  Blob,
  DateFormula,
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
};

/// \brief One key's declaration. The first key of a table is its primary key.
struct KeyDef {
  std::string_view name;           ///< The AL key name, `Key1` by convention.
  std::span<const FieldNo> fields; ///< The key fields, in declaration order.
  bool clustered;                  ///< The `Clustered` property.
};

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
/// rather than `FieldError(FieldNumber::Code)` -- because the address of a member is enough to find
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

} // namespace agiru
