#pragma once

#include <cstdint>

/// \file
/// \brief AL `DataClassification` -- what kind of data a field or a telemetry dimension holds.

namespace agiru {

/// \brief AL `DataClassification`.
///
/// \note THE MEMBERS ARE MEASURED FROM THE SOURCE RATHER THAN FROM A PAGE, because the platform
///       documentation describes the PROPERTY and never tabulates the option. Every value the
///       BaseApp writes is here, counted over its `.al` files: SystemMetadata 3 232,
///       CustomerContent 2 794, EndUserIdentifiableInformation 284,
///       OrganizationIdentifiableInformation 57, EndUserPseudonymousIdentifiers 56,
///       AccountData 7. `ToBeClassified` is the platform's default for a field that says nothing,
///       which is why it comes first.
enum class DataClassification : std::int32_t {
  ToBeClassified,                     ///< The default: nothing has been said about the field.
  SystemMetadata,                     ///< Data the platform itself keeps.
  EndUserIdentifiableInformation,     ///< Data that identifies a person.
  AccountData,                        ///< Data about the tenant's account.
  EndUserPseudonymousIdentifiers,     ///< Identifiers that stand for a person without naming one.
  CustomerContent,                    ///< What the customer put in.
  OrganizationIdentifiableInformation ///< Data that identifies the organisation.
};

} // namespace agiru
