#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/ReflectionOptions.h"
#include "platform/ReflectionTypes.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
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

/// \brief System 28/29 `Table Metadata` declaration. Primary-key Get projects qualified installed
/// declarations through the shared runtime, also for RecordRef, without SQL snapshots. Other live
/// reads and writes remain refused; temporary records keep their independent writable storage.
class TableMetadata_Table : public Table<TableMetadata_Table> {
public:
  /// \brief Original System table identity.
  static constexpr TableId kId{2000000136};
  /// \brief Original AL object name.
  static constexpr std::string_view kName{"Table Metadata"};
  /// \brief Original extension availability, not a product integration requirement.
  static constexpr std::string_view kScope{"Cloud"};

  detail::StateHandle State_Block;

  /// \brief Original System Name field length.
  static constexpr std::size_t kNameLength = 30;
  /// \brief Original System Caption field length.
  static constexpr std::size_t kCaptionLength = 80;
  /// \brief Original System ExternalName field length.
  static constexpr std::size_t kExternalNameLength = 248;
  /// \brief Original System Reason field length.
  static constexpr std::size_t kReasonLength = 248;
  /// \brief Original System Permissions field length.
  static constexpr std::size_t kPermissionsLength = 5;
  /// \brief Original System Namespace field length.
  static constexpr std::size_t kNamespaceLength = 500;

  /// \brief AL `Table Metadata.ID`.
  ::agiru::Integer ID{};
  /// \brief AL `Table Metadata.Name`.
  Text<kNameLength> Name{};
  /// \brief AL `Table Metadata.Caption`.
  Text<kCaptionLength> Caption{};
  /// \brief AL `Table Metadata.DataPerCompany`.
  Boolean DataPerCompany{};
  /// \brief AL `Table Metadata.LookupPageID`.
  ::agiru::Integer LookupPageID{};
  /// \brief AL `Table Metadata.DrillDownPageId`.
  ::agiru::Integer DrillDownPageID{};
  /// \brief AL `Table Metadata.DataCaptionFields`.
  Text<kCaptionLength> DataCaptionFields{};
  /// \brief AL `Table Metadata.PasteIsValid`.
  Boolean PasteIsValid{};
  /// \brief AL `Table Metadata.LinkedObject`.
  Boolean LinkedObject{};
  /// \brief AL `Table Metadata.DataIsExternal`.
  Boolean DataIsExternal{};
  /// \brief AL `Table Metadata.TableType`.
  Option<TableMetadataTableType> TableType{};
  /// \brief AL `Table Metadata.ExternalName`.
  Text<kExternalNameLength> ExternalName{};
  /// \brief AL `Table Metadata.ObsoleteState`.
  Option<TableMetadataObsoleteState> ObsoleteState{};
  /// \brief AL `Table Metadata.ObsoleteReason`.
  Text<kReasonLength> ObsoleteReason{};
  /// \brief AL `Table Metadata.DataClassification`.
  Option<FieldDataClassification> DataClassification{};
  /// \brief AL `Table Metadata.ReplicateData`.
  Boolean ReplicateData{};
  /// \brief AL `Table Metadata.CompressionType`.
  Option<TableMetadataCompressionType> CompressionType{};
  /// \brief AL `Table Metadata.App ID`.
  Guid AppID{};
  /// \brief AL `Table Metadata.InherentPermissions`.
  Text<kPermissionsLength> InherentPermissions{};
  /// \brief AL `Table Metadata.InherentEntitlements`.
  Text<kPermissionsLength> InherentEntitlements{};
  /// \brief AL `Table Metadata.Scope`.
  Option<TableMetadataScope> Scope{};
  /// \brief AL `Table Metadata.Access`.
  Option<TableMetadataAccess> Access{};
  /// \brief AL `Table Metadata.AL Namespace`.
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
  /// \brief Implicit rowversion buffer; live-provider version ownership remains separate.
  BigInteger SystemRowVersion{};
  /// \brief Current creator User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemCreatedByUserName{};
  /// \brief Current creator full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemCreatedByFullName{};
  /// \brief Current modifier User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemModifiedByUserName{};
  /// \brief Current modifier full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemModifiedByFullName{};

