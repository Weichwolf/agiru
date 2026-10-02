#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace agiru::platform {

/// \brief Original System Page Metadata.PageType members, not internal PageType ordinals.
enum class PageMetadataPageType : std::int32_t {
  Card = 0,
  List = 1,
  RoleCenter = 2,
  CardPart = 3,
  ListPart = 4,
  Document = 5,
  Worksheet = 6,
  ListPlus = 7,
  ConfirmationDialog = 8,
  NavigatePage = 9,
  StandardDialog = 10,
  Api = 11,
  HeadlinePart = 12,
};

/// \brief Projects only member identities declared by the pinned System Page Metadata table.
/// \param type The internal page property, not an AL option ordinal.
/// \return The source member, or no value for types absent from that declaration.
[[nodiscard]] constexpr std::optional<PageMetadataPageType> PageMetadataTypeOf(PageType type) {
  switch (type) {
    case PageType::Card: return PageMetadataPageType::Card;
    case PageType::List: return PageMetadataPageType::List;
    case PageType::RoleCenter: return PageMetadataPageType::RoleCenter;
    case PageType::CardPart: return PageMetadataPageType::CardPart;
    case PageType::ListPart: return PageMetadataPageType::ListPart;
    case PageType::Document: return PageMetadataPageType::Document;
    case PageType::Worksheet: return PageMetadataPageType::Worksheet;
    case PageType::ListPlus: return PageMetadataPageType::ListPlus;
    case PageType::ConfirmationDialog: return PageMetadataPageType::ConfirmationDialog;
    case PageType::NavigatePage: return PageMetadataPageType::NavigatePage;
    case PageType::StandardDialog: return PageMetadataPageType::StandardDialog;
    case PageType::Api: return PageMetadataPageType::Api;
    case PageType::HeadlinePart: return PageMetadataPageType::HeadlinePart;
    case PageType::ReportPreview:
    case PageType::ReportProcessingOnly:
    case PageType::XmlPort:
    case PageType::PromptDialog:
    case PageType::ConfigurationDialog:
    case PageType::UserControlHost: return std::nullopt;
  }
  return std::nullopt;
}

}

/// \brief Original System names, captions and thirteen ordinary option positions.
template <> struct agiru::OptionTraits<agiru::platform::PageMetadataPageType> {
  /// \brief Source vocabulary; PageType property values do not extend it.
  static constexpr std::array<agiru::EnumValueDef, 13> kValues{{
      {.ordinal = 0, .name = "Card", .caption = "Card"},
      {.ordinal = 1, .name = "List", .caption = "List"},
      {.ordinal = 2, .name = "RoleCenter", .caption = "RoleCenter"},
      {.ordinal = 3, .name = "CardPart", .caption = "CardPart"},
      {.ordinal = 4, .name = "ListPart", .caption = "ListPart"},
      {.ordinal = 5, .name = "Document", .caption = "Document"},
      {.ordinal = 6, .name = "Worksheet", .caption = "Worksheet"},
      {.ordinal = 7, .name = "ListPlus", .caption = "ListPlus"},
      {.ordinal = 8, .name = "ConfirmationDialog", .caption = "ConfirmationDialog"},
      {.ordinal = 9, .name = "NavigatePage", .caption = "NavigatePage"},
      {.ordinal = 10, .name = "StandardDialog", .caption = "StandardDialog"},
      {.ordinal = 11, .name = "API", .caption = "API"},
      {.ordinal = 12, .name = "HeadlinePart", .caption = "HeadlinePart"},
  }};
};

namespace agiru::platform {

/// \brief Source declaration of System.Reflection.Page Metadata (2000000138).
/// \note System.app 28.0.53152.0, Virtual Tables/PageMetadata.Table.al. Declaration fidelity
///       does not implement a live read-only provider, app identity or permissions.
///       Common Scope/fieldgroup metadata remains board:0034; rowversion remains board:0013.
class PageMetadata_Table : public Table<PageMetadata_Table> {
public:
  static constexpr TableId kId{2000000138};                 ///< Original System table ID.
  static constexpr std::string_view kName{"Page Metadata"}; ///< Original AL name.
  detail::StateHandle State_Block; ///< Record-variable state at required first-member offset.
  static constexpr std::size_t kNameLength = 30;        ///< Source Name length.
  static constexpr std::size_t kCaptionLength = 80;     ///< Source Caption length.
  static constexpr std::size_t kExpressionLength = 250; ///< Source expression/API-version lengths.
  static constexpr std::size_t kApiIdentityLength = 40; ///< Source publisher/group lengths.
  static constexpr std::size_t kPermissionsLength = 5;  ///< Source permission/entitlement lengths.
  static constexpr std::size_t kNamespaceLength = 500;  ///< Source AL Namespace length.

