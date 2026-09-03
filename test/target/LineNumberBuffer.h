// Generated from Modules/System/Utilities/LineNumberBuffer.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::app::tables {

class LineNumberBuffer_Table;
using LineNumberBuffer = LineNumberBuffer_Table;

class LineNumberBuffer_Table : public Table<LineNumberBuffer_Table> {
public:
  static constexpr TableId kId{283};
  static constexpr std::string_view kName{"Line Number Buffer"};

  detail::StateHandle State_Block;

  Integer OldLineNumber{};
  Integer NewLineNumber{};

  struct Field_No {
    static constexpr ::agiru::FieldNo OldLineNumber{1};
    static constexpr ::agiru::FieldNo NewLineNumber{2};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::OldLineNumber}};
};

inline constexpr std::array<FieldDef, 2> kLineNumberBufferFields{{
    Declare<&LineNumberBuffer::OldLineNumber>(LineNumberBuffer::Field_No::OldLineNumber,
                                              "Old Line Number",
                                              "Old Line Number",
                                              offsetof(LineNumberBuffer, OldLineNumber)),
    Declare<&LineNumberBuffer::NewLineNumber>(LineNumberBuffer::Field_No::NewLineNumber,
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
static_assert(offsetof(agiru::app::tables::LineNumberBuffer, State_Block) == 0,
              "the record variable's state is the FIRST member, which is how the base reaches it "
              "through the address of the object");
static_assert(std::is_standard_layout_v<LineNumberBuffer>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kLineNumberBufferFields.size() == 2, "table 283 declares 2 fields");

} // namespace agiru::app::tables

template <> struct agiru::TableTraits<agiru::app::tables::LineNumberBuffer> {
  static constexpr const TableDef &kTable = agiru::app::tables::kLineNumberBufferTable;
};
