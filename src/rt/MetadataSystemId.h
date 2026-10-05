#pragma once

#include "meta/Ids.h"
#include "type/Guid.h"

#include <cstdint>

namespace agiru::detail {

[[nodiscard]] Guid
MetadataSystemId(TableId provider, std::int32_t id1, std::int32_t id2 = 0, std::int32_t id3 = 0);

}