  /// \brief Original source field numbers; implicit fields use the canonical platform numbers.
  struct Field_No : SystemFieldNumbers {
    /// \brief Original `ID` number.
    static constexpr ::agiru::FieldNo ID{1};
    /// \brief Original `Name` number.
    static constexpr ::agiru::FieldNo Name{2};
    /// \brief Original `Caption` number.
    static constexpr ::agiru::FieldNo Caption{3};
    /// \brief Original `DataPerCompany` number.
    static constexpr ::agiru::FieldNo DataPerCompany{4};
    /// \brief Original `LookupPageID` number.
    static constexpr ::agiru::FieldNo LookupPageID{5};
    /// \brief Original `DrillDownPageId` number.
    static constexpr ::agiru::FieldNo DrillDownPageID{6};
    /// \brief Original `DataCaptionFields` number.
    static constexpr ::agiru::FieldNo DataCaptionFields{7};
    /// \brief Original `PasteIsValid` number.
    static constexpr ::agiru::FieldNo PasteIsValid{8};
    /// \brief Original `LinkedObject` number.
    static constexpr ::agiru::FieldNo LinkedObject{9};
    /// \brief Original `DataIsExternal` number.
    static constexpr ::agiru::FieldNo DataIsExternal{10};
    /// \brief Original `TableType` number.
    static constexpr ::agiru::FieldNo TableType{11};
    /// \brief Original `ExternalName` number.
    static constexpr ::agiru::FieldNo ExternalName{12};
    /// \brief Original `ObsoleteState` number.
    static constexpr ::agiru::FieldNo ObsoleteState{13};
    /// \brief Original `ObsoleteReason` number.
    static constexpr ::agiru::FieldNo ObsoleteReason{14};
    /// \brief Original `DataClassification` number.
    static constexpr ::agiru::FieldNo DataClassification{15};
    /// \brief Original `ReplicateData` number.
    static constexpr ::agiru::FieldNo ReplicateData{16};
    /// \brief Original `CompressionType` number.
    static constexpr ::agiru::FieldNo CompressionType{17};
    /// \brief Original `App ID` number.
    static constexpr ::agiru::FieldNo AppID{18};
    /// \brief Original `InherentPermissions` number.
    static constexpr ::agiru::FieldNo InherentPermissions{19};
    /// \brief Original `InherentEntitlements` number.
    static constexpr ::agiru::FieldNo InherentEntitlements{20};
    /// \brief Original `Scope` number.
    static constexpr ::agiru::FieldNo Scope{21};
    /// \brief Original `Access` number.
    static constexpr ::agiru::FieldNo Access{22};
    /// \brief Original `AL Namespace` number.
    static constexpr ::agiru::FieldNo ALNamespace{23};
  };

