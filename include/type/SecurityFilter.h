#pragma once

#include <cstdint>

/// \file
/// \brief AL `SecurityFilter` -- Specifies how security filters are applied to the record.
///
/// The members and their order come from `methods-auto/securityfilter/securityfilter-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `SecurityFilter`. Specifies how security filters are applied to the record.
enum class SecurityFilter : std::int32_t {
  Validated,  ///< All security filters are applied to this instance of the record and if any code
              ///< tries to access a record that is outside the range of the security filters, then
              ///< an error occurs.
  Filtered,   ///< All security filters are applied to this instance of the record.
  Ignored,    ///< All security filters are ignored for this instance of the record.
  Disallowed, ///< Security filters are not allowed on the record. If any security filters are set,
              ///< then you receive an error when you run the object that uses this instance of the
              ///< record.
};

}
