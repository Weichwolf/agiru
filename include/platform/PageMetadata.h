#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/ReflectionTypes.h"
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
#include <string_view>

namespace agiru::platform {

/// \brief System 28/29 `Page Metadata` declaration; live-provider/provenance acceptance is
/// separate.
class PageMetadata_Table : public Table<PageMetadata_Table> {
public:
  /// \brief Original System table identity.
  static constexpr TableId kId{2000000138};
  /// \brief Original AL object name.
  static constexpr std::string_view kName{"Page Metadata"};
  /// \brief Original extension availability, not a product integration requirement.
  static constexpr std::string_view kScope{"Cloud"};

  detail::StateHandle State_Block;

  /// \brief Original System Name field length.
  static constexpr std::size_t kNameLength = 30;
  /// \brief Original System Caption field length.
  static constexpr std::size_t kCaptionLength = 80;
  /// \brief Original System Expression field length.
  static constexpr std::size_t kExpressionLength = 250;
  /// \brief Original System ApiName field length.
  static constexpr std::size_t kApiNameLength = 40;
  /// \brief Original System Permissions field length.
  static constexpr std::size_t kPermissionsLength = 5;
  /// \brief Original System Namespace field length.
  static constexpr std::size_t kNamespaceLength = 500;

  /// \brief AL `Page Metadata.ID`.
  ::agiru::Integer ID{};
  /// \brief AL `Page Metadata.Name`.
  Text<kNameLength> Name{};
  /// \brief AL `Page Metadata.Caption`.
  Text<kCaptionLength> Caption{};
  /// \brief AL `Page Metadata.Editable`.
  Boolean Editable{};
  /// \brief AL `Page Metadata.PageType`.
  Option<PageMetadataPageType> PageType{};
  /// \brief AL `Page Metadata.CardPageID`.
  ::agiru::Integer CardPageID{};
  /// \brief AL `Page Metadata.DataCaptionExpr.`.
  Text<kExpressionLength> DataCaptionExpr{};
  /// \brief AL `Page Metadata.RefreshOnActivate`.
  Boolean RefreshOnActivate{};
  /// \brief AL `Page Metadata.APIPublisher`.
  Text<kApiNameLength> APIPublisher{};
  /// \brief AL `Page Metadata.APIGroup`.
  Text<kApiNameLength> APIGroup{};
  /// \brief AL `Page Metadata.APIVersion`.
  Text<kExpressionLength> APIVersion{};
  /// \brief AL `Page Metadata.EntitySetName`.
  Text<kExpressionLength> EntitySetName{};
  /// \brief AL `Page Metadata.EntityName`.
  Text<kExpressionLength> EntityName{};
  /// \brief AL `Page Metadata.SourceTable`.
  ::agiru::Integer SourceTable{};
  /// \brief AL `Page Metadata.SourceTableView`.
  Text<kExpressionLength> SourceTableView{};
  /// \brief AL `Page Metadata.InsertAllowed`.
  Boolean InsertAllowed{};
  /// \brief AL `Page Metadata.ModifyAllowed`.
  Boolean ModifyAllowed{};
  /// \brief AL `Page Metadata.DeleteAllowed`.
  Boolean DeleteAllowed{};
  /// \brief AL `Page Metadata.DelayedInsert`.
  Boolean DelayedInsert{};
  /// \brief AL `Page Metadata.ShowFilter`.
  Boolean ShowFilter{};
  /// \brief AL `Page Metadata.MultipleNewLines`.
  Boolean MultipleNewLines{};
  /// \brief AL `Page Metadata.SaveValues`.
  Boolean SaveValues{};
  /// \brief AL `Page Metadata.AutoSplitKey`.
  Boolean AutoSplitKey{};
  /// \brief AL `Page Metadata.DataCaptionFields`.
  Text<kExpressionLength> DataCaptionFields{};
  /// \brief AL `Page Metadata.SourceTableTemporary`.
  Boolean SourceTableTemporary{};
  /// \brief AL `Page Metadata.LinksAllowed`.
  Boolean LinksAllowed{};
  /// \brief AL `Page Metadata.ChangeTrackingAllowed`.
  Boolean ChangeTrackingAllowed{};
  /// \brief AL `Page Metadata.PopulateAllFields`.
  Boolean PopulateAllFields{};
  /// \brief AL `Page Metadata.App ID`.
  Guid AppID{};
  /// \brief AL `Page Metadata.InherentPermissions`.
  Text<kPermissionsLength> InherentPermissions{};
  /// \brief AL `Page Metadata.InherentEntitlements`.
  Text<kPermissionsLength> InherentEntitlements{};
  /// \brief AL `Page Metadata.AL Namespace`.
  Text<kNamespaceLength> ALNamespace{};
  /// \brief Implicit AL `SystemId`.
  Guid SystemId{};
  /// \brief Implicit AL `SystemCreatedAt`.
  DateTime SystemCreatedAt{};
  /// \brief Implicit AL `SystemCreatedBy`.
  Guid SystemCreatedBy{};
  /// \brief Implicit AL `SystemModifiedAt`.
  DateTime SystemModifiedAt{};
  /// \brief Implicit AL `SystemModifiedBy`.
  Guid SystemModifiedBy{};

