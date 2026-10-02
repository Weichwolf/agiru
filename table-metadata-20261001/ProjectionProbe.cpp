#include "meta/TableDef.h"
#include "platform/TableMetadata.h"

#include "Check.h"

#include <array>
#include <cstdint>
#include <optional>
#include <utility>

int main() {
  return gate::Run("TableMetadataProjection", [] {
    using Kind = agiru::platform::TableMetadataTableType;
    constexpr std::array pairs{
        std::pair{agiru::TableType::Normal, Kind::Normal},
        std::pair{agiru::TableType::CRM, Kind::CRM},
        std::pair{agiru::TableType::ExternalSQL, Kind::ExternalSQL},
        std::pair{agiru::TableType::Exchange, Kind::Exchange},
        std::pair{agiru::TableType::MicrosoftGraph, Kind::MicrosoftGraph},
        std::pair{agiru::TableType::Temporary, Kind::Temporary}};
    for (const auto &[property, option] : pairs) {
      CHECK_TRUE("projection uses member identity, not internal property ordinal",
                 agiru::platform::TableMetadataTypeOf(property) == option);
    }
    CHECK_TRUE("CDS cannot be guessed as the source Query member",
               !agiru::platform::TableMetadataTypeOf(agiru::TableType::CDS).has_value());
    CHECK_TRUE("unknown property cannot become Normal",
               !agiru::platform::TableMetadataTypeOf(static_cast<agiru::TableType>(255)).has_value());
  });
}
