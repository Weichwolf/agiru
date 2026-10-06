#pragma once

#include "meta/Ids.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {

struct FieldDef;
struct TableDef;

namespace platform {
class Field;
}

namespace detail {

struct CatalogueScan;

std::uint16_t EffectiveFieldLength(const FieldDef &def);

std::string_view ReflectionFieldName(const FieldDef &def);

std::string FieldOptionMembers(const FieldDef &def);

void LoadFieldMetadata(platform::Field &row, const TableDef &table, const FieldDef &def);

[[nodiscard]] std::optional<bool> GetInstalledFieldMetadata(void *record, const TableDef &table);

[[nodiscard]] bool IsInstalledFieldProvider(const TableDef &table);

void RequireFieldMetadataProjection(FieldNo no);

[[nodiscard]] bool ScanInstalledFields(const TableDef &table, const CatalogueScan &scan);

[[nodiscard]] std::optional<bool>
FindInstalledFields(void *record, const TableDef &table, std::string_view which);

[[nodiscard]] std::optional<std::int32_t>
NextInstalledField(void *record, const TableDef &table, std::int32_t steps);

[[nodiscard]] std::optional<std::int32_t>
CountInstalledFields(const void *record, const TableDef &table, bool firstOnly = false);

}

}
