#pragma once

/// \file
/// \brief Trusted host policy shared by native sessions, never supplied by AL or HTTP input.
namespace agiru {

/// \brief Runtime policy copied into each session before execution.
struct SessionOptions {
  /// BC server DisableWriteInsideTryFunctions; false permits writes without try-call rollback.
  /// The default preserves the BC online policy. Native hosts may select the on-premises policy.
  bool disableWriteInsideTryFunctions = false;
};

}
