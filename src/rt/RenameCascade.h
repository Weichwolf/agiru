#pragma once

#include "meta/TableDef.h"

namespace agiru::detail {

void CascadeRename(const void *record, const void *before, const TableDef &table);

}
