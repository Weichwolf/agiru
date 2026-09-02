#pragma once

#include "type/Boolean.h"

#include <string>

/// \file
/// \brief The .NET type `NavTenantSettingsHelper` -- the tenant's own deployment facts.

namespace agiru::dotnet {

/// \brief What the platform tells AL about the tenant it runs in.
///
/// `Codeunit "Environment Information"` is built on it: `IsProduction`, `IsSandbox`,
/// `GetEnvironmentName`, `GetApplicationFamily` and `GetRingName` are the five it reads, and the
/// BaseApp branches on the answers throughout.
///
/// \note IT READS THE SESSION AND HOLDS NOTHING, so every answer is `static`. The values are a
///       property of the TENANT and live on the session (`agiru::TenantSettings`), because the test
///       libraries set them -- a function returning a constant could not be set.
///
/// \note The predecessor has no equivalent: `~/Git/openerp` never built this type, so every call on
///       it returned its nil value and `IsSandbox()` answered falsely without failing. That is the
///       shape of defect this tree is arranged against, and it is why the answers come from a value
///       somebody can set rather than from nothing.
class NavTenantSettingsHelper {
public:
  /// \brief Whether this is a production tenant.
  /// \return True unless the tenant says it is a sandbox.
  [[nodiscard]] static Boolean IsProduction();

  /// \brief Whether this is a sandbox tenant.
  /// \return What the tenant says.
  [[nodiscard]] static Boolean IsSandbox();

  /// \brief The environment's name.
  /// \return What the tenant says; empty when self-hosted.
  [[nodiscard]] static std::string GetEnvironmentName();

  /// \brief The localisation family.
  /// \return What the tenant says; `W1` by default.
  [[nodiscard]] static std::string GetApplicationFamily();

  /// \brief The deployment ring.
  /// \return What the tenant says; empty when there is none.
  [[nodiscard]] static std::string GetRingName();

  /// \brief Turns Microsoft 365 collaboration on for the tenant.
  /// \throws Error always.
  /// \warning REFUSED. It CHANGES the tenant, and this runtime has no tenant service to change --
  ///          doing nothing quietly would report success for something that did not happen.
  static void EnableM365Collaboration();
};

} // namespace agiru::dotnet
