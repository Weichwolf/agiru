#pragma once

#include "meta/Ids.h"

/// \file
/// \brief Session-owned SingleInstance storage independent of generated codeunit templates.

namespace agiru::detail {

/// \brief The one instance of a SingleInstance codeunit in the current session.
/// \param id The codeunit's AL identity.
/// \param make Factory for a new instance.
/// \param free Destructor for an owned instance.
/// \return The existing or newly owned instance.
/// \throws SessionError without a session; Error for an invalid factory or null result.
/// \note Storage belongs to the session, not the worker or its current connection.
/// Company-close invalidation remains part of WI 0058's absorbed company lifecycle contract.
[[nodiscard]] void *SingleInstanceOf(CodeunitId id, void *(*make)(), void (*free)(void *));

/// \brief Frees all SingleInstances belonging to the current session.
void ReleaseSingleInstances();

}
