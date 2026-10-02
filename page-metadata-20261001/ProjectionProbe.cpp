#include "meta/PageDef.h"
#include "platform/PageMetadata.h"

#include "Check.h"

#include <array>
#include <cstdint>
#include <utility>

int main() {
  return gate::Run("PageMetadataProjection", [] {
    using Kind = agiru::platform::PageMetadataPageType;
    constexpr std::array pairs{
        std::pair{agiru::PageType::Card, Kind::Card},
        std::pair{agiru::PageType::List, Kind::List},
        std::pair{agiru::PageType::RoleCenter, Kind::RoleCenter},
        std::pair{agiru::PageType::CardPart, Kind::CardPart},
        std::pair{agiru::PageType::ListPart, Kind::ListPart},
        std::pair{agiru::PageType::Document, Kind::Document},
        std::pair{agiru::PageType::Worksheet, Kind::Worksheet},
        std::pair{agiru::PageType::ListPlus, Kind::ListPlus},
        std::pair{agiru::PageType::ConfirmationDialog, Kind::ConfirmationDialog},
        std::pair{agiru::PageType::NavigatePage, Kind::NavigatePage},
        std::pair{agiru::PageType::StandardDialog, Kind::StandardDialog},
        std::pair{agiru::PageType::Api, Kind::Api},
        std::pair{agiru::PageType::HeadlinePart, Kind::HeadlinePart}};
    for (const auto &[property, option] : pairs) {
      CHECK_TRUE("projection uses declared member identity, not internal ordinal",
                 agiru::platform::PageMetadataTypeOf(property) == option);
    }
    constexpr std::array missing{agiru::PageType::ReportPreview,
                                 agiru::PageType::ReportProcessingOnly,
                                 agiru::PageType::XmlPort,
                                 agiru::PageType::PromptDialog,
                                 agiru::PageType::ConfigurationDialog,
                                 agiru::PageType::UserControlHost};
    for (const auto property : missing) {
      CHECK_TRUE("absent source member cannot silently become Card",
                 !agiru::platform::PageMetadataTypeOf(property).has_value());
    }
    CHECK_TRUE("unknown type cannot silently become Card",
               !agiru::platform::PageMetadataTypeOf(static_cast<agiru::PageType>(255)).has_value());
  });
}
