#include "ReflectionMetadata.h"

#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "platform/ReflectionTypes.h"

#include <expected>
#include <string_view>

namespace agiru::detail {

std::expected<platform::PageMetadataPageType, std::string_view> MetadataPageType(PageType type) {
  using Native = platform::PageMetadataPageType;
  switch (type) {
    case PageType::Card: return Native::Card;
    case PageType::List: return Native::List;
    case PageType::RoleCenter: return Native::RoleCenter;
    case PageType::CardPart: return Native::CardPart;
    case PageType::ListPart: return Native::ListPart;
    case PageType::Document: return Native::Document;
    case PageType::Worksheet: return Native::Worksheet;
    case PageType::ListPlus: return Native::ListPlus;
    case PageType::ConfirmationDialog: return Native::ConfirmationDialog;
    case PageType::NavigatePage: return Native::NavigatePage;
    case PageType::StandardDialog: return Native::StandardDialog;
    case PageType::Api: return Native::Api;
    case PageType::HeadlinePart: return Native::HeadlinePart;
    case PageType::ReportPreview:
      return std::unexpected("Page Metadata.PageType has no verified ReportPreview member");
    case PageType::ReportProcessingOnly:
      return std::unexpected("Page Metadata.PageType has no verified ReportProcessingOnly member");
    case PageType::XmlPort:
      return std::unexpected("Page Metadata.PageType has no verified XmlPort member");
    case PageType::PromptDialog:
      return std::unexpected("Page Metadata.PageType has no verified PromptDialog member");
    case PageType::ConfigurationDialog:
      return std::unexpected("Page Metadata.PageType has no verified ConfigurationDialog member");
    case PageType::UserControlHost:
      return std::unexpected("Page Metadata.PageType has no verified UserControlHost member");
  }
  return std::unexpected("unknown PageType cannot become Page Metadata.Card");
}

std::expected<platform::TableMetadataTableType, std::string_view>
MetadataTableType(TableType type) {
  using Native = platform::TableMetadataTableType;
  switch (type) {
    case TableType::Normal: return Native::Normal;
    case TableType::CRM: return Native::CRM;
    case TableType::ExternalSQL: return Native::ExternalSQL;
    case TableType::Exchange: return Native::Exchange;
    case TableType::MicrosoftGraph: return Native::MicrosoftGraph;
    case TableType::Temporary: return Native::Temporary;
    case TableType::CDS:
      return std::unexpected(
          "Table Metadata.TableType has no verified CDS member; Query is distinct");
  }
  return std::unexpected("unknown TableType cannot become Table Metadata.Normal");
}

}
