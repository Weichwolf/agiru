#pragma once

#include <cstdint>

/// \file
/// \brief AL `CommitBehavior` -- Specifies whether commit is allowed within the scope of the
/// method.
///
/// The members and their order come from `methods-auto/commitbehavior/commitbehavior-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `CommitBehavior`. Specifies whether commit is allowed within the scope of the method.
enum class CommitBehavior : std::int32_t {
  Ignore, ///< Ignore commits within the scope of this method.
  Error,  ///< Throw an error when a commit is attempted within the scope of this method.
};

}
