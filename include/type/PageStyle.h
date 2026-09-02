#pragma once

#include <cstdint>

/// \file
/// \brief AL `PageStyle` -- Represents the different kinds of styles that can be applied to page
/// controls.
///
/// The members and their order come from `methods-auto/pagestyle/pagestyle-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `PageStyle`. Represents the different kinds of styles that can be applied to page
/// controls.
enum class PageStyle : std::int32_t {
  None,            ///< None
  Standard,        ///< Standard
  StandardAccent,  ///< Blue
  Strong,          ///< Bold
  StrongAccent,    ///< Blue + Bold
  Attention,       ///< Red + Italic
  AttentionAccent, ///< Blue + Italic
  Favorable,       ///< Bold + Green
  Unfavorable,     ///< Bold + Italic + Red
  Ambiguous,       ///< Yellow
  Subordinate,     ///< Grey
};

} // namespace agiru
