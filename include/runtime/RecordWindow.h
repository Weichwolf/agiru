#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

/// \file
/// \brief Bounded SQL record windows in the record's declared order and collation.

namespace agiru {

struct TableDef;

/// \brief Direction of a read window; relative positions exclude the supplied anchor.
enum class RecordWindowPosition : std::uint8_t {
  Unknown,
  First,
  After,
  Before,
  Last,
};

/// \brief Move-only, immutable rows from one bounded SQL statement.
/// Owns its returned values, not a connection, cursor, page or transaction boundary.
/// The caller keeps it private to its session and retains the immutable table declaration.
class RecordWindow {
public:
  /// \brief Releases the bounded result without SQL or implicit commits.
  ~RecordWindow();
  RecordWindow(const RecordWindow &) = delete;
  RecordWindow &operator=(const RecordWindow &) = delete;
  /// \brief Transfers result ownership. \param other The window moved from.
  RecordWindow(RecordWindow &&other) noexcept;
  /// \brief Transfers result ownership. \param other The window moved from. \return This.
  RecordWindow &operator=(RecordWindow &&other) noexcept;

  /// \return Number of displayable rows, never exceeding the requested limit.
  [[nodiscard]] std::size_t Size() const;
  /// \return Whether another row existed beyond this window in the requested direction.
  [[nodiscard]] bool HasMore() const;
  /// \return Rows transferred by SQL, including at most one continuation probe.
  /// \note This is not a bound on database scan work; indexed plans require qualification.
  [[nodiscard]] std::size_t RowsRead() const;

  /// \brief Loads one exact stored row, retaining the destination's filters and sort key.
  /// Rows are always addressed in the declared forward order, also for Before/Last.
  /// \param index Zero-based display row, excluding the continuation probe.
  /// \param record Non-null typed storage matching the window's table declaration.
  /// \throws Error for invalid storage/index, absent authority or unsupported providers.
  /// \note Reauthorizes reads; closes the destination cursor, marks its SQL position and
  /// runs no AL validation/page triggers, save, Commit or rollback.
  void Load(std::size_t index, void *record) const;

private:
  struct Impl;
  explicit RecordWindow(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
  friend RecordWindow ReadRecordWindow(const void *record,
                                       const TableDef &table,
                                       RecordWindowPosition position,
                                       std::size_t limit);
};

/// \brief Validates the trusted row bound before any page trigger or row-save effects.
/// \param limit Positive bound leaving room for one SQL continuation probe.
/// \throws Error with RecordWindowLimit for zero or an unrepresentable SQL bound.
void ValidateRecordWindowLimit(std::size_t limit);

/// \brief Executes a bounded read using all active filter groups and the complete stable key.
/// ORDER BY and seek predicates use the same SQL columns, directions and collations,
/// with the primary key appended as a unique tie-breaker. No offsets/client-side comparison.
/// \param record Non-null matching storage; supplies filters, sort key and relative anchor.
/// \param table Immutable source declaration, retained for the result lifetime.
/// \param position First/Last, or strictly After/Before the loaded source key values.
/// \param limit Positive trusted server bound; the SQL statement reads at most limit + 1.
/// \return Owned rows in declared forward order without changing the source record/cursor.
/// \throws Error for invalid limits/positions, missing permissions or unsupported temporary,
/// virtual or sequence providers. Those need separate collation-qualified adapters.
/// \note This primitive is not list HTML, AL trigger ordering or BC collation parity.
[[nodiscard]] RecordWindow ReadRecordWindow(const void *record,
                                            const TableDef &table,
                                            RecordWindowPosition position,
                                            std::size_t limit);

}
