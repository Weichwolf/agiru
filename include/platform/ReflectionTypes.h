#pragma once

#include "meta/EnumDef.h"
#include "type/Option.h"

#include <array>
#include <cstdint>

namespace agiru::platform {

/// \brief Original System Page/Table Metadata option vocabulary, not property-enum ordinals.
enum class PageMetadataPageType : std::int32_t {
  Card = 0,               ///< AL `Card`.
  List = 1,               ///< AL `List`.
  RoleCenter = 2,         ///< AL `RoleCenter`.
  CardPart = 3,           ///< AL `CardPart`.
  ListPart = 4,           ///< AL `ListPart`.
  Document = 5,           ///< AL `Document`.
  Worksheet = 6,          ///< AL `Worksheet`.
  ListPlus = 7,           ///< AL `ListPlus`.
  ConfirmationDialog = 8, ///< AL `ConfirmationDialog`.
  NavigatePage = 9,       ///< AL `NavigatePage`.
  StandardDialog = 10,    ///< AL `StandardDialog`.
  Api = 11,               ///< AL `API`.
  HeadlinePart = 12,      ///< AL `HeadlinePart`.
};

/// \brief Original System Page/Table Metadata option vocabulary, not property-enum ordinals.
enum class TableMetadataTableType : std::int32_t {
  Normal = 0,         ///< AL `Normal`.
  CRM = 1,            ///< AL `CRM`.
  ExternalSQL = 2,    ///< AL `ExternalSQL`.
  Exchange = 3,       ///< AL `Exchange`.
  MicrosoftGraph = 4, ///< AL `MicrosoftGraph`.
  Query = 5,          ///< AL `Query`.
  Temporary = 6,      ///< AL `Temporary`.
};

/// \brief Original System Page/Table Metadata option vocabulary, not property-enum ordinals.
enum class TableMetadataObsoleteState : std::int32_t {
  No = 0,      ///< AL `No`.
  Pending = 1, ///< AL `Pending`.
  Removed = 2, ///< AL `Removed`.
};

/// \brief Original System Page/Table Metadata option vocabulary, not property-enum ordinals.
enum class TableMetadataCompressionType : std::int32_t {
  Unspecified = 0, ///< AL `Unspecified`.
  None = 1,        ///< AL `None`.
  Row = 2,         ///< AL `Row`.
  Page = 3,        ///< AL `Page`.
};

/// \brief Original System Page/Table Metadata option vocabulary, not property-enum ordinals.
enum class TableMetadataScope : std::int32_t {
  Cloud = 0,  ///< AL `Cloud`.
  OnPrem = 1, ///< AL `OnPrem`.
};

/// \brief Original System Page/Table Metadata option vocabulary, not property-enum ordinals.
enum class TableMetadataAccess : std::int32_t {
  Public = 0,   ///< AL `Public`.
  Internal = 1, ///< AL `Internal`.
};

}

/// \brief Source-declared option names and captions in zero-based declaration order.
template <> struct agiru::OptionTraits<agiru::platform::PageMetadataPageType> {
  /// \brief Original System option population; no additional property kinds.
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

/// \brief Source-declared option names and captions in zero-based declaration order.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataTableType> {
  /// \brief Original System option population; no additional property kinds.
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

/// \brief Source-declared option names and captions in zero-based declaration order.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataObsoleteState> {
  /// \brief Original System option population; no additional property kinds.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "No", .caption = "No"},
      {.ordinal = 1, .name = "Pending", .caption = "Pending"},
      {.ordinal = 2, .name = "Removed", .caption = "Removed"},
  }};
};

/// \brief Source-declared option names and captions in zero-based declaration order.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataCompressionType> {
  /// \brief Original System option population; no additional property kinds.
  static constexpr std::array<agiru::EnumValueDef, 4> kValues{{
      {.ordinal = 0, .name = "Unspecified", .caption = "Unspecified"},
      {.ordinal = 1, .name = "None", .caption = "None"},
      {.ordinal = 2, .name = "Row", .caption = "Row"},
      {.ordinal = 3, .name = "Page", .caption = "Page"},
  }};
};

/// \brief Source-declared option names and captions in zero-based declaration order.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataScope> {
  /// \brief Original System option population; no additional property kinds.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Cloud", .caption = "Cloud"},
      {.ordinal = 1, .name = "OnPrem", .caption = "OnPrem"},
  }};
};

/// \brief Source-declared option names and captions in zero-based declaration order.
template <> struct agiru::OptionTraits<agiru::platform::TableMetadataAccess> {
  /// \brief Original System option population; no additional property kinds.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Public", .caption = "Public"},
      {.ordinal = 1, .name = "Internal", .caption = "Internal"},
  }};
};
