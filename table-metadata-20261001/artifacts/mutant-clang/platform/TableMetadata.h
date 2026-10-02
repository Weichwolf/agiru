#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/ReflectionOptions.h"
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

/// \brief Original System Table Metadata.TableType option, not the TableType property.
enum class TableMetadataTableType : std::int32_t {
  Normal = 0,         ///< Ordinary table.
  CRM = 1,            ///< CRM integration table.
  ExternalSQL = 2,    ///< External SQL table.
  Exchange = 3,       ///< Exchange table.
  MicrosoftGraph = 4, ///< Microsoft Graph table.
  Query = 5,          ///< Source-declared Query member; not CDS.
  Temporary = 6,      ///< Temporary table.
};

/// \brief Table Metadata shares the original Field obsolete-state vocabulary.
using TableMetadataObsoleteState = ObsoleteState;

/// \brief Original System Table Metadata.CompressionType members.
enum class TableMetadataCompressionType : std::int32_t {
  Unspecified = 0, ///< Externally selected compression.
  None = 1,        ///< No compression.
  Row = 2,         ///< Row compression.
  Page = 3,        ///< Page compression.
};

/// \brief Original System Table Metadata.Scope members, distinct from profile scope.
enum class TableMetadataScope : std::int32_t {
  Cloud = 0,  ///< Available to cloud/on-premises targets.
  OnPrem = 1, ///< Available only to on-premises targets.
};

/// \brief Original System Table Metadata.Access members.
enum class TableMetadataAccess : std::int32_t {
  Public = 0,   ///< Public declaration.
  Internal = 1, ///< App-internal declaration; not a runtime security boundary.
};

/// \brief Projects only source-backed TableType property/member pairs.
/// \param type The internal table property, not an option ordinal.
/// \return The corresponding source member, or no value for an unproved CDS/unknown mapping.
[[nodiscard]] constexpr std::optional<TableMetadataTableType> TableMetadataTypeOf(TableType type) {
  switch (type) {
    case TableType::Normal: return TableMetadataTableType::Normal;
    case TableType::CRM: return TableMetadataTableType::CRM;
    case TableType::ExternalSQL: return TableMetadataTableType::ExternalSQL;
    case TableType::Exchange: return TableMetadataTableType::Exchange;
    case TableType::MicrosoftGraph: return TableMetadataTableType::MicrosoftGraph;
    case TableType::Temporary: return TableMetadataTableType::Temporary;
    case TableType::CDS: return TableMetadataTableType::Query;
  }
  return std::nullopt;
}

}

/// \brief Source TableType names, captions and ordinary option positions.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataTableType> {
  /// \brief All seven declared members; Query occupies position five.
  static constexpr std::array<agiru::EnumValueDef, 7> kValues{{
      {.ordinal = 0, .name = "Normal", .caption = "Normal"},
      {.ordinal = 1, .name = "CRM", .caption = "CRM"},
      {.ordinal = 2, .name = "ExternalSQL", .caption = "ExternalSQL"},
      {.ordinal = 3, .name = "Exchange", .caption = "Exchange"},
      {.ordinal = 4, .name = "MicrosoftGraph", .caption = "MicrosoftGraph"},
      {.ordinal = 5, .name = "Query", .caption = "Query"},
      {.ordinal = 6, .name = "Temporary", .caption = "Temporary"},
  }};
};

/// \brief Source compression names, captions and ordinary option positions.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataCompressionType> {
  /// \brief All four original members.
  static constexpr std::array<agiru::EnumValueDef, 4> kValues{{
      {.ordinal = 0, .name = "Unspecified", .caption = "Unspecified"},
      {.ordinal = 1, .name = "None", .caption = "None"},
      {.ordinal = 2, .name = "Row", .caption = "Row"},
      {.ordinal = 3, .name = "Page", .caption = "Page"},
  }};
};

/// \brief Source deployment-scope names, captions and ordinary option positions.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataScope> {
  /// \brief Both original members.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Cloud", .caption = "Cloud"},
      {.ordinal = 1, .name = "OnPrem", .caption = "OnPrem"},
  }};
};

/// \brief Source access names, captions and ordinary option positions.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataAccess> {
  /// \brief Both original members, not the four-member Field.Access option.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Public", .caption = "Public"},
      {.ordinal = 1, .name = "Internal", .caption = "Internal"},
  }};
};

namespace agiru::platform {

/// \brief Source declaration of System.Reflection.Table Metadata (2000000136).
/// \note Retaining its fields does not implement the read-only live virtual-table provider.
class TableMetadata_Table : public Table<TableMetadata_Table> {
public:
  /// \brief Original System object ID.
  static constexpr TableId kId{2000000136};
  /// \brief Original AL object name.
  static constexpr std::string_view kName{"Table Metadata"};
  /// \brief Record-variable state at the runtime's required first-member offset.
  detail::StateHandle State_Block;
  /// \brief Original Name length.
  static constexpr std::size_t kNameLength = 30;
  /// \brief Original Caption and DataCaptionFields length.
  static constexpr std::size_t kCaptionLength = 80;
  /// \brief Original ObsoleteReason and ExternalName length.
  static constexpr std::size_t kReasonLength = 248;
  /// \brief Original permission/entitlement text length.
  static constexpr std::size_t kPermissionsLength = 5;
  /// \brief Original AL Namespace length.
  static constexpr std::size_t kNamespaceLength = 500;