  /// \brief Documented implicit primary key from the lowest declared field ID.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

/// \brief Native record binding for AL `Table Metadata`.
using TableMetadata = TableMetadata_Table;

/// \brief All source fields plus the canonical implicit system fields.
inline constexpr auto kTableMetadataFields = WithImplicitFields<TableMetadata,
                                                                SystemFieldProfile::Runtime18,
                                                                TableType::Normal,
                                                                false>(std::array<FieldDef, 23>{{
    Declare<&TableMetadata::ID>(
        TableMetadata::Field_No::ID, "ID", "ID", offsetof(TableMetadata, ID)),
    Declare<&TableMetadata::Name>(
        TableMetadata::Field_No::Name, "Name", "Name", offsetof(TableMetadata, Name)),
    Declare<&TableMetadata::Caption>(
        TableMetadata::Field_No::Caption, "Caption", "Caption", offsetof(TableMetadata, Caption)),
    Declare<&TableMetadata::DataPerCompany>(TableMetadata::Field_No::DataPerCompany,
                                            "DataPerCompany",
                                            "DataPerCompany",
                                            offsetof(TableMetadata, DataPerCompany)),
    Declare<&TableMetadata::LookupPageID>(TableMetadata::Field_No::LookupPageID,
                                          "LookupPageID",
                                          "LookupPageID",
                                          offsetof(TableMetadata, LookupPageID)),
    Declare<&TableMetadata::DrillDownPageID>(TableMetadata::Field_No::DrillDownPageID,
                                             "DrillDownPageId",
                                             "DrillDownPageId",
                                             offsetof(TableMetadata, DrillDownPageID)),
    Declare<&TableMetadata::DataCaptionFields>(TableMetadata::Field_No::DataCaptionFields,
                                               "DataCaptionFields",
                                               "DataCaptionFields",
                                               offsetof(TableMetadata, DataCaptionFields)),
    Declare<&TableMetadata::PasteIsValid>(TableMetadata::Field_No::PasteIsValid,
                                          "PasteIsValid",
                                          "PasteIsValid",
                                          offsetof(TableMetadata, PasteIsValid)),
    Declare<&TableMetadata::LinkedObject>(TableMetadata::Field_No::LinkedObject,
                                          "LinkedObject",
                                          "LinkedObject",
                                          offsetof(TableMetadata, LinkedObject)),
    Declare<&TableMetadata::DataIsExternal>(TableMetadata::Field_No::DataIsExternal,
                                            "DataIsExternal",
                                            "DataIsExternal",
                                            offsetof(TableMetadata, DataIsExternal)),
    Declare<&TableMetadata::TableType>(TableMetadata::Field_No::TableType,
                                       "TableType",
                                       "TableType",
                                       offsetof(TableMetadata, TableType)),
    Declare<&TableMetadata::ExternalName>(TableMetadata::Field_No::ExternalName,
                                          "ExternalName",
                                          "ExternalName",
                                          offsetof(TableMetadata, ExternalName)),
    Declare<&TableMetadata::ObsoleteState>(TableMetadata::Field_No::ObsoleteState,
                                           "ObsoleteState",
                                           "ObsoleteState",
                                           offsetof(TableMetadata, ObsoleteState)),
    Declare<&TableMetadata::ObsoleteReason>(TableMetadata::Field_No::ObsoleteReason,
                                            "ObsoleteReason",
                                            "ObsoleteReason",
                                            offsetof(TableMetadata, ObsoleteReason)),
    Declare<&TableMetadata::DataClassification>(TableMetadata::Field_No::DataClassification,
                                                "DataClassification",
                                                "DataClassification",
                                                offsetof(TableMetadata, DataClassification)),
    Declare<&TableMetadata::ReplicateData>(TableMetadata::Field_No::ReplicateData,
                                           "ReplicateData",
                                           "ReplicateData",
                                           offsetof(TableMetadata, ReplicateData)),
    Declare<&TableMetadata::CompressionType>(TableMetadata::Field_No::CompressionType,
                                             "CompressionType",
                                             "CompressionType",
                                             offsetof(TableMetadata, CompressionType)),
    Declare<&TableMetadata::AppID>(
        TableMetadata::Field_No::AppID, "App ID", "App ID", offsetof(TableMetadata, AppID)),
    Declare<&TableMetadata::InherentPermissions>(TableMetadata::Field_No::InherentPermissions,
                                                 "InherentPermissions",
                                                 "InherentPermissions",
                                                 offsetof(TableMetadata, InherentPermissions)),
    Declare<&TableMetadata::InherentEntitlements>(TableMetadata::Field_No::InherentEntitlements,
                                                  "InherentEntitlements",
                                                  "InherentEntitlements",
                                                  offsetof(TableMetadata, InherentEntitlements)),
    Declare<&TableMetadata::Scope>(
        TableMetadata::Field_No::Scope, "Scope", "Scope", offsetof(TableMetadata, Scope)),
    Declare<&TableMetadata::Access>(
        TableMetadata::Field_No::Access, "Access", "Access", offsetof(TableMetadata, Access)),
    Declare<&TableMetadata::ALNamespace>(TableMetadata::Field_No::ALNamespace,
                                         "AL Namespace",
                                         "AL Namespace",
                                         offsetof(TableMetadata, ALNamespace)),
}});

/// \brief Effective primary key; its case-sensitive AL name is part of the contract.
inline constexpr std::array<KeyDef, 1> kTableMetadataKeys{{
    KeyDef{.name = "ID", .fields = TableMetadata::kKey1, .clustered = true},
}};

/// \brief Original source declaration, not a physical virtual-table provider guarantee.
inline constexpr TableDef kTableMetadataTable{
    .id = TableMetadata::kId,
    .name = TableMetadata::kName,
    .caption = TableMetadata::kName,
    .fields = kTableMetadataFields,
    .keys = kTableMetadataKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
    .providerRefusal =
        "live table metadata projection and schema identity are unavailable (board:0034/0044/0013)",
};

static_assert(FieldsAreSorted(kTableMetadataTable),
              "reflection fields are searched by original number");

}

/// \brief Native declaration ownership for AL reflection and temporary records.
template <> struct agiru::TableTraits<agiru::platform::TableMetadata> {
  /// \brief Immutable original-source table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kTableMetadataTable;
};