  /// \brief Original source field numbers; implicit fields use the canonical platform numbers.
  struct Field_No : SystemFieldNumbers {
    /// \brief Original `ID` number.
    static constexpr ::agiru::FieldNo ID{1};
    /// \brief Original `Name` number.
    static constexpr ::agiru::FieldNo Name{2};
    /// \brief Original `Caption` number.
    static constexpr ::agiru::FieldNo Caption{3};
    /// \brief Original `Editable` number.
    static constexpr ::agiru::FieldNo Editable{4};
    /// \brief Original `PageType` number.
    static constexpr ::agiru::FieldNo PageType{5};
    /// \brief Original `CardPageID` number.
    static constexpr ::agiru::FieldNo CardPageID{6};
    /// \brief Original `DataCaptionExpr.` number.
    static constexpr ::agiru::FieldNo DataCaptionExpr{7};
    /// \brief Original `RefreshOnActivate` number.
    static constexpr ::agiru::FieldNo RefreshOnActivate{8};
    /// \brief Original `APIPublisher` number.
    static constexpr ::agiru::FieldNo APIPublisher{9};
    /// \brief Original `APIGroup` number.
    static constexpr ::agiru::FieldNo APIGroup{10};
    /// \brief Original `APIVersion` number.
    static constexpr ::agiru::FieldNo APIVersion{11};
    /// \brief Original `EntitySetName` number.
    static constexpr ::agiru::FieldNo EntitySetName{12};
    /// \brief Original `EntityName` number.
    static constexpr ::agiru::FieldNo EntityName{13};
    /// \brief Original `SourceTable` number.
    static constexpr ::agiru::FieldNo SourceTable{14};
    /// \brief Original `SourceTableView` number.
    static constexpr ::agiru::FieldNo SourceTableView{15};
    /// \brief Original `InsertAllowed` number.
    static constexpr ::agiru::FieldNo InsertAllowed{16};
    /// \brief Original `ModifyAllowed` number.
    static constexpr ::agiru::FieldNo ModifyAllowed{17};
    /// \brief Original `DeleteAllowed` number.
    static constexpr ::agiru::FieldNo DeleteAllowed{18};
    /// \brief Original `DelayedInsert` number.
    static constexpr ::agiru::FieldNo DelayedInsert{19};
    /// \brief Original `ShowFilter` number.
    static constexpr ::agiru::FieldNo ShowFilter{20};
    /// \brief Original `MultipleNewLines` number.
    static constexpr ::agiru::FieldNo MultipleNewLines{21};
    /// \brief Original `SaveValues` number.
    static constexpr ::agiru::FieldNo SaveValues{22};
    /// \brief Original `AutoSplitKey` number.
    static constexpr ::agiru::FieldNo AutoSplitKey{23};
    /// \brief Original `DataCaptionFields` number.
    static constexpr ::agiru::FieldNo DataCaptionFields{24};
    /// \brief Original `SourceTableTemporary` number.
    static constexpr ::agiru::FieldNo SourceTableTemporary{25};
    /// \brief Original `LinksAllowed` number.
    static constexpr ::agiru::FieldNo LinksAllowed{26};
    /// \brief Original `ChangeTrackingAllowed` number.
    static constexpr ::agiru::FieldNo ChangeTrackingAllowed{27};
    /// \brief Original `PopulateAllFields` number.
    static constexpr ::agiru::FieldNo PopulateAllFields{28};
    /// \brief Original `App ID` number.
    static constexpr ::agiru::FieldNo AppID{29};
    /// \brief Original `InherentPermissions` number.
    static constexpr ::agiru::FieldNo InherentPermissions{30};
    /// \brief Original `InherentEntitlements` number.
    static constexpr ::agiru::FieldNo InherentEntitlements{31};
    /// \brief Original `AL Namespace` number.
    static constexpr ::agiru::FieldNo ALNamespace{32};
  };

  /// \brief Original source primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

/// \brief Native record binding for AL `Page Metadata`.
using PageMetadata = PageMetadata_Table;

/// \brief All source fields plus the canonical implicit system fields.
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

/// \brief Effective primary key; its case-sensitive AL name is part of the contract.
inline constexpr std::array<KeyDef, 1> kPageMetadataKeys{{
    KeyDef{.name = "pk", .fields = PageMetadata::kKey1, .clustered = true},
}};

/// \brief Original source declaration, not a physical virtual-table provider guarantee.
inline constexpr TableDef kPageMetadataTable{
    .id = PageMetadata::kId,
    .name = PageMetadata::kName,
    .caption = PageMetadata::kName,
    .fields = kPageMetadataFields,
    .keys = kPageMetadataKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
    .providerRefusal =
        "live page metadata projection and schema identity are unavailable (board:0034/0044/0013)",
};

static_assert(FieldsAreSorted(kPageMetadataTable),
              "reflection fields are searched by original number");

}

/// \brief Native declaration ownership for AL reflection and temporary records.
template <> struct agiru::TableTraits<agiru::platform::PageMetadata> {
  /// \brief Immutable original-source table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPageMetadataTable;
};
