#pragma once

#include "agiru/Ids.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace agiru {

/// AL's field data types, as the ones the transpiler emits. `devenv-*-property.md` and the
/// per-type pages under `methods-auto/` are the source; this list grows as the generator learns
/// types, and a type it cannot emit is a translation error rather than a silent `Text`.
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

/// One field's declaration, as static const data.
///
/// EVERYTHING HERE LIVES IN `.rodata` AND IS NEVER BUILT AT RUNTIME. That is the decision the
/// 512 MB target forces (CLAUDE.md, board:0006): the predecessor built 9 300 objects' metadata as
/// heap objects at startup and paid a gigabyte per process for it. Emitted as `constexpr` arrays,
/// the same information is demand-paged by the kernel, shared between processes, and costs nothing
/// until it is touched.
///
/// `offset` is the field's position inside the record, which is how `RecordRef`/`FieldRef` reach a
/// field by number without a virtual call or a map lookup.
struct FieldDef {
  FieldNo no;
  std::string_view name;    ///< the AL name, spaces and all: "Work Type Code"
  std::string_view caption; ///< the Caption property, which AL error messages quote
  FieldType type;
  std::uint16_t length; ///< declared length for Code and Text, 0 otherwise
  std::size_t offset;   ///< offsetof within the record

  /// The member names of an Option or Enum field, empty otherwise.
  ///
  /// They sit in the metadata rather than in the value because they are the same for every record
  /// of the table: one `.rodata` run for the whole table instead of a copy per instance. An error
  /// message needs them -- AL renders an option by its member name, never by its ordinal.
  std::span<const std::string_view> members;
};

/// One key's declaration. The first key is the primary key and AL calls it Key1 by convention.
struct KeyDef {
  std::string_view name;
  std::span<const FieldNo> fields;
  bool clustered;
};

/// One table's declaration.
struct TableDef {
  TableId id;
  std::string_view name;    ///< the AL name: "Resource Cost"
  std::string_view caption; ///< the Caption property
  std::span<const FieldDef> fields;
  std::span<const KeyDef> keys;
};

/// The field with this number, or `nullptr`.
///
/// AL addresses fields by NUMBER, never by position -- field numbers are sparse in real tables,
/// where an obsoleted field leaves its number behind and nothing fills the gap. A linear walk is
/// right at these sizes: the widest table in the BaseApp has a few hundred fields, they sit in one
/// contiguous `.rodata` run, and a binary search would touch more cache lines than it saves.
///
/// A free function rather than a method, so that `TableDef`, `FieldDef` and `KeyDef` stay plain
/// aggregates: the generator writes them with designated initializers, which wants public data and
/// no member functions at all.
[[nodiscard]] constexpr const FieldDef *Field(const TableDef &table, FieldNo no) {
  for (const FieldDef &f : table.fields) {
    if (f.no == no) { return &f; }
  }
  return nullptr;
}

} // namespace agiru
