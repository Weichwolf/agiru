#pragma once

#include <cstdint>

/// \file
/// \brief AL `PromptMode` -- Specifies the current mode of a PromptDialog page.
///
/// The members and their order come from `methods-auto/promptmode/promptmode-option.md`, which is
/// the specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `PromptMode`. Specifies the current mode of a PromptDialog page.
enum class PromptMode : std::int32_t {
  Prompt,   ///< Prompting the user for input for the copilot interaction.
  Generate, ///< Generating the output of the copilot interaction.
  Content,  ///< Showing the output of the copilot interaction.
};

} // namespace agiru
