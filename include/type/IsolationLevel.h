#pragma once

#include <cstdint>

/// \file
/// \brief AL `IsolationLevel` -- The isolation level applied for this record.
///
/// The members and their order come from `methods-auto/isolationlevel/isolationlevel-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `IsolationLevel`. The isolation level applied for this record.
enum class IsolationLevel : std::int32_t {
  Default, ///< Follows the table's isolation level for reads, same behavior as not choosing an
           ///< IsolationLevel.
  ReadUncommitted, ///< Allows the record to read uncommitted changes from other transactions.
  ReadCommitted,   ///< Only allows for reads committed data, but does not guarantee that rows read
                   ///< will stay consistent throughout the entirety of the transaction.
  RepeatableRead,  ///< Guarantees that rows read will stay consistent during the entirety of the
                   ///< current transaction. Does not allow reading of uncommitted data.
  UpdLock, ///< Ensures that the rows read will stay consistent in the entirety of the current
           ///< transaction, while also blocking readers with the same isolation level. Does not
           ///< allow reading of uncommitted data.
};

}
