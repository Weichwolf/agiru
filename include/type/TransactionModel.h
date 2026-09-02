#pragma once

#include <cstdint>

/// \file
/// \brief AL `TransactionModel` -- Represents a test transaction model.
///
/// The members and their order come from
/// `methods-auto/transactionmodel/transactionmodel-option.md`, which is the specification: an AL
/// option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `TransactionModel`. Represents a test transaction model.
enum class TransactionModel : std::int32_t {
  AutoCommit,   ///< The transaction automatically commits after the Test method has run.
  AutoRollback, ///< The transaction is automatically rolled back after the Test method has run.
  None,         ///< No write-transaction is open in the test-method code, and writes will fail. The
        ///< transaction model mirrors the model used by the "real" client. Every call from the
        ///< TestPage to the "server" has its own transaction.
};

} // namespace agiru
