#pragma once

#include <cstddef>
#include <cstdint>

/// \file
/// \brief Client-independent block loading for generated list pages.

namespace agiru {

class RecordId;
class PageCore;

/// \brief Navigation in source order (SQL collation or declared AL provider), never an offset.
enum class PageWindowPosition : std::uint8_t { Unknown, First, Next, Previous, Last };

/// \brief Measured row bound and continuation in the requested direction.
struct PageWindowState {
  std::size_t rows = 0; ///< Presented rows, excluding the continuation probe.
  std::size_t rowsRead =
      0;             ///< SQL transferred rows, or custom source rows visited including probe.
                     ///< Custom positioning/AL may read more; no SQL scan/byte bound implied.
  bool more = false; ///< Another row existed beyond this window in the requested direction.
};

/// \brief Synchronous, trusted presentation receiver; never retain the borrowed controls.
/// Implementations read through PageDispatcher with mandatory field authorization and
/// capture exact values immediately. They must not navigate, edit or execute actions.
/// Partial output is discarded if loading raises; callbacks run within the caller's
/// authenticated session and transaction, not a separate client runtime.
class PageWindowReceiver {
public:
  /// \brief Releases a receiver through its interface.
  virtual ~PageWindowReceiver() = default;
  /// \brief Captures one row after its AL OnAfterGetRecord trigger and event.
  /// \param identity Original source primary key, before AL changes to the record buffer.
  /// \param controls Borrowed live controls, including calculated row variables.
  virtual void Row(const RecordId &identity, PageCore &controls) = 0;
  /// \brief Captures the selected row after all row callbacks and any current-row trigger.
  /// \param identity Selected source primary key, empty for an empty view.
  /// \param controls Borrowed live controls on the selected post-trigger record buffer.
  virtual void Current(const RecordId &identity, PageCore &controls) = 0;
};

}
