#pragma once

#include "meta/TableDef.h"
#include "runtime/RecordState.h"

#include <cstdint>
#include <string_view>

namespace agiru::detail {

[[nodiscard]] TempTable *TempOf(void *record);
[[nodiscard]] const TempTable *TempOf(const void *record);

bool TempInsert(void *record, const TableDef &table);
bool TempGet(void *record, const TableDef &table);
bool TempModify(void *record, const TableDef &table);
bool TempDelete(void *record, const TableDef &table);
std::int32_t TempDeleteAll(void *record, const TableDef &table);
std::int32_t TempCount(void *record, const TableDef &table);
void TempCalcSum(void *record, const TableDef &table, const FieldDef &def);
bool TempIsEmpty(void *record, const TableDef &table);
bool TempFindSet(void *record, const TableDef &table);
bool TempFind(void *record, const TableDef &table, std::string_view which);
std::int32_t TempNext(void *record, const TableDef &table, std::int32_t steps);

}
