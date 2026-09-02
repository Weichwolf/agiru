#pragma once

#include <cstdint>

/// \file
/// \brief AL `TableConnectionType` -- Use variables of this data type to specify the type of
/// connection to an external database.
///
/// The members and their order come from
/// `methods-auto/tableconnectiontype/tableconnectiontype-option.md`, which is the specification: an
/// AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `TableConnectionType`. Use variables of this data type to specify the type of
/// connection to an external database.
enum class TableConnectionType : std::int32_t {
  CRM, ///< Specifies the table as an integration table for integrating Dynamics 365 Business
       ///< Central with Dynamics 365 for Sales. The table is typically based on an entity in
       ///< Dynamics 365 for Sales, such as the Accounts entity.
  ExternalSQL, ///< Specifies the table as a table or view in SQL Server that is not in the Dynamics
               ///< 365 Business Central database.
  Exchange,    ///< This is for internal use only.
  MicrosoftGraph, ///< This is for internal use only.
};

} // namespace agiru
