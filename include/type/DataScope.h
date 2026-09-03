#pragma once

#include <cstdint>

/// \file
/// \brief AL `DataScope` -- Identifies the scope of stored data in the isolated storage.
///
/// The members and their order come from `methods-auto/datascope/datascope-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `DataScope`. Identifies the scope of stored data in the isolated storage.
enum class DataScope : std::int32_t {
  Module,  ///< Indicates that the record is available in the scope of the app(extension) context.
  Company, ///< Indicates that the record is available in the scope of the company within the app
           ///< context.
  User,    ///< Indicates that the record is available for a user within the app context.
  CompanyAndUser, ///< Indicates that the record is available for a user and specific company within
                  ///< the app context.
};

}
