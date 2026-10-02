#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "platform/PageMetadata.h"
#include "platform/ReflectionTypes.h"
#include "platform/TableMetadata.h"
#include "runtime/ErrorValue.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Integer.h"

#include "Check.h"
#include "ReflectionMetadata.h"

#include <array>
#include <bit>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace {

constexpr std::uint8_t kInvalidType = 255;
constexpr agiru::Integer kTemporaryId = 50175;

void PageTypes() {
  using Property = agiru::PageType;
  using Native = agiru::platform::PageMetadataPageType;
  constexpr std::array supported{
      std::pair{Property::Card, Native::Card},
      std::pair{Property::List, Native::List},
      std::pair{Property::RoleCenter, Native::RoleCenter},
      std::pair{Property::CardPart, Native::CardPart},
      std::pair{Property::ListPart, Native::ListPart},
      std::pair{Property::Document, Native::Document},
      std::pair{Property::Worksheet, Native::Worksheet},
      std::pair{Property::ListPlus, Native::ListPlus},
      std::pair{Property::ConfirmationDialog, Native::ConfirmationDialog},
      std::pair{Property::NavigatePage, Native::NavigatePage},
      std::pair{Property::StandardDialog, Native::StandardDialog},
      std::pair{Property::Api, Native::Api},
      std::pair{Property::HeadlinePart, Native::HeadlinePart},
  };
  for (const auto &[property, native] : supported) {
    const auto result = agiru::detail::MetadataPageType(property);
    CHECK_TRUE("each source-declared page kind maps by identity", result && *result == native);
  }
  constexpr std::array refused{
      std::pair{Property::ReportPreview, std::string_view{"ReportPreview"}},
      std::pair{Property::ReportProcessingOnly, std::string_view{"ReportProcessingOnly"}},
      std::pair{Property::XmlPort, std::string_view{"XmlPort"}},
      std::pair{Property::PromptDialog, std::string_view{"PromptDialog"}},
      std::pair{Property::ConfigurationDialog, std::string_view{"ConfigurationDialog"}},
      std::pair{Property::UserControlHost, std::string_view{"UserControlHost"}},
  };
  for (const auto &[property, name] : refused) {
    const auto result = agiru::detail::MetadataPageType(property);
    CHECK_TRUE("absent page metadata members refuse by name",
               !result && result.error().contains(name));
  }
  const auto invalid = agiru::detail::MetadataPageType(std::bit_cast<Property>(kInvalidType));
  CHECK_TRUE("unknown page kinds never become Card",
             !invalid && invalid.error().contains("unknown"));
  CHECK_TRUE("reflection has exactly thirteen source members",
             agiru::OptionTraits<Native>::kValues.size() == supported.size());
  CHECK_TRUE("HeadlinePart's reflection ordinal is not its property enum ordinal",
             static_cast<std::int32_t>(Native::HeadlinePart) !=
                 static_cast<std::int32_t>(Property::HeadlinePart));
}

void TableTypes() {
  using Property = agiru::TableType;
  using Native = agiru::platform::TableMetadataTableType;
  constexpr std::array supported{
      std::pair{Property::Normal, Native::Normal},
      std::pair{Property::CRM, Native::CRM},
      std::pair{Property::ExternalSQL, Native::ExternalSQL},
      std::pair{Property::Exchange, Native::Exchange},
      std::pair{Property::MicrosoftGraph, Native::MicrosoftGraph},
      std::pair{Property::Temporary, Native::Temporary},
  };
  for (const auto &[property, native] : supported) {
    const auto result = agiru::detail::MetadataTableType(property);
    CHECK_TRUE("table property kinds map to their declared reflection identities",
               result && *result == native);
  }
  const auto cds = agiru::detail::MetadataTableType(Property::CDS);
  CHECK_TRUE("CDS cannot be reinterpreted as Query", !cds && cds.error().contains("CDS"));
  CHECK_TRUE("the refusal retains the distinct Query vocabulary",
             !cds && cds.error().contains("Query"));
  const auto invalid = agiru::detail::MetadataTableType(std::bit_cast<Property>(kInvalidType));
  CHECK_TRUE("unknown table kinds never become Normal",
             !invalid && invalid.error().contains("unknown"));
  CHECK_TRUE("native Query remains the sixth source member",
             agiru::OptionTraits<Native>::kValues[5].name == "Query");
  CHECK_TRUE("ExternalSQL's source ordinal is not its property enum ordinal",
             static_cast<std::int32_t>(Native::ExternalSQL) !=
                 static_cast<std::int32_t>(Property::ExternalSQL));
}

template <typename Row> void MissingProviderIsNotAnEmptySnapshot() {
  Row row{};
  const auto original = row.SystemId;
  const auto refuses = [&](auto operation) {
    bool refused = false;
    try {
      operation();
    } catch (const agiru::Error &error) {
      const std::string message = error.what();
      refused = message.contains(Row::kName) && message.contains("live ") &&
                message.contains("schema identity");
    }
    CHECK_TRUE("unqualified live metadata refuses before SQL or invented emptiness", refused);
  };
  refuses([&] { agiru::RequireTableProvider(agiru::TableTraits<Row>::kTable); });
  refuses([&] { static_cast<void>(row.FindFirst()); });
  refuses([&] { static_cast<void>(row.FindSet()); });
  refuses([&] { static_cast<void>(row.Get(kTemporaryId)); });
  refuses([&] { static_cast<void>(row.Count()); });
  refuses([&] { static_cast<void>(row.Insert()); });
  refuses([&] { static_cast<void>(row.Modify()); });
  refuses([&] { static_cast<void>(row.Delete()); });
  CHECK_TRUE("refused persistence never stamps an implicit system identity",
             row.SystemId == original);
  agiru::Temporary<Row> temporary;
  temporary.ID = kTemporaryId;
  temporary.Name = "Source name";
  temporary.Caption = "Different caption";
  temporary.Insert();
  CHECK_TRUE("temporary source rows remain usable", static_cast<bool>(temporary.Get(kTemporaryId)));
  CHECK_TEXT("Name is never reconstructed from Caption", temporary.Name.Value(), "Source name");
  CHECK_TEXT(
      "Caption retains independent source text", temporary.Caption.Value(), "Different caption");
  CHECK_TRUE("temporary storage uses the original ID key", temporary.Count() == 1);
}

}

int main() {
  return gate::Run("ReflectionMetadata", [] {
    PageTypes();
    TableTypes();
    MissingProviderIsNotAnEmptySnapshot<agiru::platform::PageMetadata>();
    MissingProviderIsNotAnEmptySnapshot<agiru::platform::TableMetadata>();
  });
}
