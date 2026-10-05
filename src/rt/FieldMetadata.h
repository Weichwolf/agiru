#pragma once

#include <cstdint>
#include <string>

namespace agiru {

struct FieldDef;
struct TableDef;

namespace platform {
class Field;
}

namespace detail {

std::uint16_t EffectiveFieldLength(const FieldDef &def);

std::string FieldOptionMembers(const FieldDef &def);

void LoadFieldMetadata(platform::Field &row, const TableDef &table, const FieldDef &def);

}

}
