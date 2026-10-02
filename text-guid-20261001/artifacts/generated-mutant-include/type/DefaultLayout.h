#pragma once

#include <cstdint>

/// \file
/// \brief AL `DefaultLayout` -- The default layout to be used by a report.
///
/// The members and their order come from `methods-auto/defaultlayout/defaultlayout-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `DefaultLayout`. The default layout to be used by a report.
enum class DefaultLayout : std::int32_t {
  None,  ///< The default layout is not set.
  RDLC,  ///< The default layout is RDLC.
  Word,  ///< The default layout is Word.
  Excel, ///< The default layout is Excel.
};

}
