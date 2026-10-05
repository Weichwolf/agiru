#include "MetadataSystemId.h"

#include "meta/Ids.h"
#include "type/Guid.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace agiru::detail {

Guid MetadataSystemId(TableId provider, std::int32_t id1, std::int32_t id2, std::int32_t id3) {
  constexpr std::array<std::size_t, Guid::kSize> kGuidMemoryByte{
      3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15};
  constexpr unsigned kByteBits = 8;
  constexpr std::size_t kWordBytes = sizeof(std::uint32_t);
  const std::array parts{provider.Value(), id1, id2, id3};
  static_assert(sizeof(parts) == Guid::kSize);
  std::array<std::uint8_t, Guid::kSize> bytes{};
  for (std::size_t at = 0; at < bytes.size(); ++at) {
    const std::size_t memoryByte = kGuidMemoryByte[at];
    const auto part = static_cast<std::uint32_t>(parts[memoryByte / kWordBytes]);
    bytes[at] = static_cast<std::uint8_t>(part >> ((memoryByte % kWordBytes) * kByteBits));
  }
  return Guid{bytes};
}

}
