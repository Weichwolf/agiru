#pragma once

#include <cstdint>

/// \file
/// \brief AL `Action` -- Represents the action that the user took on the page.
///
/// The members and their order come from `methods-auto/action/action-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `Action`. Represents the action that the user took on the page.
enum class Action : std::int32_t {
  None, ///< Represents the result of running a page.
  OK,   ///< Represents the result of the user closing a page window by performing one of the
        ///< following actions:<br/>      - Chooses the **OK** button.<br/>      - Chooses the **X**
      ///< button when there was no **Cancel** button on the window.<br/>      - Presses the Esc key
      ///< when there is no **Cancel** button on the window.
  Cancel, ///< Represents the result of the user closing a page window by performing one of the
          ///< following actions:<br/>      - Chooses the **Cancel** button.<br/>      - Chooses the
          ///< **X** button when there is a **Cancel** button on the window.<br/>      - Presses the
          ///< Esc key when there is a **Cancel** button on the window
  LookupOK, ///< Represents the result of the user closing a lookup window by performing one of the
            ///< following actions:<br/>      - Chooses the **OK** button.<br/>      - Chooses an
            ///< item in the Lookup window.
  LookupCancel, ///< Represents the result of the user closing a lookup window by choosing the
                ///< **Cancel** button.
  Yes, ///< Represents the result of the user closing a confirmation window by choosing the **Yes**
       ///< button.
  No,  ///< Represents the result of the user closing a confirmation window by performing one of the
       ///< following actions:<br/>      - Chooses the No button.<br/>      - Chooses the X
       ///< button.<br/>      - Presses the Esc key.
  RunObject, ///< Represents the result of the user selecting an option that ran another object.
  RunSystem, ///< Represents the result of the user selecting an option that ran an external
             ///< program.
};

} // namespace agiru
