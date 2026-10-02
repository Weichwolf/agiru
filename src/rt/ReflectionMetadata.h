#pragma once

#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "platform/ReflectionTypes.h"

#include <expected>
#include <string_view>

namespace agiru::detail {

std::expected<platform::PageMetadataPageType, std::string_view> MetadataPageType(PageType type);

std::expected<platform::TableMetadataTableType, std::string_view> MetadataTableType(TableType type);

}
