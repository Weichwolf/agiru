#include "meta/ModuleDef.h"
#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "platform/PageMetadata.h"
#include "platform/ReflectionOptions.h"
#include "platform/ReflectionTypes.h"
#include "platform/TableMetadata.h"
#include "runtime/ErrorValue.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"

#include "Check.h"
#include "ReflectionMetadata.h"
#include "TableMetadata.h"

#include <array>
#include <bit>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

namespace {

constexpr std::uint8_t kInvalidType = 255;
constexpr agiru::Integer kTemporaryId = 50175;

constexpr agiru::ModuleDef kSourceModule{.id = "118874ab-44bc-4ccb-9daf-59763539ab16",
                                         .name = "Source App",
                                         .publisher = "agiru tests",
                                         .version = "1.2.3.4"};
constexpr std::array kSourceFields{agiru::FieldDef{.name = "ID", .no = agiru::FieldNo{1}},
                                   agiru::FieldDef{.name = "Key Text", .no = agiru::FieldNo{4}},
                                   agiru::FieldDef{.name = "Description", .no = agiru::FieldNo{9}}};
constexpr std::array kCaptionFields{agiru::FieldNo{9}, agiru::FieldNo{1}};
constexpr agiru::TableDef kSourceTable{.id = agiru::TableId{60074},
                                       .name = "Source name",
                                       .caption = "Independent caption",
                                       .fields = kSourceFields,
                                       .externalName = "external_source",
                                       .dataPerCompany = false,
                                       .replicateData = false,
                                       .compressionType = "Row",
                                       .dataCaptionFields = kCaptionFields,
                                       .inherentPermissions = "rm",
                                       .inherentEntitlements = "r",
                                       .access = "Internal",
                                       .lookupPageId = agiru::PageId{50176},
                                       .drillDownPageId = agiru::PageId{50177},
                                       .pasteIsValid = false,
                                       .obsoleteState = "Pending",
                                       .module = &kSourceModule,
                                       .nameSpace = "Microsoft.Fixture",
                                       .scope = "OnPrem",
                                       .obsoleteReason = "Original reason",
                                       .dataClassification = "AccountData",
                                       .linkedObject = true};

void TableProjection() {
  using namespace agiru::platform;
  const auto row = agiru::detail::ProjectTableMetadata(kSourceTable);
  CHECK_TRUE("original table ID survives projection", row.ID == kSourceTable.id.Value());
  CHECK_TEXT("Name remains independent of Caption", row.Name.Value(), "Source name");
  CHECK_TEXT("Caption retains its declared text", row.Caption.Value(), "Independent caption");
  CHECK_TRUE("declared company scope survives projection", !row.DataPerCompany);
  CHECK_TRUE("lookup page retains its exact source ID", row.LookupPageID == 50176);
  CHECK_TRUE("drilldown page retains its exact source ID", row.DrillDownPageID == 50177);
  CHECK_TEXT("caption fields use original numbers and declared order",
             row.DataCaptionFields.Value(),
             "9,1");
  CHECK_TRUE("paste and linked-object declarations are distinct",
             !row.PasteIsValid && row.LinkedObject);
  CHECK_TRUE("a linked normal table is not an external table type", !row.DataIsExternal);
  CHECK_TRUE("normal table reflection uses its native option member",
             row.TableType == TableMetadataTableType::Normal);
  CHECK_TEXT("external name is not a replacement for the AL name",
             row.ExternalName.Value(),
             "external_source");
  CHECK_TRUE("obsolete state retains its original option member",
             row.ObsoleteState == TableMetadataObsoleteState::Pending);
  CHECK_TEXT("obsolete reason survives projection", row.ObsoleteReason.Value(), "Original reason");
  CHECK_TRUE("table classification is not guessed from its fields",
             row.DataClassification == FieldDataClassification::AccountData);
  CHECK_TRUE("replication retains the source declaration", !row.ReplicateData);
  CHECK_TRUE("compression retains its native member",
             row.CompressionType == TableMetadataCompressionType::Row);
  CHECK_TRUE("App ID is parsed from the original declaring module",
             row.AppID == *agiru::Guid::FromText(kSourceModule.id));
  CHECK_TEXT(
      "inherent permissions are retained as declarations", row.InherentPermissions.Value(), "rm");
  CHECK_TEXT("entitlements are metadata, not commercial activation",
             row.InherentEntitlements.Value(),
             "r");
  CHECK_TRUE("Scope does not become the deployment target",
             row.Scope == TableMetadataScope::OnPrem);
  CHECK_TRUE("Access retains compile-time visibility", row.Access == TableMetadataAccess::Internal);
  CHECK_TEXT(
      "original AL namespace survives projection", row.ALNamespace.Value(), "Microsoft.Fixture");
  CHECK_TRUE("projection does not allocate per-session catalogue state",
             row.State_Block.Peek() == nullptr);
}

void TableProjectionVariants() {
  using Property = agiru::TableType;
  using Native = agiru::platform::TableMetadataTableType;
  for (const auto &[type, native, external] :
       {std::tuple{Property::Temporary, Native::Temporary, false},
        std::tuple{Property::ExternalSQL, Native::ExternalSQL, true},
        std::tuple{Property::CRM, Native::CRM, true},
        std::tuple{Property::Exchange, Native::Exchange, true},
        std::tuple{Property::MicrosoftGraph, Native::MicrosoftGraph, true}}) {
    auto source = kSourceTable;
    source.tableType = type;
    const auto row = agiru::detail::ProjectTableMetadata(source);
    CHECK_TRUE("declared table type and external-data classification stay aligned",
               row.TableType == native && static_cast<bool>(row.DataIsExternal) == external);
  }
  auto source = kSourceTable;
  source.dataCaptionFields = {};
  const auto noCaption = agiru::detail::ProjectTableMetadata(source);
  CHECK_TEXT("absent caption fields stay empty; the AL consumer chooses its primary-key fallback",
             noCaption.DataCaptionFields.Value(),
             "");
  constexpr std::array duplicate{agiru::FieldNo{1}, agiru::FieldNo{1}};
  source.dataCaptionFields = duplicate;
  const auto repeated = agiru::detail::ProjectTableMetadata(source);
  CHECK_TEXT("projection never deduplicates an explicitly repeated caption field",
             repeated.DataCaptionFields.Value(),
             "1,1");
}

void TableProjectionDefaults() {
  using namespace agiru::platform;
  const agiru::TableDef source{
      .id = agiru::TableId{kTemporaryId}, .name = "Ordinary AL defaults", .module = &kSourceModule};
  for (const auto kind : {agiru::TableType::Normal, agiru::TableType::Temporary}) {
    auto declaration = source;
    declaration.tableType = kind;
    const auto row = agiru::detail::ProjectTableMetadata(declaration);
    CHECK_TRUE("AL table classification defaults to compiler-emitted CustomerContent",
               row.DataClassification == FieldDataClassification::CustomerContent);
    CHECK_TRUE("absent table Access defaults to compiler-emitted Public",
               row.Access == TableMetadataAccess::Public);
    CHECK_TRUE("absent compression defaults to compiler-emitted Unspecified",
               row.CompressionType == TableMetadataCompressionType::Unspecified);
    CHECK_TRUE("absent obsoletion uses the documented No default",
               row.ObsoleteState == TableMetadataObsoleteState::No);
    CHECK_TRUE("omitted table Scope remains Cloud even in an OnPrem app",
               row.Scope == TableMetadataScope::Cloud);
    CHECK_TRUE("effective defaults do not overwrite immutable source omission",
               declaration.dataClassification.empty() && declaration.access.empty() &&
                   declaration.compressionType.empty() && declaration.obsoleteState.empty() &&
                   declaration.scope.empty());
    CHECK_TRUE("ordinary AL Boolean defaults survive the projection",
               row.DataPerCompany && row.ReplicateData && row.PasteIsValid && !row.LinkedObject);
    CHECK_TRUE("default projection allocates no record/session state",
               row.State_Block.Peek() == nullptr);
  }
}

void TableProjectionRefusals() {
  const auto refused = [](const agiru::TableDef &source, std::string_view reason) {
    try {
      static_cast<void>(agiru::detail::ProjectTableMetadata(source));
      return false;
    } catch (const agiru::Error &error) { return std::string_view(error.what()).contains(reason); }
  };
  auto source = kSourceTable;
  source.module = nullptr;
  CHECK_TRUE("missing module identity cannot become a null App ID", refused(source, "App ID"));
  for (const std::string_view id : {"invalid", "00000000-0000-0000-0000-000000000000"}) {
    const agiru::ModuleDef owner{
        .id = id, .name = "Source App", .publisher = "agiru tests", .version = "1.2.3.4"};
    source.module = &owner;
    CHECK_TRUE("invalid original owner identities refuse", refused(source, "App ID"));
  }
  for (const auto member : {&agiru::TableDef::compressionType,
                            &agiru::TableDef::obsoleteState,
                            &agiru::TableDef::dataClassification,
                            &agiru::TableDef::scope,
                            &agiru::TableDef::access}) {
    source = kSourceTable;
    source.*member = "Unverified";
    CHECK_TRUE("invalid explicit properties never fall back to ordinary AL defaults",
               refused(source, "Table Metadata."));
    source.id = agiru::platform::TableMetadata_Table::kId;
    source.*member = {};
    CHECK_TRUE("native property omission needs separate creation-metadata authority",
               refused(source, "Table Metadata."));
  }
  source = kSourceTable;
  source.tableType = agiru::TableType::CDS;
  CHECK_TRUE("CDS never becomes the native Query member", refused(source, "CDS"));
  constexpr std::array missingField{agiru::FieldNo{8}};
  source = kSourceTable;
  source.dataCaptionFields = missingField;
  CHECK_TRUE("caption fields must exist in the represented source",
             refused(source, "absent field: 8"));
}

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

template <typename Native, std::size_t N>
void CheckPropertyMembers(std::string_view property,
                          const std::array<std::pair<std::string_view, Native>, N> &members,
                          auto convert) {
  CHECK_TRUE("the entire native property vocabulary is covered: " + std::string(property),
             agiru::OptionTraits<Native>::kValues.size() == members.size());
  for (const auto &[name, value] : members) {
    const auto exact = convert(name);
    CHECK_TRUE("source property maps by identity: " + std::string(name), exact && *exact == value);
    std::string lower{name};
    for (char &c : lower) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
    const auto folded = convert(lower);
    CHECK_TRUE("AL property names are case-insensitive: " + std::string(name),
               folded && *folded == value);
  }
  for (const std::string_view unknown : {std::string_view{}, std::string_view{"Unverified"}}) {
    const auto refused = convert(unknown);
    CHECK_TRUE("unverified property authority is not replaced by a default",
               !refused && refused.error().contains(property) && refused.error().contains(unknown));
  }
}

void TableProperties() {
  using namespace agiru::platform;
  using namespace agiru::detail;
  CheckPropertyMembers(
      "ObsoleteState",
      std::array{std::pair{std::string_view{"No"}, TableMetadataObsoleteState::No},
                 std::pair{std::string_view{"Pending"}, TableMetadataObsoleteState::Pending},
                 std::pair{std::string_view{"Removed"}, TableMetadataObsoleteState::Removed}},
      MetadataObsoleteState);
  CheckPropertyMembers(
      "CompressionType",
      std::array{
          std::pair{std::string_view{"Unspecified"}, TableMetadataCompressionType::Unspecified},
          std::pair{std::string_view{"None"}, TableMetadataCompressionType::None},
          std::pair{std::string_view{"Row"}, TableMetadataCompressionType::Row},
          std::pair{std::string_view{"Page"}, TableMetadataCompressionType::Page}},
      MetadataCompressionType);
  CheckPropertyMembers(
      "Scope",
      std::array{std::pair{std::string_view{"Cloud"}, TableMetadataScope::Cloud},
                 std::pair{std::string_view{"OnPrem"}, TableMetadataScope::OnPrem}},
      MetadataScope);
  CheckPropertyMembers(
      "Access",
      std::array{std::pair{std::string_view{"Public"}, TableMetadataAccess::Public},
                 std::pair{std::string_view{"Internal"}, TableMetadataAccess::Internal}},
      MetadataAccess);
  CheckPropertyMembers(
      "DataClassification",
      std::array{
          std::pair{std::string_view{"CustomerContent"}, FieldDataClassification::CustomerContent},
          std::pair{std::string_view{"ToBeClassified"}, FieldDataClassification::ToBeClassified},
          std::pair{std::string_view{"EndUserIdentifiableInformation"},
                    FieldDataClassification::EndUserIdentifiableInformation},
          std::pair{std::string_view{"AccountData"}, FieldDataClassification::AccountData},
          std::pair{std::string_view{"EndUserPseudonymousIdentifiers"},
                    FieldDataClassification::EndUserPseudonymousIdentifiers},
          std::pair{std::string_view{"OrganizationIdentifiableInformation"},
                    FieldDataClassification::OrganizationIdentifiableInformation},
          std::pair{std::string_view{"SystemMetadata"}, FieldDataClassification::SystemMetadata}},
      MetadataDataClassification);
  for (const std::string_view state : {"Moved", "PendingMove"}) {
    const auto refused = MetadataObsoleteState(state);
    CHECK_TRUE("moved property states cannot become native No/Pending/Removed",
               !refused && refused.error().contains(state));
  }
  constexpr std::array aliases{
      std::pair{std::string_view{"Extension"}, TableMetadataScope::Cloud},
      std::pair{std::string_view{"Internal"}, TableMetadataScope::OnPrem},
      std::pair{std::string_view{"Personalization"}, TableMetadataScope::Cloud}};
  for (const auto &[scope, native] : aliases) {
    const auto mapped = MetadataScope(scope);
    CHECK_TRUE("documented legacy aliases normalize to current native scope members",
               mapped && *mapped == native);
    std::string lower{scope};
    for (char &c : lower) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
    const auto folded = MetadataScope(lower);
    CHECK_TRUE("legacy scope aliases retain AL case-insensitive matching",
               folded && *folded == native);
  }
  for (const std::string_view access : {"Local", "Protected"}) {
    const auto refused = MetadataAccess(access);
    CHECK_TRUE("field accessibility cannot become table accessibility",
               !refused && refused.error().contains(access));
  }
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
    TableProperties();
    TableProjection();
    TableProjectionVariants();
    TableProjectionDefaults();
    TableProjectionRefusals();
    MissingProviderIsNotAnEmptySnapshot<agiru::platform::PageMetadata>();
    MissingProviderIsNotAnEmptySnapshot<agiru::platform::TableMetadata>();
  });
}