  ::agiru::Integer ID{};                         ///< AL ID.
  Text<kNameLength> Name;                        ///< AL Name, not Caption.
  Text<kCaptionLength> Caption;                  ///< AL Caption.
  Boolean Editable{};                            ///< AL Editable.
  Option<PageMetadataPageType> PageType;         ///< AL PageType.
  ::agiru::Integer CardPageID{};                 ///< AL CardPageID.
  Text<kExpressionLength> DataCaptionExpr;       ///< AL DataCaptionExpr.
  Boolean RefreshOnActivate{};                   ///< AL RefreshOnActivate.
  Text<kApiIdentityLength> APIPublisher;         ///< AL APIPublisher.
  Text<kApiIdentityLength> APIGroup;             ///< AL APIGroup.
  Text<kExpressionLength> APIVersion;            ///< AL APIVersion.
  Text<kExpressionLength> EntitySetName;         ///< AL EntitySetName.
  Text<kExpressionLength> EntityName;            ///< AL EntityName.
  ::agiru::Integer SourceTable{};                ///< AL SourceTable.
  Text<kExpressionLength> SourceTableView;       ///< AL SourceTableView.
  Boolean InsertAllowed{};                       ///< AL InsertAllowed.
  Boolean ModifyAllowed{};                       ///< AL ModifyAllowed.
  Boolean DeleteAllowed{};                       ///< AL DeleteAllowed.
  Boolean DelayedInsert{};                       ///< AL DelayedInsert.
  Boolean ShowFilter{};                          ///< AL ShowFilter.
  Boolean MultipleNewLines{};                    ///< AL MultipleNewLines.
  Boolean SaveValues{};                          ///< AL SaveValues.
  Boolean AutoSplitKey{};                        ///< AL AutoSplitKey.
  Text<kExpressionLength> DataCaptionFields;     ///< AL DataCaptionFields.
  Boolean SourceTableTemporary{};                ///< AL SourceTableTemporary.
  Boolean LinksAllowed{};                        ///< AL LinksAllowed.
  Boolean ChangeTrackingAllowed{};               ///< AL ChangeTrackingAllowed.
  Boolean PopulateAllFields{};                   ///< AL PopulateAllFields.
  Guid AppID;                                    ///< AL App ID.
  Text<kPermissionsLength> InherentPermissions;  ///< AL InherentPermissions.
  Text<kPermissionsLength> InherentEntitlements; ///< AL InherentEntitlements.
  Text<kNamespaceLength> ALNamespace;            ///< AL AL Namespace.
  /// \brief AL `PageMetadata.SystemId`.
  Guid SystemId;
  /// \brief AL `PageMetadata.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `PageMetadata.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `PageMetadata.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `PageMetadata.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief Original AL field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo Name{2};
    static constexpr ::agiru::FieldNo Caption{3};
    static constexpr ::agiru::FieldNo Editable{4};
    static constexpr ::agiru::FieldNo PageType{5};
    static constexpr ::agiru::FieldNo CardPageID{6};
    static constexpr ::agiru::FieldNo DataCaptionExpr{7};
    static constexpr ::agiru::FieldNo RefreshOnActivate{8};
    static constexpr ::agiru::FieldNo APIPublisher{9};
    static constexpr ::agiru::FieldNo APIGroup{10};
    static constexpr ::agiru::FieldNo APIVersion{11};
    static constexpr ::agiru::FieldNo EntitySetName{12};
    static constexpr ::agiru::FieldNo EntityName{13};
    static constexpr ::agiru::FieldNo SourceTable{14};
    static constexpr ::agiru::FieldNo SourceTableView{15};
    static constexpr ::agiru::FieldNo InsertAllowed{16};
    static constexpr ::agiru::FieldNo ModifyAllowed{17};
    static constexpr ::agiru::FieldNo DeleteAllowed{18};
    static constexpr ::agiru::FieldNo DelayedInsert{19};
    static constexpr ::agiru::FieldNo ShowFilter{20};
    static constexpr ::agiru::FieldNo MultipleNewLines{21};
    static constexpr ::agiru::FieldNo SaveValues{22};
    static constexpr ::agiru::FieldNo AutoSplitKey{23};
    static constexpr ::agiru::FieldNo DataCaptionFields{24};
    static constexpr ::agiru::FieldNo SourceTableTemporary{25};
    static constexpr ::agiru::FieldNo LinksAllowed{26};
    static constexpr ::agiru::FieldNo ChangeTrackingAllowed{27};
    static constexpr ::agiru::FieldNo PopulateAllFields{28};
    static constexpr ::agiru::FieldNo AppID{29};
    static constexpr ::agiru::FieldNo InherentPermissions{30};
    static constexpr ::agiru::FieldNo InherentEntitlements{31};
    static constexpr ::agiru::FieldNo ALNamespace{32};
  };

