#pragma once

#include <string>

namespace agiru {

struct FieldDef;
struct TableDef;

namespace platform {
class Field;
}

namespace detail {

std::string FieldOptionMembers(const FieldDef &def);

void LoadFieldMetadata(platform::Field &row, const TableDef &table, const FieldDef &def);

}

}
