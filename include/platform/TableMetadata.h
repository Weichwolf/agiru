#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
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
#include <string_view>

namespace agiru::platform {

/// \brief The `TableType` of `Table Metadata`: BC's system enum `Table Type`, in its order.
enum class TableMetadataTableType : std::int32_t {
  Normal = 0,
  CRM = 1,
  ExternalSQL = 2,
  Exchange = 3,
  MicrosoftGraph = 4,
  CDS = 5,
  Temporary = 6,
};

/// \brief The `ObsoleteState` of `Table Metadata`: `No`, `Pending`, `Removed`.
enum class TableMetadataObsoleteState : std::int32_t {
  No = 0,
  Pending = 1,
  Removed = 2,
};

}

template <> struct agiru::OptionTraits<agiru::platform::TableMetadataTableType> {
  static constexpr std::array<agiru::EnumValueDef, 7> kValues{{
      {.ordinal = 0, .name = "Normal", .caption = "Normal"},
      {.ordinal = 1, .name = "CRM", .caption = "CRM"},
      {.ordinal = 2, .name = "ExternalSQL", .caption = "ExternalSQL"},
      {.ordinal = 3, .name = "Exchange", .caption = "Exchange"},
      {.ordinal = 4, .name = "MicrosoftGraph", .caption = "MicrosoftGraph"},
      {.ordinal = 5, .name = "CDS", .caption = "CDS"},
      {.ordinal = 6, .name = "Temporary", .caption = "Temporary"},
  }};
};

template <> struct agiru::OptionTraits<agiru::platform::TableMetadataObsoleteState> {
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "No", .caption = "No"},
      {.ordinal = 1, .name = "Pending", .caption = "Pending"},
      {.ordinal = 2, .name = "Removed", .caption = "Removed"},
  }};
};

namespace agiru::platform {

/// \brief The virtual table `Table Metadata` (2000000136): one row per table this build carries,
///        written from the catalogue when the runner's database is provisioned, the way `AllObj`
///        is. `Workflow Event`, `Table Relations Metadata` and the record-link and data-migration
///        codeunits read it (39 `Get`, 22 `TableType`, 16 `ObsoleteState` in the BaseApp; 27 UT
///        cases stopped at its absence, 2026-09-10). The field numbers follow the predecessor's
///        `virtual_metadata.py`; no BaseApp site names one by number.
class TableMetadata_Table : public Table<TableMetadata_Table> {
public:
  static constexpr TableId kId{2000000136};
  static constexpr std::string_view kName{"Table Metadata"};

  detail::StateHandle State_Block;

  static constexpr std::size_t kNameLength = 30;
  static constexpr std::size_t kCaptionLength = 249;
  static constexpr std::size_t kReasonLength = 250;

  ::agiru::Integer ID{};
  Text<kNameLength> Name;
  Text<kCaptionLength> Caption;
  Option<TableMetadataObsoleteState> ObsoleteState;
  Text<kReasonLength> ObsoleteReason;
  Option<TableMetadataTableType> TableType;
  Boolean DataPerCompany;
  ::agiru::Integer LookupPageID{};
  ::agiru::Integer DrillDownPageID{};
  Boolean DataIsExternal;
  Text<kCaptionLength> ExternalName;
  /// \brief AL `TableMetadata.SystemId`.
  Guid SystemId;
  /// \brief AL `TableMetadata.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `TableMetadata.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `TableMetadata.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `TableMetadata.SystemModifiedBy`.
  Guid SystemModifiedBy;

  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo Name{2};
    static constexpr ::agiru::FieldNo Caption{3};
    static constexpr ::agiru::FieldNo ObsoleteState{4};
    static constexpr ::agiru::FieldNo ObsoleteReason{5};
    static constexpr ::agiru::FieldNo TableType{6};
    static constexpr ::agiru::FieldNo DataPerCompany{7};
    static constexpr ::agiru::FieldNo LookupPageID{8};
    static constexpr ::agiru::FieldNo DrillDownPageID{9};
    static constexpr ::agiru::FieldNo DataIsExternal{10};
    static constexpr ::agiru::FieldNo ExternalName{11};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

using TableMetadata = TableMetadata_Table;

inline constexpr std::array<FieldDef, 11> kTableMetadataFields{{
    Declare<&TableMetadata::ID>(
        TableMetadata::Field_No::ID, "ID", "ID", offsetof(TableMetadata, ID)),
    Declare<&TableMetadata::Name>(
        TableMetadata::Field_No::Name, "Name", "Name", offsetof(TableMetadata, Name)),
    Declare<&TableMetadata::Caption>(
        TableMetadata::Field_No::Caption, "Caption", "Caption", offsetof(TableMetadata, Caption)),
    Declare<&TableMetadata::ObsoleteState>(TableMetadata::Field_No::ObsoleteState,
                                           "ObsoleteState",
                                           "ObsoleteState",
                                           offsetof(TableMetadata, ObsoleteState)),
    Declare<&TableMetadata::ObsoleteReason>(TableMetadata::Field_No::ObsoleteReason,
                                            "ObsoleteReason",
                                            "ObsoleteReason",
                                            offsetof(TableMetadata, ObsoleteReason)),
    Declare<&TableMetadata::TableType>(TableMetadata::Field_No::TableType,
                                       "TableType",
                                       "TableType",
                                       offsetof(TableMetadata, TableType)),
    Declare<&TableMetadata::DataPerCompany>(TableMetadata::Field_No::DataPerCompany,
                                            "DataPerCompany",
                                            "DataPerCompany",
                                            offsetof(TableMetadata, DataPerCompany)),
    Declare<&TableMetadata::LookupPageID>(TableMetadata::Field_No::LookupPageID,
                                          "LookupPageID",
                                          "LookupPageID",
                                          offsetof(TableMetadata, LookupPageID)),
    Declare<&TableMetadata::DrillDownPageID>(TableMetadata::Field_No::DrillDownPageID,
                                             "DrillDownPageID",
                                             "DrillDownPageID",
                                             offsetof(TableMetadata, DrillDownPageID)),
    Declare<&TableMetadata::DataIsExternal>(TableMetadata::Field_No::DataIsExternal,
                                            "DataIsExternal",
                                            "DataIsExternal",
                                            offsetof(TableMetadata, DataIsExternal)),
    Declare<&TableMetadata::ExternalName>(TableMetadata::Field_No::ExternalName,
                                          "ExternalName",
                                          "ExternalName",
                                          offsetof(TableMetadata, ExternalName)),
}};

inline constexpr std::array<KeyDef, 1> kTableMetadataKeys{{
    KeyDef{.name = "Key1", .fields = TableMetadata::kKey1, .clustered = true},
}};

inline constexpr TableDef kTableMetadataTable{
    .id = TableMetadata::kId,
    .name = TableMetadata::kName,
    .caption = TableMetadata::kName,
    .fields = kTableMetadataFields,
    .keys = kTableMetadataKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kTableMetadataTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::TableMetadata> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kTableMetadataTable;
};