  ::agiru::Integer ID{};                                ///< AL ID.
  Text<kNameLength> Name;                               ///< AL Name, not Caption.
  Text<kCaptionLength> Caption;                         ///< AL Caption.
  Boolean DataPerCompany{};                             ///< AL DataPerCompany.
  ::agiru::Integer LookupPageID{};                      ///< AL LookupPageID.
  ::agiru::Integer DrillDownPageId{};                   ///< AL DrillDownPageId.
  Text<kCaptionLength> DataCaptionFields;               ///< AL DataCaptionFields.
  Boolean PasteIsValid{};                               ///< AL PasteIsValid.
  Boolean LinkedObject{};                               ///< AL LinkedObject.
  Boolean DataIsExternal{};                             ///< AL DataIsExternal.
  Option<TableMetadataTableType> TableType;             ///< AL TableType.
  Text<kReasonLength> ExternalName;                     ///< AL ExternalName.
  Option<TableMetadataObsoleteState> ObsoleteState;     ///< AL ObsoleteState.
  Text<kReasonLength> ObsoleteReason;                   ///< AL ObsoleteReason.
  Option<FieldDataClassification> DataClassification;   ///< AL DataClassification.
  Boolean ReplicateData{};                              ///< AL ReplicateData.
  Option<TableMetadataCompressionType> CompressionType; ///< AL CompressionType.
  Guid AppID;                                           ///< AL App ID.
  Text<kPermissionsLength> InherentPermissions;         ///< AL InherentPermissions.
  Text<kPermissionsLength> InherentEntitlements;        ///< AL InherentEntitlements.
  Option<TableMetadataScope> Scope;                     ///< AL Scope.
  Option<TableMetadataAccess> Access;                   ///< AL Access.
  Text<kNamespaceLength> ALNamespace;                   ///< AL AL Namespace.
  Guid SystemId;                                        ///< Implicit SystemId accessor.
  DateTime SystemCreatedAt;                             ///< Implicit SystemCreatedAt accessor.
  Guid SystemCreatedBy;                                 ///< Implicit SystemCreatedBy accessor.
  DateTime SystemModifiedAt;                            ///< Implicit SystemModifiedAt accessor.
  Guid SystemModifiedBy;                                ///< Implicit SystemModifiedBy accessor.

  /// \brief Original AL field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo Name{2};
    static constexpr ::agiru::FieldNo Caption{3};
    static constexpr ::agiru::FieldNo DataPerCompany{4};
    static constexpr ::agiru::FieldNo LookupPageID{5};
    static constexpr ::agiru::FieldNo DrillDownPageId{6};
    static constexpr ::agiru::FieldNo DataCaptionFields{7};
    static constexpr ::agiru::FieldNo PasteIsValid{8};
    static constexpr ::agiru::FieldNo LinkedObject{9};
    static constexpr ::agiru::FieldNo DataIsExternal{10};
    static constexpr ::agiru::FieldNo TableType{11};
    static constexpr ::agiru::FieldNo ExternalName{12};
    static constexpr ::agiru::FieldNo ObsoleteState{13};
    static constexpr ::agiru::FieldNo ObsoleteReason{14};
    static constexpr ::agiru::FieldNo DataClassification{15};
    static constexpr ::agiru::FieldNo ReplicateData{16};
    static constexpr ::agiru::FieldNo CompressionType{17};
    static constexpr ::agiru::FieldNo AppID{18};
    static constexpr ::agiru::FieldNo InherentPermissions{19};
    static constexpr ::agiru::FieldNo InherentEntitlements{20};
    static constexpr ::agiru::FieldNo Scope{21};
    static constexpr ::agiru::FieldNo Access{22};
    static constexpr ::agiru::FieldNo ALNamespace{23};
  };

  /// \brief Effective default primary key from the first declared field.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

/// \brief The original AL name's native binding.
using TableMetadata = TableMetadata_Table;

/// \brief All twenty-three original fields in field-number order.
inline constexpr std::array<FieldDef, 23> kTableMetadataFields{{
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
    Declare<&TableMetadata::DrillDownPageId>(TableMetadata::Field_No::DrillDownPageId,
                                             "DrillDownPageId",
                                             "DrillDownPageId",
                                             offsetof(TableMetadata, DrillDownPageId)),
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
}};

/// \brief Effective default primary key; no explicit keys section exists in the source.
inline constexpr std::array<KeyDef, 1> kTableMetadataKeys{{
    KeyDef{.name = "Key1", .fields = TableMetadata::kKey1, .clustered = true},
}};

/// \brief Original represented table properties; Scope/DropDown still need metadata support.
inline constexpr TableDef kTableMetadataTable{
    .id = TableMetadata::kId,
    .name = TableMetadata::kName,
    .caption = TableMetadata::kName,
    .fields = kTableMetadataFields,
    .keys = kTableMetadataKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
};

static_assert(FieldsAreSorted(kTableMetadataTable));
static_assert(offsetof(TableMetadata, State_Block) == 0);

}

/// \brief Native declaration selected for the original System table.
template <> struct agiru::TableTraits<agiru::platform::TableMetadata> {
  /// \brief The immutable table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kTableMetadataTable;
};
