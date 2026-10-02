#pragma once

#include "meta/EnumDef.h"

#include <array>
#include <cstdint>

namespace agiru::platform {

/// \brief Source-declared obsolete states shared by Field and Table Metadata.
enum class ObsoleteState : std::int32_t {
  No = 0,      ///< Not obsolete.
  Pending = 1, ///< Pending removal.
  Removed = 2, ///< Removed from AL use.
};

/// \brief Source-declared Field/Table Metadata classifiers, not telemetry ordinals.
enum class FieldDataClassification : std::int32_t {
  CustomerContent = 0,                     ///< Customer-provided content.
  ToBeClassified = 1,                      ///< Unclassified data.
  EndUserIdentifiableInformation = 2,      ///< User-identifying data.
  AccountData = 3,                         ///< Account data.
  EndUserPseudonymousIdentifiers = 4,      ///< Pseudonymous user identifiers.
  OrganizationIdentifiableInformation = 5, ///< Organisation-identifying data.
  SystemMetadata = 6,                      ///< Non-identifying system metadata.
};

}

/// \brief Original System Field/Table Metadata obsolete-state vocabulary.
template <> struct agiru::OptionTraits<agiru::platform::ObsoleteState> {
  /// \brief Names and ordinals in declaration order.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "No", .caption = "No"},
      {.ordinal = 1, .name = "Pending", .caption = "Pending"},
      {.ordinal = 2, .name = "Removed", .caption = "Removed"},
  }};
};

/// \brief Original System Field/Table Metadata classifier vocabulary.
template <> struct agiru::OptionTraits<agiru::platform::FieldDataClassification> {
  /// \brief Names and ordinary option positions, separate from telemetry classifiers.
  static constexpr std::array<agiru::EnumValueDef, 7> kValues{{
      {.ordinal = 0, .name = "CustomerContent", .caption = "CustomerContent"},
      {.ordinal = 1, .name = "ToBeClassified", .caption = "ToBeClassified"},
      {.ordinal = 2,
       .name = "EndUserIdentifiableInformation",
       .caption = "EndUserIdentifiableInformation"},
      {.ordinal = 3, .name = "AccountData", .caption = "AccountData"},
      {.ordinal = 4,
       .name = "EndUserPseudonymousIdentifiers",
       .caption = "EndUserPseudonymousIdentifiers"},
      {.ordinal = 5,
       .name = "OrganizationIdentifiableInformation",
       .caption = "OrganizationIdentifiableInformation"},
      {.ordinal = 6, .name = "SystemMetadata", .caption = "SystemMetadata"},
  }};
};
