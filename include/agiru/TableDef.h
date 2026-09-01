#pragma once

#include "agiru/Ids.h"

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

  /// \brief The member names of an Option or Enum field, empty otherwise.
  ///
  /// They live in the metadata rather than in the value because they are the same for every record
  /// of the table: one `.rodata` run for the whole table instead of a copy per instance. An error
  /// message needs them, since AL renders an option by its member name and never by its ordinal.
  std::span<const std::string_view> members;
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
/// AL addresses fields by NUMBER and never by position: numbers are sparse in real tables, where an
/// obsoleted field leaves its number behind and nothing fills the gap. A linear walk is right at
/// these sizes -- the widest BaseApp table has a few hundred fields in one contiguous `.rodata`
/// run, and a binary search would touch more cache lines than it saves.
///
/// \note A free function rather than a method, so that `TableDef`, `FieldDef` and `KeyDef` stay
///       plain aggregates: the generator writes them with designated initializers.
[[nodiscard]] constexpr const FieldDef *Field(const TableDef &table, FieldNo no) {
  for (const FieldDef &f : table.fields) {
    if (f.no == no) { return &f; }
  }
  return nullptr;
}

} // namespace agiru