  /// \brief Original primary key on ID.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

/// \brief The original AL name's native binding.
using PageMetadata = PageMetadata_Table;

/// \brief All thirty-two source fields and the five common implicit system fields.
inline constexpr auto kPageMetadataFields = WithSystemFields<PageMetadata>(std::array<FieldDef,
                                                                                      32>{{
    Declare<&PageMetadata::ID>(PageMetadata::Field_No::ID, "ID", "ID", offsetof(PageMetadata, ID)),
    Declare<&PageMetadata::Name>(
        PageMetadata::Field_No::Name, "Name", "Name", offsetof(PageMetadata, Name)),
    Declare<&PageMetadata::Caption>(
        PageMetadata::Field_No::Caption, "Caption", "Caption", offsetof(PageMetadata, Caption)),
    Declare<&PageMetadata::Editable>(
        PageMetadata::Field_No::Editable, "Editable", "Editable", offsetof(PageMetadata, Editable)),
    Declare<&PageMetadata::PageType>(
        PageMetadata::Field_No::PageType, "PageType", "PageType", offsetof(PageMetadata, PageType)),
    Declare<&PageMetadata::CardPageID>(PageMetadata::Field_No::CardPageID,
                                       "CardPageID",
                                       "CardPageID",
                                       offsetof(PageMetadata, CardPageID)),
    Declare<&PageMetadata::DataCaptionExpr>(PageMetadata::Field_No::DataCaptionExpr,
                                            "DataCaptionExpr.",
                                            "DataCaptionExpr.",
                                            offsetof(PageMetadata, DataCaptionExpr)),
    Declare<&PageMetadata::RefreshOnActivate>(PageMetadata::Field_No::RefreshOnActivate,
                                              "RefreshOnActivate",
                                              "RefreshOnActivate",
                                              offsetof(PageMetadata, RefreshOnActivate)),
    Declare<&PageMetadata::APIPublisher>(PageMetadata::Field_No::APIPublisher,
                                         "APIPublisher",
                                         "APIPublisher",
                                         offsetof(PageMetadata, APIPublisher)),
    Declare<&PageMetadata::APIGroup>(
        PageMetadata::Field_No::APIGroup, "APIGroup", "APIGroup", offsetof(PageMetadata, APIGroup)),
    Declare<&PageMetadata::APIVersion>(PageMetadata::Field_No::APIVersion,
                                       "APIVersion",
                                       "APIVersion",
                                       offsetof(PageMetadata, APIVersion)),
    Declare<&PageMetadata::EntitySetName>(PageMetadata::Field_No::EntitySetName,
                                          "EntitySetName",
                                          "EntitySetName",
                                          offsetof(PageMetadata, EntitySetName)),
    Declare<&PageMetadata::EntityName>(PageMetadata::Field_No::EntityName,
                                       "EntityName",
                                       "EntityName",
                                       offsetof(PageMetadata, EntityName)),
    Declare<&PageMetadata::SourceTable>(PageMetadata::Field_No::SourceTable,
                                        "SourceTable",
                                        "SourceTable",
                                        offsetof(PageMetadata, SourceTable)),
    Declare<&PageMetadata::SourceTableView>(PageMetadata::Field_No::SourceTableView,
                                            "SourceTableView",
                                            "SourceTableView",
                                            offsetof(PageMetadata, SourceTableView)),
    Declare<&PageMetadata::InsertAllowed>(PageMetadata::Field_No::InsertAllowed,
                                          "InsertAllowed",
                                          "InsertAllowed",
                                          offsetof(PageMetadata, InsertAllowed)),
    Declare<&PageMetadata::ModifyAllowed>(PageMetadata::Field_No::ModifyAllowed,
                                          "ModifyAllowed",
                                          "ModifyAllowed",
                                          offsetof(PageMetadata, ModifyAllowed)),
    Declare<&PageMetadata::DeleteAllowed>(PageMetadata::Field_No::DeleteAllowed,
                                          "DeleteAllowed",
                                          "DeleteAllowed",
                                          offsetof(PageMetadata, DeleteAllowed)),
    Declare<&PageMetadata::DelayedInsert>(PageMetadata::Field_No::DelayedInsert,
                                          "DelayedInsert",
                                          "DelayedInsert",
                                          offsetof(PageMetadata, DelayedInsert)),
    Declare<&PageMetadata::ShowFilter>(PageMetadata::Field_No::ShowFilter,
                                       "ShowFilter",
                                       "ShowFilter",
                                       offsetof(PageMetadata, ShowFilter)),
    Declare<&PageMetadata::MultipleNewLines>(PageMetadata::Field_No::MultipleNewLines,
                                             "MultipleNewLines",
                                             "MultipleNewLines",
                                             offsetof(PageMetadata, MultipleNewLines)),
    Declare<&PageMetadata::SaveValues>(PageMetadata::Field_No::SaveValues,
                                       "SaveValues",
                                       "SaveValues",
                                       offsetof(PageMetadata, SaveValues)),
    Declare<&PageMetadata::AutoSplitKey>(PageMetadata::Field_No::AutoSplitKey,
                                         "AutoSplitKey",
                                         "AutoSplitKey",
                                         offsetof(PageMetadata, AutoSplitKey)),
    Declare<&PageMetadata::DataCaptionFields>(PageMetadata::Field_No::DataCaptionFields,
                                              "DataCaptionFields",
                                              "DataCaptionFields",
                                              offsetof(PageMetadata, DataCaptionFields)),
    Declare<&PageMetadata::SourceTableTemporary>(PageMetadata::Field_No::SourceTableTemporary,
                                                 "SourceTableTemporary",
                                                 "SourceTableTemporary",
                                                 offsetof(PageMetadata, SourceTableTemporary)),
    Declare<&PageMetadata::LinksAllowed>(PageMetadata::Field_No::LinksAllowed,
                                         "LinksAllowed",
                                         "LinksAllowed",
                                         offsetof(PageMetadata, LinksAllowed)),
    Declare<&PageMetadata::ChangeTrackingAllowed>(PageMetadata::Field_No::ChangeTrackingAllowed,
                                                  "ChangeTrackingAllowed",
                                                  "ChangeTrackingAllowed",
                                                  offsetof(PageMetadata, ChangeTrackingAllowed)),
    Declare<&PageMetadata::PopulateAllFields>(PageMetadata::Field_No::PopulateAllFields,
                                              "PopulateAllFields",
                                              "PopulateAllFields",
                                              offsetof(PageMetadata, PopulateAllFields)),
    Declare<&PageMetadata::AppID>(
        PageMetadata::Field_No::AppID, "App ID", "App ID", offsetof(PageMetadata, AppID)),
    Declare<&PageMetadata::InherentPermissions>(PageMetadata::Field_No::InherentPermissions,
                                                "InherentPermissions",
                                                "InherentPermissions",
                                                offsetof(PageMetadata, InherentPermissions)),
    Declare<&PageMetadata::InherentEntitlements>(PageMetadata::Field_No::InherentEntitlements,
                                                 "InherentEntitlements",
                                                 "InherentEntitlements",
                                                 offsetof(PageMetadata, InherentEntitlements)),
    Declare<&PageMetadata::ALNamespace>(PageMetadata::Field_No::ALNamespace,
                                        "AL Namespace",
                                        "AL Namespace",
                                        offsetof(PageMetadata, ALNamespace)),
}});

/// \brief Original primary key declaration; the primary key is effectively clustered.
inline constexpr std::array<KeyDef, 1> kPageMetadataKeys{{
    KeyDef{.name = "pk", .fields = PageMetadata::kKey1, .clustered = true},
}};

/// \brief Source-backed shared table declaration and inherent read/execute permission.
inline constexpr TableDef kPageMetadataTable{
    .id = PageMetadata::kId,
    .name = PageMetadata::kName,
    .caption = PageMetadata::kName,
    .fields = kPageMetadataFields,
    .keys = kPageMetadataKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
};

static_assert(FieldsAreSorted(kPageMetadataTable), "the field table is searched by number");
static_assert(offsetof(PageMetadata, State_Block) == 0, "the state is the first member");

}

/// \brief The source declaration used by the common table runtime.
template <> struct agiru::TableTraits<agiru::platform::PageMetadata> {
  /// \brief Immutable Page Metadata declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPageMetadataTable;
};
