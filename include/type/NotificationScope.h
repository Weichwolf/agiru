#pragma once

#include <cstdint>

/// \file
/// \brief AL `NotificationScope` -- Specifies the context in which the notification appears in the
/// client.
///
/// The members and their order come from
/// `methods-auto/notificationscope/notificationscope-option.md`, which is the specification: an AL
/// option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `NotificationScope`. Specifies the context in which the notification appears in the
/// client.
enum class NotificationScope : std::int32_t {
  GlobalScope, ///< The notifications are not directly related to the user's current task. **Note:**
               ///< GlobalScope is currently not supported, so do not use this value.
  LocalScope,  ///< The notification appears in context of the user's current task, on the page the
               ///< user is currently working on. This is the default value.
};

} // namespace agiru
