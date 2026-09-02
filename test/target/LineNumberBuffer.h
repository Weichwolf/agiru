// Generated from Modules/System/Utilities/LineNumberBuffer.Table.al. Do not edit.

#pragma once

#include "agiru.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::app::tables {

class LineNumberBuffer : public Table<LineNumberBuffer> {
public:
  static constexpr TableId kId{283};
  static constexpr std::string_view kName{"Line Number Buffer"};

  Integer OldLineNumber{};
  Integer NewLineNumber{};

  struct FieldNumber {
    static constexpr FieldNo OldLineNumber{1};
    static constexpr FieldNo NewLineNumber{2};
  };

  static constexpr std::array<FieldNo, 1> kKey1{{FieldNumber::OldLineNumber}};
};

inline constexpr std::array<FieldDef, 2> kLineNumberBufferFields{{
    Declare<&LineNumberBuffer::OldLineNumber>(LineNumberBuffer::FieldNumber::OldLineNumber,
                                              "Old Line Number",
                                              "Old Line Number",
                                              offsetof(LineNumberBuffer, OldLineNumber)),
    Declare<&LineNumberBuffer::NewLineNumber>(LineNumberBuffer::FieldNumber::NewLineNumber,
                                              "New Line Number",
                                              "New Line Number",
                                              offsetof(LineNumberBuffer, NewLineNumber)),
}};

inline constexpr std::array<KeyDef, 1> kLineNumberBufferKeys{{
    KeyDef{.name = "Key1", .fields = LineNumberBuffer::kKey1, .clustered = true},
}};

inline constexpr TableDef kLineNumberBufferTable{
    .id = LineNumberBuffer::kId,
    .name = LineNumberBuffer::kName,
    .caption = LineNumberBuffer::kName,
    .fields = kLineNumberBufferFields,
    .keys = kLineNumberBufferKeys,
};

static_assert(FieldsAreSorted(kLineNumberBufferTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(std::is_standard_layout_v<LineNumberBuffer>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kLineNumberBufferFields.size() == 2, "table 283 declares 2 fields");

} // namespace agiru::app::tables

template <> struct agiru::TableTraits<agiru::app::tables::LineNumberBuffer> {
  static constexpr const TableDef &kTable = agiru::app::tables::kLineNumberBufferTable;
};
