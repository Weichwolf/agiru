#pragma once

#include <cstdint>

/// \file
/// \brief AL `PageBackgroundTaskErrorLevel` -- Specifies how an error in the page background task
/// appears in the client.
///
/// The members and their order come from
/// `methods-auto/pagebackgroundtaskerrorlevel/pagebackgroundtaskerrorlevel-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `PageBackgroundTaskErrorLevel`. Specifies how an error in the page background task
/// appears in the client.
enum class PageBackgroundTaskErrorLevel : std::int32_t {
  Error,   ///< Error occuring in a page background task is displayed as an normal error in the
           ///< client. This is the default value.
  Warning, ///< Error occuring in a page background task is displayed as an warning in the client.
           ///< **Note:** Any error thrown in completion trigger will ignore this value and will be
           ///< displayed in the client as a normal error.
  Ignore,  ///< Error occuring in a page background task is not displayed in the client. **Note:**
           ///< Any error thrown in completion trigger will ignore this value and will be displayed
           ///< in the client as a normal error.
};

}
