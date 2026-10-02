#pragma once

#include "meta/Ids.h"

namespace agiru::detail {

/// \brief Binds one Manual subscriber instance in the active session.
/// \param id The subscriber codeunit. \param instance The live instance.
/// \return False for duplicate binding or absent subscription metadata.
/// \throws SessionError without a session; Error for automatic subscribers or null instances.
bool BindSubscriptions(CodeunitId id, void *instance);

/// \brief Unbinds one Manual instance from the active session only.
/// \param id The subscriber codeunit. \param instance The live instance.
/// \return False if this session has no matching binding or subscription metadata.
/// \throws SessionError without a session; Error for automatic subscribers.
bool UnbindSubscriptions(CodeunitId id, void *instance);

/// \brief Removes a destroyed instance's bindings from the active session and its ancestors.
/// \param id The destroyed codeunit. \param instance Its former address, never dereferenced.
/// \note Does not allocate or require an active session. Cross-worker lifetime escape is not
///       supported by the current session activation model (WI 0718/0006).
void ReleaseSubscriptions(CodeunitId id, void *instance) noexcept;

}
