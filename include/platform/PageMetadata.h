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

/// \brief `Page Metadata.PageType`, the platform's option members in their order.
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
  ReportPreview = 12,
  ReportProcessingOnly = 13,
  HeadlinePart = 14,
  PromptDialog = 15,
  UserControlHost = 16,
};

}

template <> struct agiru::OptionTraits<agiru::platform::PageMetadataPageType> {
  static constexpr std::array<agiru::EnumValueDef, 17> kValues{{
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
      {.ordinal = 12, .name = "ReportPreview", .caption = "ReportPreview"},
      {.ordinal = 13, .name = "ReportProcessingOnly", .caption = "ReportProcessingOnly"},
      {.ordinal = 14, .name = "HeadlinePart", .caption = "HeadlinePart"},
      {.ordinal = 15, .name = "PromptDialog", .caption = "PromptDialog"},
      {.ordinal = 16, .name = "UserControlHost", .caption = "UserControlHost"},
  }};
};

namespace agiru::platform {

/// \brief The virtual table `Page Metadata` (2000000138): one row per page the build carries,
///        written from the page catalogue when the database is provisioned, the way `Table
///        Metadata` is. `PageManagement` and the profile pages read it by `ID`.
class PageMetadata_Table : public Table<PageMetadata_Table> {
public:
  static constexpr TableId kId{2000000138};
  static constexpr std::string_view kName{"Page Metadata"};

  detail::StateHandle State_Block;

  static constexpr std::size_t kNameLength = 30;
  static constexpr std::size_t kCaptionLength = 249;

  ::agiru::Integer ID{};
  Text<kNameLength> Name;
  Text<kCaptionLength> Caption;
  Option<PageMetadataPageType> PageType;
  ::agiru::Integer SourceTable{};
  ::agiru::Integer CardPageID{};
  Boolean SourceTableTemporary;
  Boolean Editable;
  Boolean InsertAllowed;
  Boolean ModifyAllowed;
  Boolean DeleteAllowed;
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

  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo Name{2};
    static constexpr ::agiru::FieldNo Caption{3};
    static constexpr ::agiru::FieldNo PageType{4};
    static constexpr ::agiru::FieldNo SourceTable{5};
    static constexpr ::agiru::FieldNo CardPageID{6};
    static constexpr ::agiru::FieldNo SourceTableTemporary{7};
    static constexpr ::agiru::FieldNo Editable{8};
    static constexpr ::agiru::FieldNo InsertAllowed{9};
    static constexpr ::agiru::FieldNo ModifyAllowed{10};
    static constexpr ::agiru::FieldNo DeleteAllowed{11};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

using PageMetadata = PageMetadata_Table;

inline constexpr std::array<FieldDef, 11> kPageMetadataFields{{
    Declare<&PageMetadata::ID>(PageMetadata::Field_No::ID, "ID", "ID", offsetof(PageMetadata, ID)),
    Declare<&PageMetadata::Name>(
        PageMetadata::Field_No::Name, "Name", "Name", offsetof(PageMetadata, Name)),
    Declare<&PageMetadata::Caption>(
        PageMetadata::Field_No::Caption, "Caption", "Caption", offsetof(PageMetadata, Caption)),
    Declare<&PageMetadata::PageType>(
        PageMetadata::Field_No::PageType, "PageType", "PageType", offsetof(PageMetadata, PageType)),
    Declare<&PageMetadata::SourceTable>(PageMetadata::Field_No::SourceTable,
                                        "SourceTable",
                                        "SourceTable",
                                        offsetof(PageMetadata, SourceTable)),
    Declare<&PageMetadata::CardPageID>(PageMetadata::Field_No::CardPageID,
                                       "CardPageID",
                                       "CardPageID",
                                       offsetof(PageMetadata, CardPageID)),
    Declare<&PageMetadata::SourceTableTemporary>(PageMetadata::Field_No::SourceTableTemporary,
                                                 "SourceTableTemporary",
                                                 "SourceTableTemporary",
                                                 offsetof(PageMetadata, SourceTableTemporary)),
    Declare<&PageMetadata::Editable>(
        PageMetadata::Field_No::Editable, "Editable", "Editable", offsetof(PageMetadata, Editable)),
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
}};

inline constexpr std::array<KeyDef, 1> kPageMetadataKeys{{
    KeyDef{.name = "Key1", .fields = PageMetadata::kKey1, .clustered = true},
}};

inline constexpr TableDef kPageMetadataTable{
    .id = PageMetadata::kId,
    .name = PageMetadata::kName,
    .caption = PageMetadata::kName,
    .fields = kPageMetadataFields,
    .keys = kPageMetadataKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kPageMetadataTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::PageMetadata> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPageMetadataTable;
};
