#pragma once

#include <cstdint>

/// \file
/// \brief AL `ReportLayoutType` -- Represents the type of a report layout.
///
/// The members and their order come from
/// `methods-auto/reportlayouttype/reportlayouttype-option.md`, which is the specification: an AL
/// option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `ReportLayoutType`. Represents the type of a report layout.
enum class ReportLayoutType : std::int32_t {
  RDLC,   ///< Denotes a report layout of type RDLC.
  Word,   ///< Denotes a report layout of type Microsoft Word.
  Excel,  ///< Denotes a report layout of type Microsoft Excel.
  Custom, ///< Denotes a report layout of a user-defined type.
};

} // namespace agiru
