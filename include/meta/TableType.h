#pragma once

#include <cstdint>

namespace agiru {

/// \brief AL source `TableType` vocabulary from `devenv-tabletype-property.md`.
/// \note These are source identities, not native metadata option ordinals.
enum class TableType : std::uint8_t {
  Normal,         ///< Ordinary stored table.
  CRM,            ///< CRM external table.
  CDS,            ///< CDS source kind; native metadata projects it to CRM.
  ExternalSQL,    ///< External SQL table.
  Exchange,       ///< Exchange external table.
  MicrosoftGraph, ///< Microsoft Graph external table.
  Temporary,      ///< Temporary-only table.
};

}
