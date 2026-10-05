#pragma once

#include "meta/EnumDef.h"
#include "meta/PageDef.h"
#include "meta/TableType.h"
#include "platform/ReflectionOptions.h"
#include "platform/ReflectionTypes.h"

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace agiru::detail {

std::expected<std::int32_t, std::string>
MetadataPropertyOrdinal(std::span<const EnumValueDef> values,
                        std::string_view name,
                        std::string_view property,
                        std::string_view owner = "Table Metadata");

std::expected<platform::PageMetadataPageType, std::string_view> MetadataPageType(PageType type);

std::expected<platform::TableMetadataTableType, std::string_view> MetadataTableType(TableType type);

std::expected<platform::TableMetadataObsoleteState, std::string>
MetadataObsoleteState(std::string_view name);

std::expected<platform::TableMetadataCompressionType, std::string>
MetadataCompressionType(std::string_view name);

std::expected<platform::TableMetadataScope, std::string> MetadataScope(std::string_view name);

std::expected<platform::TableMetadataAccess, std::string> MetadataAccess(std::string_view name);

std::expected<platform::FieldDataClassification, std::string>
MetadataDataClassification(std::string_view name);

}
