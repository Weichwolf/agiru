#pragma once

#include <cstdint>

/// \file
/// \brief AL `ClientType` -- Represents the type of the client executing the operation.
///
/// The members and their order come from `methods-auto/clienttype/clienttype-option.md`, which is
/// the specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `ClientType`. Represents the type of the client executing the operation.
enum class ClientType : std::int32_t {
  Background,   ///< A background session.
  ChildSession, ///< A child session.
  Desktop,      ///< A desktop client.
  Management,   ///< A management client.
  NAS,          ///< A NAS client.
  OData,        ///< A NAS client.
  Phone,        ///< Microsoft Dynamics Business Central Phone client.
  SOAP,         ///< A SOAP client.
  Tablet,       ///< Microsoft Dynamics Business Central Tablet client.
  Web,          ///< Microsoft Dynamics Business Central Web client.
  Windows,      ///< Microsoft Dynamics Business Central Windows client.
  Current,      ///< Microsoft Dynamics Business Central Windows client.
  Default,      ///< The default client.
  ODataV4,      ///< A ODataV4 client.
  Api,          ///< An API client.
  Teams,        ///< Microsoft Teams client.
};

}
