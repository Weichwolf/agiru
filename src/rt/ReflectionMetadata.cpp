#include "ReflectionMetadata.h"

#include "meta/EnumDef.h"
#include "meta/PageDef.h"
#include "meta/TableType.h"
#include "platform/ReflectionOptions.h"
#include "platform/ReflectionTypes.h"
#include "type/Option.h"
#include "type/Outcome.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace agiru::detail {

namespace {

bool SameProperty(std::string_view left, std::string_view right) {
  if (left.size() != right.size()) { return false; }
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(left[i])) !=
        std::tolower(static_cast<unsigned char>(right[i]))) {
      return false;
    }
  }
  return true;
}

template <typename Native>
Outcome<Native, std::string> MetadataProperty(std::string_view name, std::string_view property) {
  const auto value = MetadataPropertyOrdinal(OptionTraits<Native>::kValues, name, property);
  if (!value) { return Failed(value.error()); }
  return static_cast<Native>(*value);
}

}

Outcome<std::int32_t, std::string> MetadataPropertyOrdinal(std::span<const EnumValueDef> values,
                                                           std::string_view name,
                                                           std::string_view property,
                                                           std::string_view owner) {
  for (const auto &value : values) {
    if (SameProperty(name, value.name)) { return value.ordinal; }
  }
  return Failed(std::string(owner) + "." + std::string(property) + " has no verified member '" +
                std::string(name) + "'");
}

Outcome<platform::TableMetadataObsoleteState, std::string>
MetadataObsoleteState(std::string_view name) {
  return MetadataProperty<platform::TableMetadataObsoleteState>(name, "ObsoleteState");
}

Outcome<platform::TableMetadataCompressionType, std::string>
MetadataCompressionType(std::string_view name) {
  return MetadataProperty<platform::TableMetadataCompressionType>(name, "CompressionType");
}

Outcome<platform::TableMetadataScope, std::string> MetadataScope(std::string_view name) {
  using Native = platform::TableMetadataScope;
  constexpr std::array aliases{std::pair{std::string_view{"Extension"}, Native::Cloud},
                               std::pair{std::string_view{"Personalization"}, Native::Cloud},
                               std::pair{std::string_view{"Internal"}, Native::OnPrem}};
  for (const auto &[alias, value] : aliases) {
    if (SameProperty(name, alias)) { return value; }
  }
  return MetadataProperty<platform::TableMetadataScope>(name, "Scope");
}

Outcome<platform::TableMetadataAccess, std::string> MetadataAccess(std::string_view name) {
  return MetadataProperty<platform::TableMetadataAccess>(name, "Access");
}

Outcome<platform::FieldDataClassification, std::string>
MetadataDataClassification(std::string_view name) {
  return MetadataProperty<platform::FieldDataClassification>(name, "DataClassification");
}

Outcome<platform::PageMetadataPageType, std::string_view> MetadataPageType(PageType type) {
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
      return Failed("Page Metadata.PageType has no verified ReportPreview member");
    case PageType::ReportProcessingOnly:
      return Failed("Page Metadata.PageType has no verified ReportProcessingOnly member");
    case PageType::XmlPort: return Failed("Page Metadata.PageType has no verified XmlPort member");
    case PageType::PromptDialog:
      return Failed("Page Metadata.PageType has no verified PromptDialog member");
    case PageType::ConfigurationDialog:
      return Failed("Page Metadata.PageType has no verified ConfigurationDialog member");
    case PageType::UserControlHost:
      return Failed("Page Metadata.PageType has no verified UserControlHost member");
  }
  return Failed("unknown PageType cannot become Page Metadata.Card");
}

Outcome<platform::TableMetadataTableType, std::string_view> MetadataTableType(TableType type) {
  using Native = platform::TableMetadataTableType;
  switch (type) {
    case TableType::Normal: return Native::Normal;
    case TableType::CRM: return Native::CRM;
    case TableType::ExternalSQL: return Native::ExternalSQL;
    case TableType::Exchange: return Native::Exchange;
    case TableType::MicrosoftGraph: return Native::MicrosoftGraph;
    case TableType::Temporary: return Native::Temporary;
    case TableType::CDS: return Native::CRM;
  }
  return Failed("unknown TableType cannot become Table Metadata.Normal");
}

}
