#pragma once

#include "type/Boolean.h"

#include <string>

/// \file
/// \brief What a tenant says about its own deployment.

namespace agiru {

/// \brief The deployment facts a tenant carries: which ring, which family, sandbox or not.
///
/// \note THEY ARE DATA AND NOT CODE, which is why they sit here rather than inside the .NET helper
///       that reads them. `Codeunit "Environment Information"` asks
///       `NavTenantSettingsHelper.IsSandbox()` and the BaseApp branches on the answer in a hundred
///       places; the test libraries SET it
///       (`EnvironmentInfoTestLibrary.SetTestabilitySoftwareAsAService`), so it has to be a value a
///       session holds and not a constant a function returns.
///
/// \note THE DEFAULTS DESCRIBE A SELF-HOSTED PRODUCTION INSTANCE, which is what this tree is until
///       something says otherwise: not the service, not a sandbox, W1, no ring.
struct TenantSettings {
  /// \brief Whether this runs as the service rather than self-hosted.
  Boolean saas = false;

  /// \brief Whether this is a sandbox rather than a production tenant.
  Boolean sandbox = false;

  /// \brief The environment's name; empty when self-hosted, because there is no environment.
  std::string environmentName;

  /// \brief The localisation family. `BC_VERSION`'s country is `w1`, so this is its default.
  std::string applicationFamily = "W1";

  /// \brief The deployment ring. `EnvironmentInformationImpl` compares it against `'PREVIEW'`, so
  ///        an empty ring is the answer that says "not a preview" without inventing a name.
  std::string ringName;
};

} // namespace agiru
