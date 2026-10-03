// Generated from Modules/System/Utilities/LineNumberBuffer.Table.al. Do not edit.

#include "LineNumberBuffer.h"

#include "runtime/Catalogue.h"

// @door

namespace agiru::app::tables {

constexpr std::array<FieldDef, 2> kLineNumberBufferFields{{
    Declare<&LineNumberBuffer::OldLineNumber>(LineNumberBuffer::Field_No::OldLineNumber,
                                              "Old Line Number",
                                              "Old Line Number",
                                              offsetof(LineNumberBuffer, OldLineNumber)),
    Declare<&LineNumberBuffer::NewLineNumber>(LineNumberBuffer::Field_No::NewLineNumber,
                                              "New Line Number",
                                              "New Line Number",
                                              offsetof(LineNumberBuffer, NewLineNumber)),
}};

constexpr std::array<KeyDef, 1> kLineNumberBufferKeys{{
    KeyDef{.name = "Key1", .fields = LineNumberBuffer::kKey1, .clustered = true},
}};

constexpr TableDef kLineNumberBufferTable{
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

namespace {
const RegisterTable<LineNumberBuffer> kInCatalogue;
}

} // namespace agiru::app::tables
